#ifndef commands_h
#define commands_h

#include "functions.h"

//void ls();
//
//void chmod(char* filename, char* username, int privilege);
//
//void chown(char* filename, char* username);
//
//void chgrp(char* filename, char* groupname);
//
//void pwd();
//
//void cd(char* path);
////a: No such file or directory
////a: Not a directory
//
//void mkdir(char* directoryName);
//
//void rmdir(char* directoryName);
//
//void mv(char* pathBefore, char* pathAfter);
//
//void cp(char* pathOriginal, char* pathDuplicate);
//
//void rm(char* filename);
//
//void ln();
//
//void cat(char* filename);
//
//void passwd();
//
//void umask();



void create_root(void);

void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path);

void create_directory(struct inode_t* working_directory, char* directory_path);

void present_working_directory(void);

void status(struct inode_t* directory, char* path);




void create_root() {
    //inodes[0] 對應根目錄
    inodes[0].link_count = 1;
    strcpy(inodes[0].user, current_user);
    strcpy(inodes[0].group, current_group);
    inodes[0].mode = ISDIR + MAX_DIRECTORY_PERMISSION - superblock.umask;
    superblock.free_inodes = superblock.free_inodes ^ (1 << get_free_inode());

    link_file(get_inode_by_num(0), ".", "/");
    link_file(get_inode_by_num(0), "..", "/");
}

void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path) {
    if (target_file_path == NULL || source_file_path == NULL) {
        return;
    }
    struct inode_t* source_file = find_file_by_path(working_directory, source_file_path);
    struct inode_t* target_parent = find_parent(working_directory, target_file_path);
    char* target_file_name = (char*)malloc(strlen(target_file_path));
    get_file_name(target_file_path, target_file_name);
    if (source_file == NULL) {
        printf("ln: %s: No such file or directory\n", source_file_path);
        return;
    } else if (target_parent == NULL) {
        printf("ln: %s: No such file or directory\n", target_file_path);
        return;
    } else if (find_file_from_parent(target_parent, target_file_name) != NULL) {
        printf("ln: %s: File exists\n", target_file_path);
        return;
    }

    //創建目錄子項
    struct child_file_t new_link_file;
    memset(&new_link_file, '\0', sizeof(struct child_file_t));
    strcpy(new_link_file.filename, target_file_name);
    new_link_file.inode_number = source_file->number;
    //寫目錄文件
    unsigned short writing_block = (unsigned short)(target_parent->size / BLOCK_SIZE);
    size_t writing_position =target_parent->size % BLOCK_SIZE;
    //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
    if (writing_block < NADDR - 2) {
        if (writing_position == 0) {
            target_parent->data_address[writing_block] = get_free_data_block();
            if (target_parent->data_address[writing_block] == BLOCK_NUM) {
                return;
            }
        }
        fseek(disk, (target_parent->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
        fwrite(&new_link_file, sizeof(struct child_file_t), 1, disk);
        target_parent->size += sizeof(struct child_file_t);
        ++source_file->link_count;
        source_file->accessed_time = time(NULL);
    }
}

void create_directory(struct inode_t* working_directory, char* directory_path) {
    if (directory_path == NULL) {
        return;
    }
    char* directory_name = (char*)malloc(strlen(directory_path));
    if (strchr(directory_path, '/') != NULL) {
        working_directory = find_parent(working_directory, directory_path);
        get_file_name(directory_path, directory_name);
    } else {
        strcpy(directory_name, directory_path);
    }
    if (working_directory == NULL) {
        printf("mkdir: %s: No such file or directory\n", directory_path);

    } else if (strlen(directory_name) > FILE_NAME_LENGTH) {
        printf("%s: Too long directory name\n", directory_path);
    } else {
        //創建目錄子項
        struct child_file_t directory;
        memset(&directory.filename, '\0', FILE_NAME_LENGTH);
        directory.inode_number = get_free_inode();
        strcpy(directory.filename, directory_name);
        //初始化 inode
        struct inode_t* inode = get_inode_by_num(directory.inode_number);
        inode->link_count = 1;
        inode->mode = ISDIR + MAX_DIRECTORY_PERMISSION - superblock.umask;
        strcpy(inode->user, current_user);
        strcpy(inode->group, current_group);
        inode->size = 0;
        inode->created_time = time(NULL);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        //寫目錄文件
        unsigned short writing_block = (unsigned short)(working_directory->size / BLOCK_SIZE);
        size_t writing_position =working_directory->size % BLOCK_SIZE;
        //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                working_directory->data_address[writing_block] = get_free_data_block();
                if (working_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (working_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&directory, sizeof(struct child_file_t), 1, disk);
            working_directory->size += sizeof(struct child_file_t);
            //創建 . .. 目錄
            char* dot_path = (char*)malloc(strlen(directory_name) + 4);
            strcpy(dot_path, directory_name);
            strcat(dot_path, "/.");
            link_file(working_directory, dot_path, directory_name);
            strcat(dot_path, ".");
            link_file(working_directory, dot_path, ".");
        }
    }
}

void present_working_directory() {
    if (current_working_inode->number == 0) {
        printf("/\n");
        return;
    }

    struct inode_t* working_directory = current_working_inode;
    struct inode_t* worked_directory = current_working_inode;
    char working_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM] = "";
    memset(working_directory_string, '\0', (FILE_NAME_LENGTH + 1) * INODE_NUM);
    char worked_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM] ="";
    memset(worked_directory_string, '\0', (FILE_NAME_LENGTH + 1) * INODE_NUM);
    char* data = (char*)malloc(sizeof(struct child_file_t) * INODE_NUM);

    read_data(working_directory, data);
    struct child_file_t* directory_content = (struct child_file_t*)data;
    for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
        if (strcmp(directory_content[i].filename, "..") == 0) {
            worked_directory = working_directory;
            working_directory = get_inode_by_num(directory_content[i].inode_number);
            break;
        }
    }
    while (working_directory->number != 0) {
        read_data(working_directory, data);
        for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
            if (directory_content[i].inode_number == worked_directory->number) {
                strcpy(working_directory_string, "/");
                strcat(working_directory_string, directory_content[i].filename);
                strcat(working_directory_string, worked_directory_string);
                strcpy(worked_directory_string, working_directory_string);
                break;
            }
        }
        directory_content = (struct child_file_t*)data;
        for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, "..") == 0) {
                worked_directory = working_directory;
                working_directory = get_inode_by_num(directory_content[i].inode_number);
                break;
            }
        }
    }
    read_data(working_directory, data);
    for (int i = 0; i < working_directory->size / sizeof(struct child_file_t); ++i) {
        if (directory_content[i].inode_number == worked_directory->number) {
            strcpy(working_directory_string, "/");
            strcat(working_directory_string, directory_content[i].filename);
            strcat(working_directory_string, worked_directory_string);
            strcpy(worked_directory_string, working_directory_string);
            //            break;
        }
    }

    printf("%s\n", working_directory_string);
}

