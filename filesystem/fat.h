//
// Created by Clàudia Peiró Vidal on 17/4/18.
//

#ifndef RAGNAROK_FAT_H
#define RAGNAROK_FAT_H


#include <stdio.h>

#define SYSTEM_NAME                  0x03
#define SECTOR_SIZE                  0x0B
#define SECTOR_CLUSTER               0x0D
#define RESERVED_SECTORS             0x0E
#define NUMBER_OF_FATS               0x10
#define ROOT_ENTRIES                 0x11
#define NUMBER_FATS_SECTOR           0x24
#define LABEL                        0x47

typedef struct fat32Information {
    char systemName[11];
    __uint16_t sectorSize;
    unsigned char sectorCluster;
    __uint16_t reservedSectors;
    __uint32_t numberOfFats;
    __uint16_t maximumRootEntries;
    __uint16_t numberOfFatsPerSector;
    char label[9];
} fat32;

void readFat32(FILE *file);

#endif //RAGNAROK_FAT_H
