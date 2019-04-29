#include "variables.h"

FILE* disk;

struct superblock_t superblock;

struct inode_t inodes[INODE_NUM];

char current_user[32];

char current_group[32];

char current_working_directory[FILE_NAME_LENGTH];

struct inode_t* current_working_inode;
