//
// Created by Clàudia Peiró Vidal on 13/4/18.
//


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "ext.h"


#define EXTENT_TREE 0x28
#define HEADER_SIZE 12

#define SEARCH_OPERATION 1
#define SHOW 3

#define FILE_NOT_FOUND "Error. File not found.\n"

FILE * file;

void showExt(ext4 ext) {
    printf("---- Filesystem Information ----\n\n");

    printf("Filesystem: EXT4\n\n");

    printf("INODE INFO\n");
    printf("Inode Size: %u\n", ext.inode.inodeSize);
    printf("Number of Inodes: %d\n", ext.inode.numberOfInodes);
    printf("First Inode: %d\n", ext.inode.firstInodeM);
    printf("Inodes Group: %d\n", ext.inode.inodesGroup);
    printf("Free Inodes: %d\n\n", ext.inode.freeInodes);

    printf("BLOCK INFO\n");
    printf("Block Size: %d\n", (int)pow(2, (10 + ext.block.blockSize)));
    printf("Reserved Blocks: %d\n", ext.block.reservedBlocks);
    printf("Free Blocks: %d\n", ext.block.freeBlocks);
    printf("Total Blocks: %d\n", ext.block.totalBlocks);
    printf("First BLock: %d\n", ext.block.firstBlock);
    printf("Block group: %d\n", ext.block.blockGroup);
    printf("Frags group: %d\n\n", ext.block.fragsGroup);


    printf("VOLUME INFO\n");
    printf("Volume name: %s\n", ext.volume.volumeName);

    time_t ts = ext.volume.lastCheck;
    printf("Last check: %s", ctime(&ts));

    ts = ext.volume.lastMount;
    printf("Last mount: %s", ctime(&ts));

    ts = ext.volume.lastWritten;
    printf("Last written: %s\n", ctime(&ts));

}

