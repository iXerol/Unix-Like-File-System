#ifndef file_h
#define file_h

#include "commands.h"

void new_volume(void);

void mount_volume(void);

void save(void);

void create_root(void);

void initialize_superblock(void);

void initialize_inodes(void) ;

void initialize_data_block(void);

void write_inode(int n);

#endif /* file_h */
