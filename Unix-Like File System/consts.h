#ifndef consts_h
#define consts_h

#include <stdlib.h>

//單文件最大塊數（4 塊直接，1 塊一級間接，1 塊二級間接）
#define NADDR 6

//數據塊大小
#define DATA_BLOCK_STACK_SIZE 200

//文件名最大長度
#define FILE_NAME_LENGTH 28

//inode 大小
#define INODE_NUM 64

//用戶名最大長度
#define USER_NAME_LENGTH 32

//文件类型
#define ISREG 00000        //reguler file
#define ISDIR 01000        //directory
#define ISCHR 02000        //character device
#define ISBLK 03000        //block device
#define ISFIFO 04000       //FIFO
#define ISLNK 05000        //symbolic link
#define ISSOCK 06000       //socket

//文件系統文件名
extern const char* FILENAME;

//block 大小
extern const size_t BLOCK_SIZE;

//索引數據塊中存儲最大塊數
extern const unsigned int NADDR_BLOCK;

//直接索引最大文件大小
extern const size_t MAX_DIRECT_FILE_SIZE;

//一級間接索引最大文件大小
extern const size_t MAX_LEVEL_1_FILE_SIZE;

//（二級間接索引）最大文件大小
extern const size_t MAX_FILE_SIZE;

//block 數量
extern const unsigned int BLOCK_NUM;

//inode 大小
extern const size_t INODE_SIZE;

//inode 起始盤塊
extern const unsigned int INODE_BLOCK_START;

//數據起始盤塊
extern const unsigned int DATA_BLOCK_START;

//用戶密碼最大長度
extern const size_t USER_PASSWORD_LENGTH;

//用戶組名最大長度
extern const size_t GROUP_NAME_LENGTH;

//用戶信息長度
extern const size_t USER_DATA_LENGTH;

//文件最大權限
extern const unsigned short MAX_FILE_PERMISSION;

//目錄最大權限
extern const unsigned short MAX_DIRECTORY_PERMISSION;

//默认umask(0022)
extern const unsigned short DEFAULT_UMASK;

//文件权限
extern const unsigned short IRWXU;       //owner has read, write, and execute permission
extern const unsigned short IRUSR;       //owner has read permission
extern const unsigned short IWUSR;       //owner has write permission
extern const unsigned short IXUSR;       //owner has execute permission

extern const unsigned short IRWXG;       //group has read, write, and execute permission
extern const unsigned short IRGRP;       //group has read permission
extern const unsigned short IWGRP;       //group has write permission
extern const unsigned short IXGRP;       //group has execute permission

extern const unsigned short IRWXO;       //others has read, write, and execute permission
extern const unsigned short IROTH;       //others has read permission
extern const unsigned short IWOTH;       //others has write permission
extern const unsigned short IXOTH;       //others has execute permission

// 最大命令長度
extern const unsigned int MAX_COMMAND_LENGTH;

#endif /* extern consts_h */