ext4 readExt4(FILE *file) {
    ext4 ext;

    // READ INODE
    fseek(file, INODE_SIZE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.inodeSize, sizeof(ext.inode.inodeSize), 1, file);

    fseek(file, EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.numberOfInodes, sizeof(ext.inode.numberOfInodes), 1, file);

    fseek(file, FIRST_INODE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.firstInodeM, sizeof(ext.inode.firstInodeM), 1, file);

    fseek(file, INODE_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.inodesGroup, sizeof(ext.inode.inodesGroup), 1, file);

    fseek(file, FREE_INODES + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.inode.freeInodes, sizeof(ext.inode.freeInodes), 1, file);


    //READ BLOCK
    fseek(file, BLOCK_SIZE + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.blockSize, sizeof(ext.block.blockSize), 1, file);

    fseek(file, RESERVED_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.reservedBlocks, sizeof(ext.block.reservedBlocks), 1, file);

    fseek(file, FREE_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.freeBlocks, sizeof(ext.block.freeBlocks), 1, file);

    fseek(file, TOTAL_BLOCKS + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.totalBlocks, sizeof(ext.block.totalBlocks), 1, file);

    fseek(file, FIRST_BLOCK + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.firstBlock, sizeof(ext.block.firstBlock), 1, file);

    fseek(file, BLOCK_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.blockGroup, sizeof(ext.block.blockGroup), 1, file);

    fseek(file, FRAGS_GROUP + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.block.fragsGroup, sizeof(ext.block.fragsGroup), 1, file);


    //READ VOLUME
    fseek(file, VOLUME_NAME + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(ext.volume.volumeName, sizeof(ext.volume.volumeName), 1, file);

    fseek(file, LAST_CHECK + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastCheck, sizeof(ext.volume.lastCheck), 1, file);

    fseek(file, LAST_MOUNT + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastMount, sizeof(ext.volume.lastMount), 1, file);

    fseek(file, LAST_WRITTEN + EXT_PADDING_SUPER_BLOCK, SEEK_SET);
    fread(&ext.volume.lastWritten, sizeof(ext.volume.lastWritten), 1, file);

    return ext;
}

void getPosInodeTable (ext4 *ext4Info, int posGroupDescrip) {
    __uint32_t posLowInodeTable;
    __uint32_t posHighInodeTable;

    //Lower 32 bits de inode table
    fseek(file, 0x8 + posGroupDescrip, SEEK_SET);
    fread(&posLowInodeTable, sizeof(posLowInodeTable), 1, file);

    //high 32 bits de inode table
    fseek(file, 0x24 + posGroupDescrip, SEEK_SET);
    fread(&posHighInodeTable, sizeof(posHighInodeTable), 1, file);

    ext4Info->posInodeTable = (__uint64_t)posHighInodeTable << 32 | posLowInodeTable;
}

void showContentofFile(uint64_t offset, __uint64_t ee_len) {

    char c = 0;
    __uint64_t i = 0;

    printf("File Found! Showing content...\n");

    fseek(file, offset, SEEK_SET);

    for (i = 0; i < ee_len; i++) {
        fread(&c, sizeof(char), 1, file);
        printf("%c", c);
        if(c == '\0')break;
    }



}

void findStartOfFile (__uint32_t inode, ext4 ext4Info, __uint64_t posicioReal, int i) {

    __uint16_t start_hi, depth, entries, ee_len = 0;
    __uint32_t start_lo;

    __uint64_t offset, posicioBlock;

    //Busquem Depth
    fseek(file, (long)posicioReal + 0x6, SEEK_SET);
    fread(&depth, sizeof(depth), 1, file);

    if (depth == 0) {

        //Busquem Depth
        fseek(file, (long)posicioReal + 0x2, SEEK_SET);
        fread(&entries, sizeof(entries), 1, file);

        int a = 0;

        for (a = 0; a < entries; a++) {

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x4, SEEK_SET);
            fread(&ee_len, sizeof(ee_len), 1, file);
            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            posicioBlock = (__uint64_t) start_hi << 32 | start_lo;

            offset = posicioBlock * ext4Info.blockSize;

            showContentofFile(offset, (__uint64_t) (ee_len * ext4Info.blockSize));

            posicioReal += HEADER_SIZE;

        }

    } else {

        //Per cada fulla processem els blocs de cadascuna
        fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
        fread(&start_hi, sizeof(start_hi), 1, file);

        fseek(file, (long)posicioReal + HEADER_SIZE + 0x4, SEEK_SET);
        fread(&start_lo, sizeof(start_lo), 1, file);

        posicioBlock = (__uint64_t) start_hi << 32 | start_lo;

        fseek(file, (long)posicioReal + HEADER_SIZE + 0x2, SEEK_SET);
        fread(&start_hi, sizeof(start_hi), 1, file);

        offset = posicioBlock * ext4Info.blockSize;

        findStartOfFile(inode, ext4Info, offset, i);

    }

}

void fileFoundInformation(__uint64_t offset) {

    __uint32_t var32, lowVar32, upperVar32;

    fseek(file, (long)offset + 0x90, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);

    fseek(file, (long)offset + 0x4, SEEK_SET);
    fread(&lowVar32, sizeof(lowVar32), 1, file);

    fseek(file,(long) offset + 0x6C, SEEK_SET);
    fread(&upperVar32, sizeof(upperVar32), 1, file);

    __uint64_t size =  (__uint64_t) upperVar32 << 32 | lowVar32;

    time_t ts = var32;

    char buff[100];

    strftime(buff, 100, "%d-%m-%Y", localtime(&ts));
    printf("File Found! Size: %lli Created on: %s\n", size, buff);
}

