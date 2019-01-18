#ifndef variables_h
#define variables_h

#include <stdio.h>
#include "superblock.h"

FILE* disk;

struct superblock_t superblock;

struct inode_t inodes[INODE_NUM];

char current_user[32];

char current_group[32];

char current_working_directory[16];

unsigned int current_working_inode;

#endif /* variables_h */
