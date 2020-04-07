#ifndef structs_h
#define structs_h

#include <time.h>
#include "consts.h"

struct inode_t {
    unsigned int number;        //inode 編號
    unsigned int data_address[NADDR];        //數據索引地址（直接、一級、二級）
    unsigned short link_count;        //連接數
    unsigned short mode;        //文件類型及權限
    char user[32];        //文件所屬用戶
    char group[32];        //文件所屬組
    size_t size;        //文件大小
    time_t created_time;        //文件創建時間
    time_t modified_time;        //文件最後編輯時間
    time_t accessed_time;        //文件最後訪問時間
};

struct superblock_t {
    unsigned int num_free_inode;    //空閒 inode 數量
    unsigned int num_free_block;    //空閒盤塊數

    unsigned short umask;       //當前 umask

    uint64_t free_inodes;       //inode 使用情況位示圖

    unsigned int free_block_stack[DATA_BLOCK_STACK_SIZE];       //空閒盤塊棧
    size_t stack_size;      //空閒盤塊棧容量
};

struct child_file_t {
    char filename[FILE_NAME_LENGTH];        //文件名
    unsigned int inode_number;          //inode 編號
};

#endif /* structs_h */
