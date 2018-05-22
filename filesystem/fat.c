//
// Created by Clàudia Peiró Vidal on 17/4/18.
//

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <time.h>
#include "fat.h"

#define SEARCH                          1
#define DEEP                            2
#define SHOW                            3
#define READ_CODE                       4
#define WRITE_CODE                      5
#define HIDE_CODE                       6
#define SHIDE_CODE                      7
#define DATE_CODE                       8

#define PRINTTREE                       1

//https://www.codeguru.com/cpp/cpp/cpp_mfc/files/article.php/c13907/Long-File-Name-LFN-Entries-in-the-FAT-Root-Directory-of-Floppy-Disks.htm#page-2


typedef struct _LongFileName
{
    uint8_t  seqNumber;
    uint8_t  fileName[13];

} lfn;


void concatLFN(char *name, lfn *pName, int size);

lfn readLongFileName (FILE * file, uint64_t pos) {
    lfn out;
    uint8_t fileName_Part1[10];
    uint8_t fileName_Part2[12];
    uint8_t fileName_Part3[4];
    int i = 0, j = 0;

    fseek(file, pos, SEEK_SET);
    fread(&(out.seqNumber), sizeof(uint8_t), 1, file);

    fseek(file, pos + sizeof(uint8_t), SEEK_SET);
    fread(fileName_Part1, sizeof(uint8_t) * 10, 1, file);

    fseek(file, pos + sizeof(uint8_t) * 13, SEEK_SET);
    fread(fileName_Part2, sizeof(uint8_t) * 12, 1, file);
    fseek(file, pos + sizeof(uint8_t) * 27, SEEK_SET);
    fread(fileName_Part3, sizeof(uint8_t) * 4, 1, file);

    for (i = 0; i < 13; i++) {

        if (j < 10) {
            out.fileName[i] = fileName_Part1[j];
        } else if (j >= 10 && j < 22) {
            out.fileName[i] = fileName_Part2[j - 9];
        } else {
            out.fileName[i] = fileName_Part3[j - 21];
        }
        j += 2;
    }
    //printf("Order: %d A part is: %s\n", out.seqNumber, out.fileName);
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

    return fat;
}

clusterData readCluster(FILE *file, uint32_t pos) {
    clusterData cluster;
    uint32_t low;
    uint16_t high;

    fseek(file, pos, SEEK_SET);
    fread(cluster.name, sizeof(cluster.name), 1, file);

    fseek(file, ATTRIBUTES + pos, SEEK_SET);
    fread(&cluster.attributs, sizeof(cluster.attributs), 1, file);

    fseek(file, DATE + pos, SEEK_SET);
    fread(&cluster.date, sizeof(&cluster.date), 1, file);

    fseek(file, NEXT_CLUSTER_LOW + pos, SEEK_SET);
    fread(&low, sizeof(uint16_t), 1, file);
    low <<= 16;

    fseek(file, NEXT_CLUSTER_HIGH + pos, SEEK_SET);
    fread(&high, sizeof(uint16_t), 1, file);

    cluster.nextCluster = (low & 0xFFFF0000) +  high;

    //cluster.nextCluster = ((((uint32_t)high) << 16) | (low));

    fseek(file, SIZE + pos, SEEK_SET);
    fread(&cluster.size, sizeof(cluster.size), 1, file);


    return cluster;
}

