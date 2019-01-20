#ifndef file_h
#define file_h

#include "functions.h"

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

        strcpy(current_user, "system");
        strcpy(current_group, "system");

        create_root();

        struct inode_t* root = get_inode_by_num(0);

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");

        create_directory(root, "/etc");
        create_directory(root, "/etc/test");
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
            fseek(disk, INODE_BLOCK_START, SEEK_SET);
            fread(inodes, INODE_SIZE, 1, disk);
        }

        current_working_inode = get_inode_by_num(0);
        strcpy(current_working_directory, "/");
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

#endif /* file_h */
