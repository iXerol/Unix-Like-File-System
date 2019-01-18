#ifndef directory_h
#define directory_h

#include <stdint.h>

struct child_file_t {
    char filename[FILE_NAME_LENGTH];
    unsigned int inode_number;
};


#endif /* directory_h */
