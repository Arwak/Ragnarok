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

    //ens posem a la posició del depth per mirar si es fulla
    //indexInode = posInodeTable * blockSize * manyInode + inodeSize + EXTENT_TREE;
    //if (operation == SEARCH_OPERATION) {
        //in += EXTENT_TREE;
    //}

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

void deepSearch(char * fileToFind,  ext4 ext4Info, leaf toExploreLeaf, int *found, int operation);

char * printName (unsigned long posBlock) {
    char * fileName;
    unsigned char length;

    fseek(file, posBlock + LENGTH_FILE, SEEK_SET);
    fread(&length, sizeof(length), 1, file);

    fileName = (char *) malloc(sizeof(char) * ((int)length + 1));

    fseek(file, posBlock + FILE_NAME, SEEK_SET);
    fread(fileName, sizeof(char) * ((int)length + 1), 1, file);

    fileName[(int)length] = '\0';

    return fileName;
}

void exploreInode(char *fileToFind, leaf toExploreLeaf, ext4 ext4Info, int *found, int operation) {
    __uint16_t  var16;

    char * fileName;
    unsigned char length;


    fseek(file, (long)toExploreLeaf.actualPosBlock + LENGTH_FILE, SEEK_SET);
    fread(&length, sizeof(length), 1, file);

    fileName = (char *) malloc(sizeof(char) * ((int)length + 1));

    fseek(file, (long)toExploreLeaf.actualPosBlock  + FILE_NAME, SEEK_SET);
    fread(fileName, sizeof(char) * ((int)length + 1), 1, file);

    fileName[(int)length] = '\0';

    printf("Exploro %s i busco file to find %s\n", fileName, fileToFind);

    if (strcmp(fileName, fileToFind) != 0) {

        //mirem següent fulla de mateix ranc
        fseek(file, (long)toExploreLeaf.actualPosBlock  + LENGTH_DIRECTORY, SEEK_SET);
        fread(&var16, sizeof(var16), 1, file);

        toExploreLeaf.actualPosBlock  += var16;

        deepSearch(fileToFind, ext4Info, toExploreLeaf,  found, operation);

    } else {
        __uint32_t var32, lowVar32, upperVar32;

        fseek(file, (long)toExploreLeaf.fatherPosBlock, SEEK_SET);
        fread(&var32, sizeof(var32), 1, file);


        __uint64_t in = ext4Info.posInodeTable * ext4Info.blockSize + ext4Info.inode.inodeSize * (var32);

        *found = 1;
        if (operation == SHOW) {
            printf("File Found! Showing content...\n");

            fseek(file, (long)in + 0x28, SEEK_SET);
            fread(&var32, sizeof(var32), 1, file);




        } else {

            fseek(file, (long)in + 0x90, SEEK_SET);
            fread(&var32, sizeof(var32), 1, file);

            fseek(file, (long)in + 0x4, SEEK_SET);
            fread(&lowVar32, sizeof(lowVar32), 1, file);

            fseek(file,(long) in + 0x6C, SEEK_SET);
            fread(&upperVar32, sizeof(upperVar32), 1, file);

            __uint64_t size =  (__uint64_t) upperVar32 << 32 | lowVar32;

            time_t ts = var32;

            char buff[100];

            strftime(buff, 100, "%d-%m-%Y", localtime(&ts));
            printf("File Found! Size: %lli Created on: %s\n", size, buff);
        }


    }
    free(fileName);
}

