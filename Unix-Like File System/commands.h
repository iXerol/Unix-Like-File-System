#ifndef commands_h
#define commands_h

#include "functions.h"

//😎void ls();
//
//void chmod(char* filename, char* username, int privilege);
//
//void chown(char* filename, char* username);
//
//void chgrp(char* filename, char* groupname);
//
//😎void pwd();
//
//void cd(char* path);
//
//😎void mkdir(char* directoryName);
//
//void rmdir(char* directoryName);
//
//void mv(char* pathBefore, char* pathAfter);
//
//void cp(char* pathOriginal, char* pathDuplicate);
//
//😎void rm(char* filename);
//
//😎void ln();
//
//😎void cat(char* filename);
//
//void passwd();
//
//😎void umask();


void show(void);

void present_working_directory(void);

void list(char* path);

void status(struct inode_t* directory, char* path);

void link_file(struct inode_t* working_directory, char* target_file_path, char* source_file_path);

void create_directory(struct inode_t* working_directory, char* directory_path);

void touch_file(char* path);

void resize_text_file(struct inode_t* inode, size_t new_size);

void remove_regular_file(char* path);

void resize_text_file(struct inode_t* inode, size_t new_size);

void cat(char* path);

void show_umask(void);

void change_umask(unsigned short new_umask);


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

void create_user(char* username, char* password, char* group) {
    if (username == NULL || password == NULL || group == NULL) {
        return;
    }
    if (strlen(username) <= USER_NAME_LENGTH && strlen(password) <= USER_PASSWORD_LENGTH && strlen(group) <= GROUP_NAME_LENGTH) {
        struct inode_t* passwd = find_file_by_path(current_working_inode, "/etc/passwd");
        if (passwd == NULL) {
            passwd = find_file_by_path(current_working_inode, "/etc");
            if (passwd == NULL) {
                create_directory(current_working_inode, "/etc");
            }
            touch_file("/etc/passwd");
            passwd = find_file_by_path(current_working_inode, "/etc/passwd");
        }
        size_t user_size = passwd->size;
        char* user_data = malloc(user_size + USER_DATA_LENGTH);
        read_data(passwd, user_data);
        char tmp_username[USER_NAME_LENGTH];

        for (size_t i = 0; i < user_size / USER_DATA_LENGTH; ++i) {
            memset(tmp_username, 0, USER_NAME_LENGTH);
            strncpy(tmp_username, user_data + i * USER_DATA_LENGTH, USER_NAME_LENGTH);
            if (strcmp(username, tmp_username) == 0) {
                printf("User already exists.\n");
                return;
            }
        }

        strcpy(user_data + user_size, username);
        strcpy(user_data + user_size + USER_NAME_LENGTH, password);
        strcpy(user_data + user_size + USER_NAME_LENGTH + USER_PASSWORD_LENGTH, group);
        erase_data(passwd);
        write_data(passwd, user_data, user_size + USER_DATA_LENGTH);
    }
}

void present_working_directory() {
    if (current_working_inode->number == 0) {
        printf("/\n");
        return;
    }

    struct inode_t* working_directory = current_working_inode;
    struct inode_t* worked_directory = current_working_inode;
    char working_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM];
    memset(working_directory_string, '\0', (FILE_NAME_LENGTH + 1) * INODE_NUM);
    char worked_directory_string[(FILE_NAME_LENGTH + 1) * INODE_NUM];
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