int searchFile(FILE * file, uint32_t cluster, fat32 info, char * fileToFind, clusterData * result) {
    int i, j, k, found, long_name_counter, size;
    clusterData clusterInfo;
    lfn * long_file_names;

    size = info.sectorSize * info.sectorPerCluster / 32;
    found = long_name_counter = 0;
    long_file_names = malloc(sizeof(lfn));

    do {

        //ens situem al cluster
        uint32_t clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                              + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                              + info.sectorSize * (cluster - 2) * info.sectorPerCluster; // ens situem al cluster

        //printf("Clusterpos: %"PRIu64"d", clusterPos);

        //per passar la primera volta del bucle
        clusterInfo.name[0] = 1;

        for (i = 0; i < size && clusterInfo.name[0] != 0; i++) {


            //llegim la informació del cluster
            clusterInfo = readCluster(file, clusterPos);



            if (clusterInfo.attributs == 0x0F) {
                //Es tracta d'un cluster amb info de long file name

                long_file_names[long_name_counter++] = readLongFileName(file, clusterPos);
                long_file_names = realloc(long_file_names, sizeof(lfn) * (long_name_counter + 1));

            } else if ((unsigned char) clusterInfo.name[0] != 0 && (unsigned char) clusterInfo.name[0] != 0xE5) {
                //sino si no em arribat al final i el cluster no esta en desus


                if (long_name_counter) {
                    //si venim d'explorar clusters amb info de long file names

                    char * file_name = malloc(sizeof(char) * 13 * size);;
                    concatLFN(file_name, long_file_names, long_name_counter);

                    //printf("with long file name: %s\n", file_name);

                    if (strcmp(file_name, fileToFind) == 0) {
                        //printf("I found the fucking file");
                        found = 1;
                        *result = clusterInfo;
                        return found;
                    }

                    //resetejem per la següent busqueda
                    long_name_counter = 0;
                    free(long_file_names);
                    long_file_names = malloc(sizeof(lfn));
                }


                if ((clusterInfo.attributs & 0x10) != 0) {
                    //si es tracta d'un directori

                    //com que no es deepsearch no entrem a directoris


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

void printFile (FILE * file, fat32 info, clusterData fileCluster) {
    int i = 0, cluster_explorer = 0;
    char c;
    uint32_t next_cluster;

    uint32_t cluster = fileCluster.nextCluster;

    uint32_t clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                          + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                          + info.sectorSize * (cluster - 2) * info.sectorPerCluster; // ens situem al cluster

    int cluster_size = info.sectorSize * info.sectorPerCluster;

    fseek(file, clusterPos, SEEK_SET);

    for (i = 0; i < fileCluster.size; i++) {

        fread(&c, sizeof(char), 1, file);

        printf("%c", c);

        if(c == '\0')
            break;

        if (cluster_explorer == cluster_size) {

            //Anem a la fat a buscar el següent cluster
            fseek(file, info.sectorSize * info.reservedSectors + sizeof(uint32_t) * cluster, SEEK_SET);
            fread(&next_cluster, sizeof(uint32_t), 1, file);

            //eliminem el unused bits 32bits -> 28bits
            next_cluster = next_cluster & 0x0FFFFFFF;

            if (next_cluster >= 0xFFFFFF7)
                break;

            //ens situem al cluster
            clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                                  + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                                  + info.sectorSize * (next_cluster - 2) * info.sectorPerCluster; // ens situem al cluster


            fseek(file, clusterPos, SEEK_SET);

            cluster = next_cluster;

            cluster_explorer = 0;
        }
        cluster_explorer++;
    }

}

int searchDeepFile(FILE * file, uint32_t cluster, fat32 info, char * fileToFind, clusterData * result) {
    int i, j, k, found, long_name_counter, size;
    clusterData clusterInfo;
    lfn * long_file_names;

    size = info.sectorSize * info.sectorPerCluster / 32;
    found = long_name_counter = 0;
    long_file_names = malloc(sizeof(lfn));

    do {
        //printf("CLUSTER %x \n", cluster);
        //ens situem al cluster
        uint32_t clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                              + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                              + info.sectorSize * (cluster - 2) * info.sectorPerCluster; // ens situem al cluster

        //per passar la primera volta del bucle
        clusterInfo.name[0] = 1;

        for (i = 0; i < size && clusterInfo.name[0] != 0; i++) {


            //llegim la informació del cluster
            clusterInfo = readCluster(file, clusterPos);
            //showCluster(clusterInfo);
            //printf("next cluster: %x \n", clusterInfo.nextCluster);


            if (clusterInfo.attributs == 0x0F) {
                //Es tracta d'un cluster amb info de long file name

                long_file_names[long_name_counter++] = readLongFileName(file, clusterPos);
                long_file_names = realloc(long_file_names, sizeof(lfn) * (long_name_counter + 1));

            } else if ((unsigned char) clusterInfo.name[0] != 0 && (unsigned char) clusterInfo.name[0] != 0xE5) {
                //sino si no em arribat al final i el cluster no esta en desus


                if (long_name_counter) {
                    //si venim d'explorar clusters amb info de long file names

                    char * file_name = malloc(sizeof(char) * 13 * size);;
                    concatLFN(file_name, long_file_names, long_name_counter);



                    //printf("with long file name: %s\n", file_name);

                    if (strcmp(file_name, fileToFind) == 0) {
                        //printf("I found the fucking file");

                        fseek(file, info.sectorSize * info.reservedSectors + sizeof(uint32_t) * cluster, SEEK_SET);
                        fread(&cluster, sizeof(uint32_t), 1, file);

                        *result = clusterInfo;
                        //printFile(file, info, clusterInfo);
                        return 1;
                    }

                    //resetejem per la següent busqueda
                    long_name_counter = 0;
                    free(long_file_names);
                    long_file_names = malloc(sizeof(lfn));
                } else {

                    for (k = 0; k < 8; k++) {
                        if (clusterInfo.name[k] == ' ') {
                            clusterInfo.name[k] = '.';
                            clusterInfo.name[++k] = clusterInfo.name[8];
                            clusterInfo.name[++k] = clusterInfo.name[9];
                            clusterInfo.name[++k] = clusterInfo.name[10];
                            clusterInfo.name[++k] = '\0';
                            break;
                        }
                    }

                    if (strcmp(clusterInfo.name, fileToFind) == 0) {
                        *result = clusterInfo;
                        //printFile(file, info, clusterInfo);
                        return 1;
                    }


                }


                if ((clusterInfo.attributs & 0x10) != 0) {
                    //si es tracta d'un directori

                    if (strcmp(clusterInfo.name, ".          ") != 0 && strcmp(clusterInfo.name, "..         ") != 0 &&(clusterInfo.name[0] != '.')) {
                        //si el directori no es dot o dotdot entrem a exploral recursivament

                        found = searchDeepFile(file, clusterInfo.nextCluster, info, fileToFind, result);

                    }

                    if (found) {
                        return found;
                    }

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

void concatLFN(char *name, lfn *pName, int size) {
    int i;

    for (i = 0; i < size/2; ++i)
    {
        lfn temp = pName[i];
        pName[i] = pName[size - 1 - i];
        pName[size - 1 - i] = temp;
    }
    for (i = 0; i < size; i++) {
       stpcpy(name + i * 13, pName[i].fileName);
    }

}

void searchFat32(FILE * file, char * fileToFind, int operation) {
    //llegim la info del volum
    int what = 0;
    clusterData result;
    fat32 info;

    info = readFat32(file);

    switch (operation) {
        case SEARCH:
            what = searchFile(file, info.rootFirstCluster, info, fileToFind, &result);
            break;
        case DEEP:
            what = searchDeepFile(file, info.rootFirstCluster, info, fileToFind, &result);
            break;
        default:
            perror("Undefined operation code.\n");
            return;
    }

    if (what) {
        printf("\nFile found! Size: %d bytes. Created at: %.2d/%.2d/%d \n", result.size, (result.date & 0x1F), ((result.date & 0x1E0) >> 5), 1980 + ((result.date & 0xFE00) >> 9));
    } else {
        printf("Error. File not found.");
    }



}

void showFile(FILE * file, char * fileToShow) {

    clusterData result;
    fat32 info;

    info = readFat32(file);

    if (searchDeepFile(file, info.rootFirstCluster, info, fileToShow, &result)) {
        printf("\nFile found! Showing content...\n\n");
        printFile(file, info, result);
    } else {
        printf("Error. File not found.");
    }

}

int searchFilePos(FILE * file, uint32_t cluster, fat32 info, char * fileToFind, uint32_t * file_position) {
    int i, j, k, found, long_name_counter, size;
    clusterData clusterInfo;
    lfn * long_file_names;

    size = info.sectorSize * info.sectorPerCluster / 32;
    found = long_name_counter = 0;
    long_file_names = malloc(sizeof(lfn));

    do {
        //printf("CLUSTER %x \n", cluster);
        //ens situem al cluster
        uint32_t clusterPos = info.sectorSize * info.reservedSectors //ens saltem els reservedblocs
                              + info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats  //ens saltem els fats
                              + info.sectorSize * (cluster - 2) * info.sectorPerCluster; // ens situem al cluster

        //per passar la primera volta del bucle
        clusterInfo.name[0] = 1;

        for (i = 0; i < size && clusterInfo.name[0] != 0; i++) {


            //llegim la informació del cluster
            clusterInfo = readCluster(file, clusterPos);
            //showCluster(clusterInfo);
            //printf("next cluster: %x \n", clusterInfo.nextCluster);


            if (clusterInfo.attributs == 0x0F) {
                //Es tracta d'un cluster amb info de long file name

                long_file_names[long_name_counter++] = readLongFileName(file, clusterPos);
                long_file_names = realloc(long_file_names, sizeof(lfn) * (long_name_counter + 1));

            } else if ((unsigned char) clusterInfo.name[0] != 0 && (unsigned char) clusterInfo.name[0] != 0xE5) {
                //sino si no em arribat al final i el cluster no esta en desus


                if (long_name_counter) {
                    //si venim d'explorar clusters amb info de long file names

                    char * file_name = malloc(sizeof(char) * 13 * size);;
                    concatLFN(file_name, long_file_names, long_name_counter);



                    //printf("with long file name: %s\n", file_name);

                    if (strcmp(file_name, fileToFind) == 0) {
                        //printf("I found the fucking file");

                        fseek(file, info.sectorSize * info.reservedSectors + sizeof(uint32_t) * cluster, SEEK_SET);
                        fread(&cluster, sizeof(uint32_t), 1, file);

                        *file_position = clusterPos;
                        //printFile(file, info, clusterInfo);
                        return 1;
                    }

                    //resetejem per la següent busqueda
                    long_name_counter = 0;
                    free(long_file_names);
                    long_file_names = malloc(sizeof(lfn));
                } else {

                    for (k = 0; k < 8; k++) {
                        if (clusterInfo.name[k] == ' ') {
                            clusterInfo.name[k] = '.';
                            clusterInfo.name[++k] = clusterInfo.name[8];
                            clusterInfo.name[++k] = clusterInfo.name[9];
                            clusterInfo.name[++k] = clusterInfo.name[10];
                            clusterInfo.name[++k] = '\0';
                            break;
                        }
                    }

                    if (strcmp(clusterInfo.name, fileToFind) == 0) {
                        *file_position = clusterPos;
                        //printFile(file, info, clusterInfo);
                        return 1;
                    }


                }


                if ((clusterInfo.attributs & 0x10) != 0) {
                    //si es tracta d'un directori

                    if (strcmp(clusterInfo.name, ".          ") != 0 && strcmp(clusterInfo.name, "..         ") != 0 &&(clusterInfo.name[0] != '.')) {
                        //si el directori no es dot o dotdot entrem a exploral recursivament

                        found = searchDeepFile(file, clusterInfo.nextCluster, info, fileToFind, file_position);

                    }

                    if (found) {
                        return found;
                    }

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

void changeAttributes (FILE * file, char * fileToChange, int operation) {

    fat32 info;
    uint32_t file_position;
    uint8_t attributs;

    info = readFat32(file);

    if (searchFilePos(file, info.rootFirstCluster, info, fileToChange, &file_position)) {

        fseek(file, file_position + ATTRIBUTES, SEEK_SET);
        fread(&attributs, sizeof(uint8_t), 1, file);

        fseek(file, file_position + ATTRIBUTES, SEEK_SET);

        switch (operation) {

            case READ_CODE:
                attributs |= 0x01;
                fwrite(&attributs, sizeof(uint8_t), 1, file);
                printf("Se han editado los permisos de %s\n", fileToChange);
                break;

            case WRITE_CODE:
                attributs &= 0x01;
                fwrite(&attributs, sizeof(uint8_t), 1, file);
                printf("Se han editado los permisos de %s\n", fileToChange);
                break;

            case HIDE_CODE:
                attributs |= 0x02;
                fwrite(&attributs, sizeof(uint8_t), 1, file);
                printf("Se han editado los permisos de %s\n", fileToChange);
                break;

            case SHIDE_CODE:
                attributs &= 0x02;
                fwrite(&attributs, sizeof(uint8_t), 1, file);
                printf("Se han editado los permisos de %s\n", fileToChange);
                break;

            default:
                perror("Undefined operation code.\n");
                break;
        }

    } else {

        printf("Error: File not found.\n");

    }

}

void changeFileDate(FILE * file, char * fileToChange, char date[8]) {

    fat32 info;
    uint32_t file_position;
    uint16_t formated_date;
    struct tm structtm;

    info = readFat32(file);

    if (searchFilePos(file, info.rootFirstCluster, info, fileToChange, &file_position)) {

            if (!strptime(date, "%d%m%Y", &structtm)) {

                formated_date = (uint16_t) ((structtm.tm_mday & 0x1F) | (((structtm.tm_mon) << 5) & 0x1E0)  | (((structtm.tm_year + 1980) << 9) & 0xFE00));

                fseek(file, file_position + DATE, SEEK_SET);
                fwrite(&formated_date, sizeof(structtm), 1, file);
                printf("La data de creación del fichero %s ha sido modificada.\n", fileToChange);
            } else {
                printf("Problema");
            }


    } else {

        printf("Error: File not found.\n");

    }

}