void showPermissos (__uint64_t offset) {
    char read = 'r';
    char write = 'w';
    char execute = 'x';

    __uint16_t comprovacio;

    fseek(file, offset, SEEK_SET);
    fread(&comprovacio, sizeof(comprovacio), 1, file);


    if (comprovacio & 0x01) {
        printf("%c", execute);
    } else {
        printf("-");
    }

    if (comprovacio & 0x02) {
        printf("%c", write);
    } else {
        printf("-");
    }

    if (comprovacio & 0x04) {
        printf("%c", read);
    } else {
        printf("-");
    }

    if (comprovacio & 0x08) {
        printf("%c", execute);
    } else {
        printf("-");
    }

    if (comprovacio & 0x10) {
        printf("%c", write);
    } else {
        printf("-");
    }

    if (comprovacio & 0x20) {
        printf("%c", read);
    } else {
        printf("-");
    }

    if (comprovacio & 0x40) {
        printf("%c", execute);
    } else {
        printf("-");
    }

    if (comprovacio & 0x80) {
        printf("%c", write);
    } else {
        printf("-");
    }

    if (comprovacio & 0x100) {
        printf("%c\n", read);
    } else {
        printf("-\n");
    }



}

void changeRead(__uint64_t offset){

    __uint16_t permisos;

    fseek(file, offset, SEEK_SET);
    fread(&permisos, sizeof(permisos), 1, file);

    printf("Permits before modifications: ");
    showPermissos(offset);

    permisos |= 0x124;

    fseek(file, offset, SEEK_SET);
    fwrite(&permisos, sizeof(permisos), 1, file);

    printf("Permits after modifications: ");
    showPermissos(offset);

}

void changeReadOff(__uint64_t offset){

    __uint16_t permisos;

    fseek(file, offset, SEEK_SET);
    fread(&permisos, sizeof(permisos), 1, file);

    printf("Permits before modifications: ");
    showPermissos(offset);

    permisos &= 0xFEDB;

    fseek(file, offset, SEEK_SET);
    fwrite(&permisos, sizeof(permisos), 1, file);

    printf("Permits after modifications: ");
    showPermissos(offset);

}

void changeCreationTime(__uint64_t offset, char * date) {
    __uint32_t var32;

    fseek(file, (long)offset + 0x90, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);

    time_t ts = var32;

    char buff[100], buff2[100];

    strftime(buff, 100, "%d-%m-%Y", localtime(&ts));
    printf("Creation time before modification %s\n", buff);

    char year[5];
    char day[3];
    char month[3];

    int i = 0;

    for (i = 0; i < 2; i++) {
        day[i] = date[i];
    }
    day[i] = '\0';

    int a = 0;
    for (i = 2; i < 4; i++) {
        month[a] = date[i];
        a++;
    }
    month[a] = '\0';

    a = 0;
    for (i = 4; i < 8; i++) {
        year[a] = date[i];
        a++;
    }
    year[a] = '\0';


    printf("Year: %s, month: %s, day: %s\n", year, month, day);

    struct tm t;
    time_t t_of_day;

    t.tm_year = atoi(year) - 1900;
    t.tm_mon = atoi(month) - 1;           // Month, 0 - jan
    t.tm_mday = atoi(day);          // Day of the month
    t_of_day = mktime(&t);

    strftime(buff2, 100, "%d-%m-%Y", localtime(&t_of_day));
    printf("Creation time suposed modification %s\n", buff2);


    /*fseek(file, (long)offset + 0x90, SEEK_SET);
    fwrite(&t_of_day, sizeof(t_of_day), 1, file);

    fseek(file, (long)offset + 0x90, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);

    ts = var32;

    strftime(buff, 100, "%d-%m-%Y", localtime(&ts));
    printf("Creation time after modification %s\n", buff);*/



}



