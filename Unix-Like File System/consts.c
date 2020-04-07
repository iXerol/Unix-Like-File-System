#include "consts.h"

//文件系統文件名
const char* FILENAME = "volume.hex";

//block 大小
const size_t BLOCK_SIZE = 512;

//索引數據塊中存儲最大塊數
const unsigned int NADDR_BLOCK = BLOCK_SIZE / sizeof(unsigned int);

//直接索引最大文件大小
const size_t MAX_DIRECT_FILE_SIZE = (NADDR - 2) * BLOCK_SIZE;

//一級間接索引最大文件大小
const size_t MAX_LEVEL_1_FILE_SIZE = MAX_DIRECT_FILE_SIZE + NADDR_BLOCK * BLOCK_SIZE;

//（二級間接索引）最大文件大小
const size_t MAX_FILE_SIZE = MAX_LEVEL_1_FILE_SIZE + NADDR_BLOCK * NADDR_BLOCK * BLOCK_SIZE;

//block 數量
const unsigned int BLOCK_NUM = 400;

//inode 大小
const size_t INODE_SIZE = 128;

//inode 起始盤塊
const unsigned int INODE_BLOCK_START = 2;

//數據起始盤塊
const unsigned int DATA_BLOCK_START = INODE_BLOCK_START + INODE_NUM * INODE_SIZE / BLOCK_SIZE;

//用戶密碼最大長度
const size_t USER_PASSWORD_LENGTH = 32;

//用戶組名最大長度
const size_t GROUP_NAME_LENGTH = 32;

//用戶信息長度
const size_t USER_DATA_LENGTH = USER_NAME_LENGTH + USER_PASSWORD_LENGTH + GROUP_NAME_LENGTH;

//文件最大權限
const unsigned short MAX_FILE_PERMISSION = 0777;

//目錄最大權限
const unsigned short MAX_DIRECTORY_PERMISSION = 0666;

//默認umask(0022)
const unsigned short DEFAULT_UMASK = 0022;

//文件權限
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

// 最大命令長度
const unsigned int MAX_COMMAND_LENGTH = 200;
