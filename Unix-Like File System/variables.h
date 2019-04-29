#ifndef variables_h
#define variables_h

#include <stdio.h>
#include "structs.h"

extern FILE* disk;

extern struct superblock_t superblock;

extern struct inode_t inodes[];

extern char current_user[];

extern char current_group[];

extern char current_working_directory[];

extern struct inode_t* current_working_inode;

#endif /* variables_h */
