#ifndef Consts_h
#define Consts_h

#include <stdint.h>

//文件系统文件名
const char* FILENAME = "volume.hex";

//单文件最大块数（4 块直接，1 块一级间接，1 块二级间接）
const unsigned short NADDR = 6;

//block 大小
const size_t BLOCK_SIZE = 512;

//文件最大大小
const size_t MAX_FILE_SIZE = (NADDR - 2) * BLOCK_SIZE + BLOCK_SIZE / sizeof(uint32_t) * BLOCK_SIZE + BLOCK_SIZE / sizeof(uint32_t) * BLOCK_SIZE / sizeof(uint32_t) * BLOCK_SIZE;

//block 数量
const unsigned int BLOCK_NUM = 256;

//inode 大小
const size_t INODE_SIZE = 128;

//inode 数量
const unsigned int INODE_NUM = 64;

//inode 起始盤塊
const unsigned int INODE_START_BLOCK = 3;

//数据起始盤塊
const unsigned int DATA_START_BLOCK = INODE_START_BLOCK + INODE_NUM * INODE_SIZE / BLOCK_SIZE;

//文件名最大长度
const size_t FILE_NAME_LENGTH = 32 - sizeof(unsigned int);

//用户名最大长度
const size_t USER_NAME_LENGTH = 32;

//用户密码最大长度
const size_t USER_PASSWORD_LENGTH = 32;

//用户組名最大长度
const size_t GROUP_NAME_LENGTH = 32;

//文件最大权限
const unsigned short MAX_PERMISSION = 0777;

//目录最大权限
const unsigned short MAX_OWNER_PERMISSION = 0666;

//默认umask(0022)
const unsigned short DEFAULT_UMASK = 0022;

//文件类型
const unsigned short ISREG = 00000;        //reguler file
const unsigned short ISDIR = 01000;        //directory
const unsigned short ISCHR = 02000;        //character device
const unsigned short ISBLK = 03000;        //block device
const unsigned short ISFIFO = 04000;       //FIFO
const unsigned short ISISLNK = 05000;      //symbolic link
const unsigned short ISSOCK = 06000;       //socket

//文件权限
const unsigned short IRWXU = 00700;       //owner has read, write, and execute permission
const unsigned short IRUSR = 00400;       //owner has read permission
const unsigned short IWUSR = 00200;       //owner has write permission
const unsigned short IXUSR = 00100;       //owner has execute permission

const unsigned short IRWXG = 00070;       //group has read, write, and execute permission
const unsigned short IRGRP = 00040;       //group has read permission
const unsigned short IWGRP = 00020;       //group has write permission
const unsigned short IXGRP = 00010;       //group has execute permission

const unsigned short IRWXO = 00007;       //others has read, write, and execute permission
const unsigned short IROTH = 00004;       //others has read permission
const unsigned short IWOTH = 00002;       //others has write permission
const unsigned short IXOTH = 00001;       //others has execute permission

#endif /* Consts_h */