void list(char* path) {
    struct inode_t* directory = current_working_inode;
    if (path != NULL && strcmp(path, "") != 0) {
        directory = find_file_by_path(current_working_inode, path);
    }
    if (directory == NULL) {
        printf("ls: %s: No such file or directory\n", path);
    } else if ((directory->mode & 07000) == ISDIR) {
        char *data = (char *)malloc(directory->size);
        read_data(directory, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;

        //        printf("Mode    Link count   User   Group    Size    Last modified    Filename\n");

        for (int i = 0; i < directory->size / sizeof(struct child_file_t); ++i) {
            struct inode_t* inode = get_inode_by_num(directory_content[i].inode_number);
            switch (inode->mode & 07000) {
                case ISREG:
                    putchar('-');
                    break;
                case ISDIR:
                    putchar('d');
                    break;
                case ISCHR:
                    putchar('c');
                    break;
                case ISBLK:
                    putchar('b');
                    break;
                case ISLNK:
                    putchar('l');
                    break;
                case ISFIFO:
                    putchar('p');
                    break;
                case ISSOCK:
                    putchar('s');
                    break;
                default:
                    break;
            }
            printf("%c%c%c%c%c%c%c%c%c ", (inode->mode & IRUSR) != 0 ? 'r' : '-',
                   (inode->mode & IWUSR) != 0 ? 'w' : '-',
                   (inode->mode & IXUSR) != 0 ? 'x' : '-',
                   (inode->mode & IRGRP) != 0 ? 'r' : '-',
                   (inode->mode & IWGRP) != 0 ? 'w' : '-',
                   (inode->mode & IXGRP) != 0 ? 'x' : '-',
                   (inode->mode & IROTH) != 0 ? 'r' : '-',
                   (inode->mode & IWOTH) != 0 ? 'w' : '-',
                   (inode->mode & IXOTH) != 0 ? 'x' : '-');

            printf("%3d ", inode->link_count);
            printf("%s  %s ", inode->user, inode->group);
            printf("%8zu", inode->size);

            char time[26];
            strcpy(time,  asctime(localtime(&inode->modified_time)));
            time[24] = ' ';
            printf(" %s",time);
            printf("%s\n", directory_content[i].filename);
        }
    } else {
        struct inode_t* inode = directory;
        switch (inode->mode & 07000) {
            case ISREG:
                putchar('-');
                break;
            case ISDIR:
                putchar('d');
                break;
            case ISCHR:
                putchar('c');
                break;
            case ISBLK:
                putchar('b');
                break;
            case ISLNK:
                putchar('l');
                break;
            case ISFIFO:
                putchar('p');
                break;
            case ISSOCK:
                putchar('s');
                break;
            default:
                break;
        }
        printf("%c%c%c%c%c%c%c%c%c ", (inode->mode & IRUSR) != 0 ? 'r' : '-',
               (inode->mode & IWUSR) != 0 ? 'w' : '-',
               (inode->mode & IXUSR) != 0 ? 'x' : '-',
               (inode->mode & IRGRP) != 0 ? 'r' : '-',
               (inode->mode & IWGRP) != 0 ? 'w' : '-',
               (inode->mode & IXGRP) != 0 ? 'x' : '-',
               (inode->mode & IROTH) != 0 ? 'r' : '-',
               (inode->mode & IWOTH) != 0 ? 'w' : '-',
               (inode->mode & IXOTH) != 0 ? 'x' : '-');

        printf("%3d ", inode->link_count);
        printf("%s  %s ", inode->user, inode->group);
        printf("%8zu", inode->size);

        char time[26];
        strcpy(time,  asctime(localtime(&inode->modified_time)));
        time[24] = ' ';
        printf(" %s",time);
        printf("%s\n", path);
    }
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
        switch (inode->mode & 07000) {
            case ISREG:
                putchar('-');
                break;
            case ISDIR:
                putchar('d');
                break;
            case ISCHR:
                putchar('c');
                break;
            case ISBLK:
                putchar('b');
                break;
            case ISLNK:
                putchar('l');
                break;
            case ISFIFO:
                putchar('p');
                break;
            case ISSOCK:
                putchar('s');
                break;
            default:
                break;
        }
        printf("%c%c%c%c%c%c%c%c%c\n", (inode->mode & IRUSR) != 0 ? 'r' : '-',
               (inode->mode & IWUSR) != 0 ? 'w' : '-',
               (inode->mode & IXUSR) != 0 ? 'x' : '-',
               (inode->mode & IRGRP) != 0 ? 'r' : '-',
               (inode->mode & IWGRP) != 0 ? 'w' : '-',
               (inode->mode & IXGRP) != 0 ? 'x' : '-',
               (inode->mode & IROTH) != 0 ? 'r' : '-',
               (inode->mode & IWOTH) != 0 ? 'w' : '-',
               (inode->mode & IXOTH) != 0 ? 'x' : '-');
        printf("User: %s\n", inode->user);
        printf("Group: %s\n", inode->group);
        printf("Created Time: %s", asctime(localtime(&inode->created_time)));
        printf("Last Modified Time: %s", asctime(localtime(&inode->modified_time)));
        printf("Last Accessed Time: %s", asctime(localtime(&inode->accessed_time)));
        printf("inode number: %u\n", inode->number);
        printf("Link Count: %u\n", inode->link_count);
    }
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
    struct inode_t* parent_directory;
    char* directory_name = (char*)malloc(strlen(directory_path));
    if (strchr(directory_path, '/') != NULL) {
        parent_directory = find_parent(current_working_inode, directory_path);
        get_file_name(directory_path, directory_name);
    } else {
        parent_directory = current_working_inode;
        strcpy(directory_name, directory_path);
    }
    if (parent_directory == NULL) {
        printf("mkdir: %s: No such file or directory\n", directory_path);
    } else if (find_file_from_parent(parent_directory, directory_name) != NULL) {
        printf("mkdir: %s: File exists\n", directory_path);
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
        unsigned short writing_block = (unsigned short)(parent_directory->size / BLOCK_SIZE);
        size_t writing_position =parent_directory->size % BLOCK_SIZE;
        //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                parent_directory->data_address[writing_block] = get_free_data_block();
                if (parent_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (parent_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&directory, sizeof(struct child_file_t), 1, disk);
            parent_directory->size += sizeof(struct child_file_t);
            //創建 . .. 目錄
            char* dot_path = (char*)malloc(strlen(directory_name) + 4);
            strcpy(dot_path, directory_name);
            strcat(dot_path, "/.");
            link_file(parent_directory, dot_path, directory_name);
            strcat(dot_path, ".");
            link_file(parent_directory, dot_path, ".");
        }
    }
}

