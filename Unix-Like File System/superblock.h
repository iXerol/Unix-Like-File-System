#ifndef superblock_h
#define superblock_h

struct superblock_t {
    unsigned int num_free_inode;

    unsigned int num_free_block;

    unsigned short umask;

    uint64_t free_inodes;

    unsigned int free_block_stack[100];
    size_t stack_size;
};

#endif /* superblock_h */
