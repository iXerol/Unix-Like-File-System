#ifndef structs_h
#define structs_h
#include <time.h>
#include "consts.h"

struct inode_t {
    unsigned int number;		
    unsigned int data_address[NADDR];		
    unsigned short link_count;		
    unsigned short mode;		
    char user[32];		
    char group[32];		
    size_t size;		
    time_t created_time;		
    time_t modified_time;        
    time_t accessed_time;        
};

struct superblock_t {
    unsigned int num_free_inode;

    unsigned int num_free_block;

    unsigned short umask;

    uint64_t free_inodes;

    unsigned int free_block_stack[100];
    size_t stack_size;
};

struct child_file_t {
    char filename[FILE_NAME_LENGTH];
    unsigned int inode_number;
};

#endif
