//
// Structure that we will read for Ext4
//

#ifndef RAGNAROK_EXT4_H
#define RAGNAROK_EXT4_H

#include <stdio.h>
#include <time.h>

#define DEEP                            2
#define SHOW                            3

#define EXT_PADDING_SUPER_BLOCK        1024


#define INODE_SIZE                   0x58
#define NUMBER_INODES                0x0
#define FIRST_INODE                  0x54
#define INODE_GROUP                  0x28
#define FREE_INODES                  0x10

#define BLOCK_SIZE                   0x18
#define RESERVED_BLOCKS              0x8
#define FREE_BLOCKS                  0xC
#define TOTAL_BLOCKS                 0x4
#define FIRST_BLOCK                  0x14
#define BLOCK_GROUP                  0x20
#define FRAGS_GROUP                  0x24

#define VOLUME_NAME                  0x78
#define LAST_CHECK                   0x40
#define LAST_MOUNT                   0x2C
#define LAST_WRITTEN                 0x30

#define READ_CODE 4
#define WRITE_CODE 5
#define HIDE_CODE 6
#define SHIDE_CODE 7
#define DATE_CODE 8


typedef struct inodeInfo {
    __uint16_t  inodeSize;        //Size of inode structure, in bytes.
    __uint32_t numberOfInodes;   //Total inode count.
    __uint32_t firstInodeM;      //First non-reserved inode.
    __uint32_t inodesGroup;      //Inodes per group.
    __uint32_t freeInodes;       //Free inode count.
} inodeInfoStruct;

typedef struct blockInfo {
    __uint32_t blockSize;            //Block size is 2 ^ (10 + s_log_block_size).
    __uint32_t reservedBlocks;       //This number of blocks can only be allocated by the super-user.
    __uint32_t freeBlocks;           //Free block count.
    __uint32_t totalBlocks;          //Total block count.
    __uint32_t firstBlock;           //First data block. This must be at least 1 for 1k-block filesystems
                                        // and is typically 0 for all other block sizes.

    __uint32_t blockGroup;           //Blocks per group.
    __uint32_t fragsGroup;           //??? no sabem que és
} blockInfoStruct;

typedef struct volumeInfo {
    char volumeName[16];                //Volume label.
    __uint32_t lastCheck;                   //Time of last check, in seconds since the epoch.
    __uint32_t lastMount;                   //Mount time, in seconds since the epoch.
    __uint32_t lastWritten;                 //Write time, in seconds since the epoch.
} volumeInfoStruct;

typedef struct info {
    inodeInfoStruct inode;
    blockInfoStruct block;
    volumeInfoStruct volume;
    int blockSize;
    __uint64_t posInodeTable;

} ext4;

ext4 readExt4(FILE *file);
void showExt(ext4 ext);
void searchExt4(FILE * file, char * filetofind, int operation);


#endif //RAGNAROK_EXT4_H
