//
// Created by Clàudia Peiró Vidal on 13/4/18.
//


#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "ext.h"

#define DEPTH 0x06
#define EXTENT_TREE 0x28
#define UPPER_BLOCK 0x6
#define LOW_BLOCK 0x8
#define HEADER_SIZE 12
#define LENGTH_DIRECTORY   0x4
#define FILE_NAME 0x8
#define LENGTH_FILE 0x6

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

unsigned long exploreDirectory(__uint64_t posBlock, __uint64_t  in, char *fileToFind, int *found) {
    __uint16_t  var16;
    __uint32_t  var32, lowVar32, upperVar32;
    char * fileName;
    unsigned char length;

    fseek(file, (long)posBlock + LENGTH_FILE, SEEK_SET);
    fread(&length, sizeof(length), 1, file);

    fileName = (char *) malloc(sizeof(char) * ((int)length + 1));

    fseek(file, (long)posBlock + FILE_NAME, SEEK_SET);
    fread(fileName, sizeof(char) * ((int)length + 1), 1, file);

    fileName[(int)length] = '\0';
    printf("File name %s is not %s\n", fileName, fileToFind);

    if (strcmp(fileName, fileToFind) == 0) {
        *found = 1;
        fseek(file, (long)in + 0x90, SEEK_SET);
        fread(&var32, sizeof(var32), 1, file);

        fseek(file, (long)in + 0x4, SEEK_SET);
        fread(&lowVar32, sizeof(lowVar32), 1, file);

        fseek(file,(long) in + 0x6C, SEEK_SET);
        fread(&upperVar32, sizeof(upperVar32), 1, file);

        __uint64_t size = (__uint64_t) upperVar32 << 32 | lowVar32;

        time_t ts = var32;

        char buff[100];

        strftime(buff, 100, "%d-%m-%Y", localtime(&ts));
        printf("File Found! Size: %lli Created on: %s\n", size, buff);

        return posBlock;
    }

    fseek(file, (long)posBlock + LENGTH_DIRECTORY, SEEK_SET);
    fread(&var16, sizeof(var16), 1, file);

    posBlock += var16;

    fseek(file, (long)posBlock, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);
    return posBlock;
}

