#ifndef file_h
#define file_h

#include "commands.h"

void new_volume(void);
void mount_volume(void);
void save(void);
void show(void);
void initialize_superblock(void);
void initialize_inodes(void) ;
void initialize_data_block(void);
void write_inode(int n);

void new_volume() {
    disk = fopen(FILENAME, "wb+");
    if (disk == NULL) {
        printf("failed to initialize file system.\n");
    } else {
        initialize_superblock();
        initialize_inodes();
        initialize_data_block();

        fseek(disk, BLOCK_SIZE, SEEK_SET);
        fwrite(&superblock, sizeof(struct superblock_t), 1, disk);

        for (int i = 0; i < INODE_NUM; ++i) {
            write_inode(i);
        }

        strcpy(current_user, "root");
        strcpy(current_group, "wheel");

        create_root();

        struct inode_t* root = get_inode_by_num(0);

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");

        create_directory(root, "/etc");
        touch_file(root, "etc/passwd");
        status(root, "/etc/passwd");
        show();
        resize_text_file(find_file_by_path(root, "etc/passwd"), 67584);
        show();
        status(root, "/etc/passwd");
    }
}

void mount_volume() {
    disk = fopen(FILENAME, "rb+");
    if (disk == NULL) {
        printf("failed to mount file system.\n");
    } else {
        fseek(disk, BLOCK_SIZE, SEEK_SET);
        fread(&superblock, sizeof(struct superblock_t), 1, disk);

        for (unsigned int i = 0; i < INODE_NUM; ++i) {
            fseek(disk, INODE_BLOCK_START * BLOCK_SIZE + INODE_SIZE * i, SEEK_SET);
            fread(inodes + i, sizeof(struct inode_t), 1, disk);
        }

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");
        show();
        resize_text_file(find_file_by_path(current_working_inode, "etc/passwd"), 67585);
//        remove_regular_file("/etc/passwd");
        status(current_working_inode, "/etc/passwd");
    }
}

void save() {
    fseek(disk, BLOCK_SIZE, SEEK_SET);
    fwrite(&superblock, sizeof(superblock), 1, disk);

    for (int i = 0; i < INODE_NUM; ++i) {
        write_inode(i);
    }

    fclose(disk);
}


void show() {
    printf("free data block: %u\n", superblock.num_free_block);
    printf("\nfree data block stack size: %zu\n", superblock.stack_size);
    printf("free data block stack:");
    for (int i = 0; i < superblock.stack_size; ++i) {
        printf(" %d", superblock.free_block_stack[i]);
    }
    printf("\n\nfree inode number: %u\ninodes:", superblock.num_free_inode);
    for (int i = 0; i < INODE_NUM; ++i) {
        printf(" %d", (superblock.free_inodes & ((uint64_t)(1) << i)) != 0 ? i : -1);
    }
    printf("\n");
}

void initialize_superblock() {
    superblock.num_free_inode = INODE_NUM;

    superblock.num_free_block = BLOCK_NUM - DATA_BLOCK_START;

    superblock.umask = DEFAULT_UMASK;

    superblock.free_inodes = UINT64_MAX;
}

void initialize_inodes() {
    memset(inodes, 0, sizeof(struct inode_t) * INODE_NUM);
    for (int i = 0; i < INODE_NUM; ++i) {
        inodes[i].number = i;
    }
}

void initialize_data_block() {
    size_t stack_size;
    unsigned int free_block_stack[100];
    unsigned int num_data_block = BLOCK_NUM - DATA_BLOCK_START;
    unsigned int i;
    superblock.stack_size = 100;
    for (i = 1; i <= 100; ++i) {
        superblock.free_block_stack[i - 1] = i;
    }
    fseek(disk, (superblock.free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
    while (i < num_data_block) {
        stack_size = (num_data_block - i < 100) ? num_data_block - i : 100;
        for (int j = 0; j < stack_size; ++i, ++j) {
            free_block_stack[j] = i;
        }
        if (stack_size < 100) {
            free_block_stack[stack_size] = free_block_stack[0];
            free_block_stack[0] = BLOCK_NUM;
            ++stack_size;
        }
        fwrite(&stack_size, sizeof(size_t), 1, disk);
        fwrite(free_block_stack, sizeof(unsigned int), stack_size, disk);
        if (free_block_stack[0] != BLOCK_NUM) {
            fseek(disk, (free_block_stack[0] + DATA_BLOCK_START) * BLOCK_SIZE, SEEK_SET);
        }
    }
}

void write_inode(int n) {
    fseek(disk, INODE_BLOCK_START * BLOCK_SIZE + n * INODE_SIZE, SEEK_SET);
    fwrite(inodes + n, sizeof(struct inode_t), 1, disk);
}

#endif /* file_h */