void exploreExtentTree (char * fileToFind, ext4 ext4Info, __uint32_t inode, int *found, char * date, int operation) {


    __uint16_t inodeSize = ext4Info.inode.inodeSize;
    __uint64_t posicioReal = ext4Info.posInodeTable * ext4Info.blockSize + inodeSize * (inode - 1);

    __uint16_t depth, start_hi, ee_len, rec_len, max_entries;
    __uint32_t start_lo, nextInode;

    __uint64_t offset;
    __uint64_t posicioBlock, directoryEnd;

    __uint8_t directoryInformation, midaNom;

    char * fileName = (char *) malloc(sizeof(char) * (1));

    posicioReal += EXTENT_TREE;

    //Busquem Max Entries
    fseek(file, (long)posicioReal + 0x2, SEEK_SET);
    fread(&max_entries, sizeof(max_entries), 1, file);


    int i;
    for (i = 0; i < max_entries && !*found; i++) {
        //Busquem Depth
        fseek(file, (long)posicioReal + 0x6, SEEK_SET);
        fread(&depth, sizeof(depth), 1, file);


        if (depth == 0) {

            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            posicioBlock = (__uint64_t) start_hi << 32 | start_lo;

            offset = posicioBlock * ext4Info.blockSize;

            //Number of blocks covered by extent
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x4, SEEK_SET);
            fread(&ee_len, sizeof(ee_len), 1, file);

            directoryEnd = offset + ee_len * ext4Info.blockSize;

            while (offset <= directoryEnd && !(*found)) {

                //Next Inode
                fseek(file, (long)offset, SEEK_SET);
                fread(&nextInode, sizeof(nextInode), 1, file);


                if (nextInode != 0) {

                    fseek(file, (long)offset + 0x6, SEEK_SET);
                    fread(&midaNom, sizeof(midaNom), 1, file);

                    fileName = (char *)realloc(fileName, midaNom + 1);

                    fseek(file, (long)offset  + 0x8, SEEK_SET);
                    fread(fileName, sizeof(char) * ((int)midaNom + 1), 1, file);

                    fileName[(int)midaNom] = '\0';


                    fseek(file, (long)offset + 0x4, SEEK_SET);
                    fread(&rec_len, sizeof(rec_len), 1, file);


                    if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
                        //Saltem . i ..
                        offset += rec_len;
                        continue;
                    }

                    //Mirem si es fitxer o si es directory
                    fseek(file, (long)offset + 0x7, SEEK_SET);
                    fread(&directoryInformation, sizeof(directoryInformation), 1, file);


                    switch (directoryInformation) {
                        case 1:
                            printf("Filename found: %s, Filename expected: %s\n", fileName, fileToFind);
                            if (strcmp(fileName, fileToFind) == 0) {
                                posicioBlock = ext4Info.posInodeTable * ext4Info.blockSize +
                                               inodeSize * (nextInode - 1);

                                switch (operation) {
                                    case DEEP:
                                        fileFoundInformation(posicioBlock);

                                        break;
                                    case SHOW:
                                        posicioBlock += EXTENT_TREE;
                                        int a = 0;
                                        findStartOfFile(nextInode, ext4Info, posicioBlock, a);

                                        break;
                                    case READ_CODE:
                                        printf("Changing read permits - Read on\n");
                                        changeRead(posicioBlock);

                                        break;
                                    case WRITE_CODE:
                                        printf("Changing read permits - Read off\n");
                                        changeReadOff(posicioBlock);

                                        break;
                                    case DATE_CODE:
                                        printf("date %s\n", date);
                                        changeCreationTime(posicioBlock, date);

                                        break;
                                }
                                *found = 1;

                            }

                            break;

                        case 2:
                            printf("Directory found: %s\n", fileName);
                            exploreExtentTree(fileToFind, ext4Info, nextInode, found, date,operation);

                            break;

                        default:
                            printf("Default %u\n", directoryInformation);

                            break;
                    }

                    offset += rec_len;

                } else {
                    break;
                }
            }
        } else {

            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            offset = (__uint64_t) start_hi << 32 | start_lo;
        }

    }
}

