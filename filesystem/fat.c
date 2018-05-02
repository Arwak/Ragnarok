//
// Created by Clàudia Peiró Vidal on 17/4/18.
//

#include <string.h>
#include "fat.h"

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

clusterData readCluster(FILE *file, __uint32_t pos) {
    clusterData cluster;
    __uint16_t low;
    __uint32_t high;

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


void searchInside(FILE * file, int cluster, fat32 info, char * fileToFind) {

    __uint32_t clusterPos = info.sectorSize * info.reservedSectors + //ens saltem els reservedblocs
            info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats + //ens saltem els fats
            info.sectorSize * info.sectorPerCluster * ( cluster - 2); // ens situem al cluster

    clusterData clusterInfo = readCluster(file, clusterPos);
    showCluster(clusterInfo);

    if (clusterInfo.name[0] == 0) {
        //done

        __uint32_t next = (__uint32_t) (info.sectorSize * info.reservedSectors + cluster * 4);

        if ((next & 0x0FFFFFFF) >= 0x0FFFFFF8) {

            printf("final");

        } else if ((next & 0x0FFFFFFF) == 0x0FFFFFF7) {

            printf("cluster corrupte");

        } else {

            searchInside(file,next,info, fileToFind);

        }


    } else {

        if ((clusterInfo.attributs & 0x0F) > 0) {
            //LFN
        }

        if ((clusterInfo.attributs & 0x10) > 0) {
            printf("Es un directori");
            do {

                clusterInfo = readCluster(file, clusterInfo.nextCluster);
                showCluster(clusterInfo);
            } while ((clusterInfo.attributs & 0x10) > 0);

        } else {
            if (strcmp(fileToFind, clusterInfo.name) == 0) {
                printf("File found");
            }

        }

    }


}



void searchFile(FILE * file, char * fileToFind, int clusterNum, fat32 info) {
    __uint32_t clusterPos = info.sectorSize * info.reservedSectors + //ens saltem els reservedblocs
                            info.sectorSize * info.numberOfSectorsPerFat * info.numberOfFats + //ens saltem els fats
                            info.sectorSize * info.sectorPerCluster * ( clusterNum - 2); // ens situem al cluster

    clusterData clusterInfo;

    do {
        clusterInfo = readCluster(file, clusterPos);

        if ((clusterInfo.attributs & 0x0F) > 0) {
            //LFN
            printf("long name");

        } else if ((clusterInfo.attributs & 0x10) > 0) {
            // directori
            searchFile(file, fileToFind, clusterInfo.nextCluster, info);

        } else {
            //file
            if (strcmp(fileToFind, clusterInfo.name) == 0) {
                printf("File found [%s] matches input [%s].\n", clusterInfo.name, fileToFind);

            } else {
                printf("File [%s] not matches input [%s].\n", clusterInfo.name, fileToFind);
                searchFile(file, fileToFind, clusterInfo.nextCluster, info);
            }

        }
    } while (clusterInfo.nextCluster < 0x0FFFFFF8);


}



void searchFat32(FILE * file, char * fileToFind, int operation) {
    //llegim la info del volum
    fat32 info = readFat32(file);

    searchInside(file, info.rootFirstCluster, info, fileToFind);



}