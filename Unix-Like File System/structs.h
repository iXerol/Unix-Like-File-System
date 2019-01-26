#ifndef structs_h
#define structs_h
#include <time.h>
#include "consts.h"

struct inode_t {
    unsigned int number;        //inode 编号
    unsigned int data_address[NADDR];        //数据索引地址（直接、一级、二级）
    unsigned short link_count;        //连接数
    unsigned short mode;        //文件类型及权限
    char user[32];        //文件所属用户
    char group[32];        //文件所属组
    size_t size;        //文件大小
    time_t created_time;        //文件创建时间
    time_t modified_time;        //文件最后编辑时间
    time_t accessed_time;        //文件最后访问时间
};

struct superblock_t {
    unsigned int num_free_inode;    //空闲 inode 数量
    unsigned int num_free_block;    //空闲盘块数

    unsigned short umask;       //当前 umask

    uint64_t free_inodes;       //inode 使用情况位示图

    unsigned int free_block_stack[DATA_BLOCK_STACK_SIZE];       //空闲盘块栈
    size_t stack_size;      //空闲盘块栈容量
};

struct child_file_t {
    char filename[FILE_NAME_LENGTH];        //文件名
    unsigned int inode_number;          //inode 编号
};

#endif /* structs_h */
