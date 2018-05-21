//
// Created by Clàudia Peiró Vidal on 17/4/18.
//

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "fat.h"

//https://www.codeguru.com/cpp/cpp/cpp_mfc/files/article.php/c13907/Long-File-Name-LFN-Entries-in-the-FAT-Root-Directory-of-Floppy-Disks.htm#page-2


typedef struct _LongFileName
{
    uint8_t  seqNumber;
    uint8_t  fileName[26];

} lfn;



lfn readLongFileName (FILE * file,  uint64_t pos) {
    lfn out;
    uint8_t fileName_Part1[10];
    uint8_t fileName_Part2[12];
    uint8_t fileName_Part3[4];
    int i = 0;

    fseek(file, pos, SEEK_SET);
    fread(&out.seqNumber, sizeof(uint8_t), 1, file);

    fseek(file, pos + sizeof(uint8_t), SEEK_SET);
    fread(fileName_Part1, sizeof(uint8_t) * 10, 1, file);

    fseek(file, pos + sizeof(uint8_t) * 13, SEEK_SET);
    fread(fileName_Part2, sizeof(uint8_t) * 12, 1, file);
    fread(fileName_Part3, sizeof(uint8_t) * 4, 1, file);

    for (i = 0; i < 26; i++) {

        if (i < 10) {
            out.fileName[i] = fileName_Part1[i];
        } else if (i > 9 && i < 22) {
            out.fileName[i] = fileName_Part1[i - 10];
        } else {
            out.fileName[i] = fileName_Part1[i - 22];
        }
    }

    return out;
}

void showFat(fat32 fat) {

    printf("---- Filesystem Information ----\n\n");

    printf("Filesystem: FAT32\n");
    printf("System Name: %s\n", fat.systemName);
    printf("Sector Size: %hu\n", fat.sectorSize);
    printf("Sectors per Cluster: %hhu\n", fat.sectorPerCluster);
    printf("Reserved Sectors: %hu\n", fat.reservedSectors);
    printf("Number of FATs: %hhu\n", fat.numberOfFats);
    printf("Maximum Root Entries: %hu\n", fat.maximumRootEntries);
    printf("Sectors per FAT: %hu\n", fat.numberOfSectorsPerFat);
    printf("Root first cluster: %hhu\n", fat.rootFirstCluster);
    printf("Label: %s\n", fat.label);
}

void showCluster (clusterData cluster) {

    printf("\n\n---- Cluster ----\n\n");

    printf("Short Name: %s\n", cluster.name);
    printf("Attributes: %hu\n", cluster.attributs);
    printf("Date: %hhu\n", cluster.date);
    printf("Next address: %hhu\n", cluster.nextCluster);
    printf("Size: %hhu\n", cluster.size);
}

fat32 readFat32(FILE *file) {
    fat32 fat;

    fseek(file, SYSTEM_NAME, SEEK_SET);
    fread(fat.systemName, sizeof(fat.systemName), 1, file);

    fseek(file, SECTOR_SIZE, SEEK_SET);
    fread(&fat.sectorSize, sizeof(fat.sectorSize), 1, file);

    fseek(file, SECTOR_CLUSTER, SEEK_SET);
    fread(&fat.sectorPerCluster, sizeof(fat.sectorPerCluster), 1, file);

    fseek(file, RESERVED_SECTORS, SEEK_SET);
    fread(&fat.reservedSectors, sizeof(fat.reservedSectors), 1, file);

    fseek(file, NUMBER_OF_FATS, SEEK_SET);
    fread(&fat.numberOfFats, sizeof(fat.numberOfFats), 1, file);

    fseek(file, ROOT_ENTRIES, SEEK_SET);
    fread(&fat.maximumRootEntries, sizeof(fat.maximumRootEntries), 1, file);

    fseek(file, ROOT_FIRST_CLUSTER, SEEK_SET);
    fread(&fat.rootFirstCluster, sizeof(fat.rootFirstCluster), 1, file);

    fseek(file, NUMBER_FATS_SECTOR, SEEK_SET);
    fread(&fat.numberOfSectorsPerFat, sizeof(fat.numberOfSectorsPerFat), 1, file);

    fseek(file, LABEL, SEEK_SET);
    fread(&fat.label, sizeof(char) * 8, 1, file);
    fat.label[8] = 0;

    showFat(fat);
    return fat;
}