void status(struct inode_t* directory, char* path) {
    if (path == NULL) {
        path = (char*)malloc(10);
        strcpy(path, "/");
    }
    struct inode_t* inode = find_file_by_path(directory, path);
    if (inode == NULL) {
        printf("stat: %s: stat: No such file or directory\n", path);
    } else {
        printf("File: %s\n", path);
        printf("Size: %zu\n", inode->size);
        printf("Permission: ");
        if (inode->mode / 01000 == ISREG / 01000) {
            printf("-");
        } else if (inode->mode / 01000 == ISDIR / 01000) {
            printf("d");
        } else if (inode->mode / 01000 == ISISLNK / 01000) {
            printf("l");
        }
        printf("%c%c%c%c%c%c%c%c%c\n", (inode->mode & IRUSR) != 0 ? 'r' : '-', (inode->mode & IWUSR) != 0 ? 'w' : '-', (inode->mode & IXUSR) != 0 ? 'x' : '-',(inode->mode & IRGRP) != 0 ? 'r' : '-', (inode->mode & IWGRP) != 0 ? 'w' : '-', (inode->mode & IXGRP) != 0 ? 'x' : '-', (inode->mode & IROTH) != 0 ? 'r' : '-', (inode->mode & IWOTH) != 0 ? 'w' : '-', (inode->mode & IXOTH) != 0 ? 'x' : '-');
        printf("User: %s\n", inode->user);
        printf("Group: %s\n", inode->group);
        printf("Created Time: %s", asctime(localtime(&inode->created_time)));
        printf("Last Modified Time: %s", asctime(localtime(&inode->modified_time)));
        printf("Last Accessed Time: %s", asctime(localtime(&inode->accessed_time)));
        printf("inode number: %u\n", inode->number);
        printf("Link Count: %u\n", inode->link_count);
    }
}

void show_umask() {
    printf("%04o\n", superblock.umask);
}

void change_umask(unsigned short new_umask) {
    if (new_umask < 01000) {
        superblock.umask = new_umask;
    } else {
        printf("umask: %o: octal number out of range\n", new_umask);
    }
}
#endif /* commands_h */