void exploreExtentTreeDirectori (char * fileToFind, ext4 ext4Info, int *found){


    __uint16_t inodeSize = ext4Info.inode.inodeSize;
    __uint64_t posicioReal = ext4Info.posInodeTable * ext4Info.blockSize + inodeSize;

    __uint16_t depth, start_hi, ee_len, rec_len, max_entries;
    __uint32_t start_lo, nextInode;

    __uint64_t offset;
    __uint64_t posicioBlock, directoryEnd;

    __uint8_t directoryInformation, midaNom;

    char * fileName = (char *) malloc(sizeof(char) * (1));

    posicioReal += EXTENT_TREE;

    //Busquem Max Entries
    fseek(file, (long)posicioReal + 0x2, SEEK_SET);
    fread(&max_entries, sizeof(max_entries), 1, file);


    int i;
    for (i = 0; i < max_entries && !*found; i++) {
        //Busquem Depth
        fseek(file, (long)posicioReal + 0x6, SEEK_SET);
        fread(&depth, sizeof(depth), 1, file);


        if (depth == 0) {

            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            posicioBlock = (__uint64_t) start_hi << 32 | start_lo;

            offset = posicioBlock * ext4Info.blockSize;

            //Number of blocks covered by extent
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x4, SEEK_SET);
            fread(&ee_len, sizeof(ee_len), 1, file);

            directoryEnd = offset + ee_len * ext4Info.blockSize;

            while (offset <= directoryEnd + 1 && !(*found)) {

                printf("Entro amb offset %llu, %llu\n", offset, directoryEnd);

                //Next Inode
                fseek(file, (long)offset, SEEK_SET);
                fread(&nextInode, sizeof(nextInode), 1, file);


                if (nextInode != 0) {

                    fseek(file, (long)offset + 0x6, SEEK_SET);
                    fread(&midaNom, sizeof(midaNom), 1, file);

                    fileName = (char *)realloc(fileName, midaNom + 1);

                    fseek(file, (long)offset  + 0x8, SEEK_SET);
                    fread(fileName, sizeof(char) * ((int)midaNom + 1), 1, file);

                    fileName[(int)midaNom] = '\0';


                    fseek(file, (long)offset + 0x4, SEEK_SET);
                    fread(&rec_len, sizeof(rec_len), 1, file);

                    printf("File %s\n", fileName);

                    if (strcmp(fileName, ".") == 0 || strcmp(fileName, "..") == 0) {
                        //Saltem . i ..
                        offset += rec_len;
                        continue;
                    }

                    //Mirem si es fitxer o si es directory
                    fseek(file, (long)offset + 0x7, SEEK_SET);
                    fread(&directoryInformation, sizeof(directoryInformation), 1, file);


                    switch (directoryInformation) {
                        case 1:
                            printf("Filename found: %s, Filename expected: %s\n", fileName, fileToFind);
                            if (strcmp(fileName, fileToFind) == 0) {

                                posicioBlock = ext4Info.posInodeTable * ext4Info.blockSize +
                                               inodeSize * (nextInode - 1);
                                fileFoundInformation(posicioBlock);

                                *found = 1;

                            }

                            break;

                        case 2:
                            printf("Directory found: %s\n", fileName);
                            break;

                    }

                    offset += rec_len;

                    printf("Offset %llu, dire %llu\n", offset, directoryEnd);

                } else {
                    break;
                }
            }
        } else {

            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            offset = (__uint64_t) start_hi << 32 | start_lo;
        }

    }




}

/**
 * Primer haurem de buscar els group descriptors
 * @param file
 */
void searchExt4(FILE * files, char * fileToFind, char * date ,int operation) {
    int posGroupDescrip;

    file = files;
    ext4 ext4Info = readExt4(file);
    int blockSize = (int)pow(2, (10 + ext4Info.block.blockSize));

    //posicio group descriptor
    posGroupDescrip = blockSize == EXT_PADDING_SUPER_BLOCK ? 2*EXT_PADDING_SUPER_BLOCK:blockSize;

    getPosInodeTable (&ext4Info, posGroupDescrip);

    ext4Info.blockSize = blockSize;
    int found = 0;

    if (operation == SEARCH_OPERATION) {
        exploreExtentTreeDirectori(fileToFind, ext4Info, &found);

    } else {
        exploreExtentTree(fileToFind, ext4Info, 2, &found, date, operation);

    }

    if (!found) {
        printf(FILE_NOT_FOUND);
    }


}