void touch_file(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* parent_directory;
    char* filename = (char*)malloc(strlen(path));
    if (strchr(path, '/') != NULL) {
        parent_directory = find_parent(current_working_inode, path);
        get_file_name(path, filename);
    } else {
        parent_directory = current_working_inode;
        strcpy(filename, path);
    }
    if (parent_directory == NULL) {
        printf("touch: %s: No such file or directory\n", path);
    } else if (find_file_from_parent(parent_directory, filename) != NULL) {
        struct inode_t* inode = find_file_from_parent(parent_directory, filename);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
    } else if (strlen(filename) > FILE_NAME_LENGTH) {
        printf("%s: Too long file name\n", path);
    } else {
        //創建目錄子項
        struct child_file_t file;
        memset(&file.filename, '\0', FILE_NAME_LENGTH);
        file.inode_number = get_free_inode();
        strcpy(file.filename, filename);
        //初始化 inode
        struct inode_t* inode = get_inode_by_num(file.inode_number);
        inode->link_count = 1;
        inode->mode = ISREG + MAX_FILE_PERMISSION - superblock.umask;
        strcpy(inode->user, current_user);
        strcpy(inode->group, current_group);
        inode->size = 0;
        inode->created_time = time(NULL);
        inode->modified_time = time(NULL);
        inode->accessed_time = time(NULL);
        //寫目錄文件
        unsigned short writing_block = (unsigned short)(parent_directory->size / BLOCK_SIZE);
        size_t writing_position =parent_directory->size % BLOCK_SIZE;
        //因為 inode 總數僅 64 個，填滿目錄前四塊直接索引塊需要 64 個子項，因此不可能填滿。
        if (writing_block < NADDR - 2) {
            if (writing_position == 0) {
                parent_directory->data_address[writing_block] = get_free_data_block();
                if (parent_directory->data_address[writing_block] == BLOCK_NUM) {
                    return_inode(inode->number);
                    return;
                }
            }
            fseek(disk, (parent_directory->data_address[writing_block] + DATA_BLOCK_START) * BLOCK_SIZE + writing_position, SEEK_SET);
            fwrite(&file, sizeof(struct child_file_t), 1, disk);
            parent_directory->size += sizeof(struct child_file_t);
        }
    }
}

void resize_text_file(struct inode_t* inode, size_t new_size) {
    erase_data(inode);
    char* data = (char*)malloc(new_size + 1);
    memset(data, 'a', new_size);
    write_data(inode, data, new_size);
}

void remove_regular_file(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* parent_directory = find_parent(current_working_inode, path);
    if (parent_directory == NULL) {
        printf("rm: %s: No such file or directory\n", path);
    }
    char* filename = (char*)malloc(strlen(path));
    get_file_name(path, filename);
    struct inode_t* file = find_file_from_parent(parent_directory, filename);
    if (file == NULL) {
        printf("rm: %s: No such file or directory\n", path);
    }
    if ((file->mode & 07000) == ISDIR) {
        printf("rm: %s: is a directory\n", path);
    } else {
        size_t parent_size = parent_directory->size;
        char *data = (char *)malloc(parent_size);
        read_data(parent_directory, data);
        struct child_file_t* directory_content = (struct child_file_t*)data;
        for (unsigned i = 0; i < parent_size / sizeof(struct child_file_t); ++i) {
            if (strcmp(directory_content[i].filename, filename) == 0) {
                directory_content[i] = directory_content[parent_size / sizeof(struct child_file_t)];
                erase_data(parent_directory);
                write_data(parent_directory, data, parent_size - sizeof(struct child_file_t));
            }
        }

        --file->link_count;
        if (file->link_count == 0) {
            erase_data(file);
            return_inode(file->number);
        }
    }
}

void cat(char* path) {
    if (path == NULL) {
        return;
    }
    struct inode_t* inode = find_file_by_path(current_working_inode, path);
    if (inode == NULL) {
        printf("cat: %s: No such file or directory\n", path);
    } else if ((inode->mode & 07000) == ISDIR) {
        printf("cat: %s: Is a directory\n", path);
    } else {
        char* data = (char*)malloc(inode->size);
        read_data(inode, data);
        for (size_t i = 0; i < inode->size; ++i) {
            putchar(data[i]);
        }
        putchar('\n');
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