int findFileinDirectory(__uint64_t posBlock, __uint64_t in, char *fileToFind) {

    int found = 0;
    __uint32_t var32;

    posBlock = exploreDirectory(posBlock, in, fileToFind, &found);

    fseek(file, (long)posBlock, SEEK_SET);
    fread(&var32, sizeof(var32), 1, file);

    if (var32 != 0 && found == 0) {
        found = findFileinDirectory(posBlock, in, fileToFind);
    } else {
        printf("Inode result %u\n", var32);
        return found;
    }
    return found;

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

unsigned long findBlock( __uint64_t in, int blockSize, int operation, leaf * toExploreLeaf) {
    __uint16_t depth;

    __uint16_t upperBlock;
    __uint32_t lowBlock;
    unsigned long posBlock;

    fseek(file, (long)in + DEPTH, SEEK_SET); //Depth
    fread(&depth, sizeof(depth), 1, file);


    if (depth == 0) {
        //Upper block
        fseek(file, (long)in + HEADER_SIZE + UPPER_BLOCK, SEEK_SET);
        fread(&upperBlock, sizeof(upperBlock), 1, file);

        //Lower block
        fseek(file, (long)in + HEADER_SIZE + LOW_BLOCK, SEEK_SET);
        fread(&lowBlock, sizeof(lowBlock), 1, file);

        //Block
        posBlock = (__uint64_t) upperBlock << 32 | lowBlock;

        posBlock *= blockSize;

        return posBlock;
    }

    if (operation == SEARCH_OPERATION) {
        return 0;
    } else {
        //Upper block
        fseek(file, (long)in + HEADER_SIZE + 0x8, SEEK_SET); //upper
        fread(&upperBlock, sizeof(upperBlock), 1, file);

        //Lower block
        fseek(file, (long)in + HEADER_SIZE + 0x4, SEEK_SET); //LOW
        fread(&lowBlock, sizeof(lowBlock), 1, file);

        //Block
        posBlock =  (__uint64_t) upperBlock << 32 | lowBlock;
        posBlock *= blockSize;
        return findBlock(posBlock, blockSize, operation, toExploreLeaf);
    }

}

void exploreInDirectory(char * fileToFind, int operation, ext4 ext4Info) {

    leaf rootLeaf;
    int found = 0;
    __uint16_t inodeSize = ext4Info.inode.inodeSize;

    __uint64_t in = ext4Info.posInodeTable * ext4Info.blockSize + inodeSize;

    printf("MaxEntries %llu\n", in);

    rootLeaf.actualPosBlock = findBlock(in + EXTENT_TREE, ext4Info.blockSize, operation, &rootLeaf);

    //Busco maxEntries
    fseek(file, (long)in + EXTENT_TREE + 0x2, SEEK_SET);
    fread(&rootLeaf.maxEntries, sizeof(rootLeaf.maxEntries), 1, file);


    rootLeaf.fatherPosBlock = rootLeaf.actualPosBlock;

    if (operation == SEARCH_OPERATION) {
        found = findFileinDirectory(rootLeaf.actualPosBlock, in, fileToFind);
        if (found == 0) {
            printf(FILE_NOT_FOUND);
        }

    }
}


void exploreExtentTree (char * fileToFind, ext4 ext4Info, __uint32_t nextInode, int *found) {

    __uint16_t inodeSize = ext4Info.inode.inodeSize;
    __uint64_t posicioReal = ext4Info.posInodeTable * ext4Info.blockSize + inodeSize * (nextInode - 1);

    __uint16_t depth, start_hi, ee_len, rec_len, max_entries;
    __uint32_t start_lo;

    __uint64_t offset;
    __uint64_t posicioBlock, directoryEnd;

    __uint8_t directoryInformation, midaNom;

    char * fileName = (char *) malloc(sizeof(char) * (1));

    posicioReal += EXTENT_TREE;

    //Busquem MaxEntries
    fseek(file, (long)posicioReal + 0x2, SEEK_SET);
    fread(&max_entries, sizeof(max_entries), 1, file);


    int i;
    for (i = 0; i < max_entries && !*found; i++) {
        //Busquem Depth
        fseek(file, (long)posicioReal + 0x6, SEEK_SET);
        fread(&depth, sizeof(depth), 1, file);

        printf("Depth %u\n", depth);

        if (depth == 0) {

            //Per cada fulla processem els blocs de cadascuna
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x6, SEEK_SET);
            fread(&start_hi, sizeof(start_hi), 1, file);

            fseek(file, (long)posicioReal + HEADER_SIZE + 0x8, SEEK_SET);
            fread(&start_lo, sizeof(start_lo), 1, file);

            posicioBlock = (__uint64_t) start_hi << 32 | start_lo;

            printf("Posicio Block %llu\n", posicioBlock);

            offset = posicioBlock * ext4Info.blockSize;

            //Number of blocks covered by extent
            fseek(file, (long)posicioReal + HEADER_SIZE + 0x4, SEEK_SET);
            fread(&ee_len, sizeof(ee_len), 1, file);

            directoryEnd = offset + ee_len * ext4Info.blockSize;

            printf("Directory end %llu %d\n", directoryEnd, *found);

            while (offset <= directoryEnd && !(*found)) {
                //Next Inode
                fseek(file, (long)offset, SEEK_SET);
                fread(&nextInode, sizeof(nextInode), 1, file);

                printf("Next inode %d\n", nextInode);

                if (nextInode != 0) {

                    fseek(file, (long)offset + 0x6, SEEK_SET);
                    fread(&midaNom, sizeof(midaNom), 1, file);

                    fileName = (char *)realloc(fileName, midaNom);

                    fseek(file, (long)offset  + 0x8, SEEK_SET);
                    fread(fileName, sizeof(char) * ((int)midaNom + 1), 1, file);

                    fileName[(int)midaNom] = '\0';


                    if (strcmp(fileName, ".") == 0) {
                        //Saltem . i ..
                        fseek(file, (long)offset + 0x4, SEEK_SET);
                        fread(&rec_len, sizeof(rec_len), 1, file);
                        offset += rec_len;

                        fseek(file, (long)offset + 0x4, SEEK_SET);
                        fread(&rec_len, sizeof(rec_len), 1, file);
                        offset += rec_len;
                    }

                    fseek(file, (long)offset + 0x4, SEEK_SET);
                    fread(&rec_len, sizeof(rec_len), 1, file);


                    //Mirem si es fitxer o si es directory
                    fseek(file, (long)offset + 0x7, SEEK_SET);
                    fread(&directoryInformation, sizeof(directoryInformation), 1, file);

                    switch (directoryInformation) {
                        case 1:

                            printf("És fitxer\n");
                            //Fitxer
                            fseek(file, (long)offset + 0x6, SEEK_SET);
                            fread(&midaNom, sizeof(midaNom), 1, file);

                            fileName = (char *)realloc(fileName, midaNom);

                            fseek(file, (long)offset  + 0x8, SEEK_SET);
                            fread(fileName, sizeof(char) * ((int)midaNom + 1), 1, file);

                            fileName[(int)midaNom] = '\0';

                            printf("Filename found: %s, Filename expected: %s\n", fileName, fileToFind);
                            if (strcmp(fileName, fileToFind) != 0) {
                                //Mirem següent
                                offset += rec_len;

                            } else {
                                printf("File Found! %s\n", fileName);
                                *found = 1;
                                break;
                            }
                            break;

                        case 2:

                            printf("És directori \n");

                            fseek(file, (long)offset + 0x6, SEEK_SET);
                            fread(&midaNom, sizeof(midaNom), 1, file);

                            fileName = (char *)realloc(fileName, midaNom);

                            fseek(file, (long)offset  + 0x8, SEEK_SET);
                            fread(fileName, sizeof(char) * ((int)midaNom + 1), 1, file);

                            fileName[(int)midaNom] = '\0';

                            printf("Directory found: %s\n", fileName);

                            //Next Inode
                            fseek(file, (long)offset, SEEK_SET);
                            fread(&nextInode, sizeof(nextInode), 1, file);

                            printf("Next inode To explore %d\n", nextInode);

                            exploreExtentTree(fileToFind, ext4Info, nextInode, found);

                            break;

                        default:

                            break;
                    }

                } else {
                    break;
                }

            }





        }

    }




}

/**
 * Primer haurem de buscar els group descriptors
 * @param file
 */
void searchExt4(FILE * files, char * fileToFind, int operation) {
    int posGroupDescrip;

    file = files;
    ext4 ext4Info = readExt4(file);
    int blockSize = (int)pow(2, (10 + ext4Info.block.blockSize));

    //posicio group descriptor
    posGroupDescrip = blockSize == EXT_PADDING_SUPER_BLOCK ? 2*EXT_PADDING_SUPER_BLOCK:blockSize;

    getPosInodeTable (&ext4Info, posGroupDescrip);

    ext4Info.blockSize = blockSize;

    if (operation == SEARCH_OPERATION) {
        exploreInDirectory(fileToFind, operation, ext4Info);
    } else {
        int found = 0;
        exploreExtentTree(fileToFind, ext4Info, 2, &found);

        if (!found) {
            printf("File not found\n");
        }
    }


}