void deepSearch(char * fileToFind,  ext4 ext4Info, leaf toExploreLeaf, int *found, int operation) {

    __uint32_t nextInode;
    __uint16_t var16, entries;

    __uint16_t inodeSize = ext4Info.inode.inodeSize;
    uint8_t fileInformation;

    char * fileName;

    //Busco node a explorar
    fseek(file, (long)toExploreLeaf.actualPosBlock, SEEK_SET);
    fread(&nextInode, sizeof(nextInode), 1, file);

    //Tamamy a limitar
    fseek(file, (long)toExploreLeaf.actualPosBlock + 0x4, SEEK_SET);
    fread(&entries, sizeof(entries), 1, file);


    printf("Actual pos block %lli, max len %lli\n", toExploreLeaf.actualPosBlock,  toExploreLeaf.fatherPosBlock + ext4Info.blockSize * toExploreLeaf.maxLen );

    //TODO Arrelgar aquest if per parar quan toca

    if (toExploreLeaf.actualPosBlock > toExploreLeaf.fatherPosBlock + ext4Info.blockSize * toExploreLeaf.maxLen) {
        toExploreLeaf.stop = 1;
    }

    if (nextInode != 0 && !toExploreLeaf.stop) {

        fseek(file, (long)toExploreLeaf.actualPosBlock + 0x7, SEEK_SET);
        fread(&fileInformation, sizeof(fileInformation), 1, file);

        if (fileInformation == 2) {

            __uint64_t in = ext4Info.posInodeTable * ext4Info.blockSize  + inodeSize * (nextInode - 1);

            //Busco posicio del block de l'inode que s'explorara
            toExploreLeaf.fatherPosBlock = toExploreLeaf.actualPosBlock;
            toExploreLeaf.actualPosBlock = findBlock(in + EXTENT_TREE, ext4Info.blockSize, operation, &toExploreLeaf);

            //Nom del pare directori que s'explorara
            fileName = printName(toExploreLeaf.actualPosBlock);

            if (strcmp(fileName, ".") == 0) {
                fseek(file, (long)toExploreLeaf.actualPosBlock + LENGTH_DIRECTORY, SEEK_SET);
                fread(&var16, sizeof(var16), 1, file);

                toExploreLeaf.actualPosBlock += var16;

                fseek(file, (long)toExploreLeaf.actualPosBlock + LENGTH_DIRECTORY, SEEK_SET);
                fread(&var16, sizeof(var16), 1, file);
                toExploreLeaf.actualPosBlock += var16;

            }

            deepSearch(fileToFind, ext4Info, toExploreLeaf, found, operation);

        } else {

            exploreInode(fileToFind, toExploreLeaf, ext4Info, found, operation);
        }

        //Fills ja no tenen més fulles, s'explora germans del pare.
        fseek(file, (long)toExploreLeaf.fatherPosBlock + LENGTH_DIRECTORY, SEEK_SET);
        fread(&var16, sizeof(var16), 1, file);
        toExploreLeaf.fatherPosBlock += var16;

        //Busco tamany a limitar
        fseek(file, (long)toExploreLeaf.fatherPosBlock + 0x4, SEEK_SET);
        fread(&toExploreLeaf.maxLen, sizeof(__uint16_t), 1, file);

        printf("Max len %d\n", toExploreLeaf.maxLen);

        toExploreLeaf.actualPosBlock = toExploreLeaf.fatherPosBlock;

        if (!*found && !toExploreLeaf.stop) {
            deepSearch(fileToFind, ext4Info, toExploreLeaf, found, operation);
        }

    }


}

void explore(char * fileToFind, int operation, ext4 ext4Info) {

    leaf rootLeaf;
    int found = 0;
    __uint16_t inodeSize = ext4Info.inode.inodeSize;

    __uint64_t in = ext4Info.posInodeTable * ext4Info.blockSize + inodeSize;


    rootLeaf.actualPosBlock = findBlock(in + EXTENT_TREE, ext4Info.blockSize, operation, &rootLeaf);
    rootLeaf.fatherPosBlock = rootLeaf.actualPosBlock;

    rootLeaf.stop = 0;

    //Max len
    fseek(file, (long)rootLeaf.actualPosBlock + 0x4, SEEK_SET);
    fread(&rootLeaf.maxLen, sizeof(rootLeaf.maxLen), 1, file);


    if (operation == SEARCH_OPERATION) {
        found = findFileinDirectory(rootLeaf.actualPosBlock, in, fileToFind);
        if (found == 0) {
            printf(FILE_NOT_FOUND);
        }

    } else {
        deepSearch(fileToFind, ext4Info, rootLeaf, &found, operation);
        if (found == 0) {
            printf(FILE_NOT_FOUND);
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
    explore(fileToFind, operation, ext4Info);
}