clusterData readCluster(FILE *file, uint64_t pos) {
    clusterData cluster;
    uint16_t low;
    uint32_t high;

    fseek(file, NAME + pos, SEEK_SET);
    fread(cluster.name, sizeof(cluster.name), 1, file);

    fseek(file, ATTRIBUTES + pos, SEEK_SET);
    fread(&cluster.attributs, sizeof(cluster.attributs), 1, file);

    fseek(file, DATE + pos, SEEK_SET);
    fread(&cluster.date, sizeof(&cluster.date), 1, file);

    fseek(file, NEXT_CLUSTER_LOW + pos, SEEK_SET);
    fread(&low, sizeof(low), 1, file);

    fseek(file, NEXT_CLUSTER_HIGH + pos, SEEK_SET);
    fread(&high, sizeof(high), 1, file);
    cluster.nextCluster = (low << 16) | high;

    fseek(file, SIZE + pos, SEEK_SET);
    fread(&cluster.size, sizeof(cluster.size), 1, file);


    return cluster;
}


int searchDeepFile(FILE * file, uint32_t cluster, fat32 info, char * fileToFind, clusterData * result) {

    int i, j, found, long_name_counter, size;
    clusterData dir;
    lfn * long_file_names;

    size = info.sectorSize * info.sectorPerCluster / 32;
    found = long_name_counter = 0;
    long_file_names = malloc(sizeof(lfn));

    do {

        //ens situem al cluster
        uint64_t clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                              + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                              + info.sectorSize * (cluster - 2) * info.sectorPerCluster; // ens situem al cluster

        printf("Clusterpos: %"PRIu64"d", clusterPos);

        //per passar la primera volta del bucle
        dir.name[0] = 1;

        for (i = 0; i < size && dir.name[0] != 0; i++) {


            //llegim la informació del cluster
            dir = readCluster(file, clusterPos);
            showCluster(dir);

            if ((dir.attributs & 0x10) != 0) {
                //si es tracta d'un directori
                printf("5");

                if (strcmp(dir.name, ".          ") != 0 && strcmp(dir.name, "..         ") != 0) {
                    //si el directori no es dot o dotdot entrem a exploral recursivament
                    found = searchDeepFile(file, dir.nextCluster, info, fileToFind, result);
                }

                if (found) {
                    return found;
                }

            }

            if (dir.attributs == 0x0F) {
                //Es tracta d'un cluster amb info de long file name
                printf("long file name\n");
                long_file_names[long_name_counter++] = readLongFileName(file, clusterPos);
                long_file_names = realloc(long_file_names, sizeof(lfn) * (long_name_counter + 1));

            } else if ((unsigned char) dir.name[0] != 0 && (unsigned char) dir.name[0] != 0xE5) {
                //sino si no em arribat al final i el cluster no esta en desus
                printf("We got");

                if (long_name_counter) {
                    //si venim d'explorar clusters amb info de long file names
                    printf("4");

                    char * file_name = malloc(sizeof(char) * 13 * long_name_counter);

                    //ajuntem el contingut dels long file names per formar el nom de l'arxiu
                    for (j = 0; j < long_name_counter; j++) {
                        strcpy(file_name + j * 13, long_file_names[j].fileName);
                    }

                    printf("with long file name: %s\n", file_name);

                    if (strcmp(file_name, fileToFind) == 0) {
                        printf("I found the fucking file");
                        found = 1;
                        return found;
                    }

                    //resetejem per la següent busqueda
                    long_name_counter = 0;
                    free(long_file_names);
                    long_file_names = malloc(sizeof(lfn));
                }

            }

            //avançem per la cadena actual de clusters
            clusterPos += 32;
        }
        //s'ha acabat la cadena de clusters que estavem explorant

        //Anem al següent cluster
        fseek(file, info.sectorSize * info.reservedSectors + sizeof(uint32_t) * cluster, SEEK_SET);
        fread(&cluster, sizeof(uint32_t), 1, file);

        //eliminem el unused bits 32bits -> 28bits
        cluster = cluster & 0x0FFFFFFF;

    //Fins que estigui corrupte 0x0FFFFFF7, o hagi el limit
    } while (cluster < 0x0FFFFFF7);


    return 0; //not found
}


void searchFat32(FILE * file, char * fileToFind, int operation) {
    //llegim la info del volum
    fat32 info = readFat32(file);
    clusterData result;

    int what = searchDeepFile(file, info.rootFirstCluster, info, fileToFind, &result);

    printf("what: %d", what);


}
