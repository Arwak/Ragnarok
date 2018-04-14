//
// Structure that we will read for Ext4
//

#ifndef RAGNAROK_EXT4_H
#define RAGNAROK_EXT4_H

#include <stdio.h>

#define EXT4                            4
#define EXT3                            3
#define EXT2                            2

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
#define FRAGS_GROUP                  0x58

#define VOLUME_NAME                  0x78
#define LAST_CHECK                   0x40
#define LAST_MOUNT                   0x2C
#define LAST_WRITTEN                 0x30


typedef struct inodeInfo {
    unsigned int  inodeSize;        //Size of inode structure, in bytes.
    unsigned long numberOfInodes;   //Total inode count.
    unsigned long firstInodeM;      //First non-reserved inode.
    unsigned long inodesGroup;      //Inodes per group.
    unsigned long freeInodes;       //Free inode count.
} inodeInfoStruct;

typedef struct blockInfo {
    unsigned long blockSize;            //Block size is 2 ^ (10 + s_log_block_size).
    unsigned long reservedBlocks;       //This number of blocks can only be allocated by the super-user.
    unsigned long freeBlocks;           //Free block count.
    unsigned long totalBlocks;          //Total block count.
    unsigned long firstBlock;           //First data block. This must be at least 1 for 1k-block filesystems
                                        // and is typically 0 for all other block sizes.

    unsigned long blockGroup;           //Blocks per group.
    unsigned long fragsGroup;           //??? no sabem que és
} blockInfoStruct;

typedef struct volumeInfo {
    char volumeName[16];                //Volume label.
    unsigned long lastCheck;            //Time of last check, in seconds since the epoch.
    unsigned long lastMount;            //Mount time, in seconds since the epoch.
    unsigned long lastWritten;          //Write time, in seconds since the epoch.
} volumeInfoStruct;

typedef struct info {
    inodeInfoStruct inode;
    blockInfoStruct block;
    volumeInfoStruct volume;

} extInformation;


void readExt(FILE* file);

#endif //RAGNAROK_EXT4_H
