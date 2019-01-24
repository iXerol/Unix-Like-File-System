#ifndef Consts_h
#define Consts_h

#include <stdint.h>


const char* FILENAME = "volume.hex";


const unsigned short NADDR = 6;


const size_t BLOCK_SIZE = 512;


const unsigned int NADDR_BLOCK = BLOCK_SIZE / sizeof(unsigned int);


const size_t MAX_DIRECT_FILE_SIZE = (NADDR - 2) * BLOCK_SIZE;


const size_t MAX_LEVEL_1_FILE_SIZE = MAX_DIRECT_FILE_SIZE + NADDR_BLOCK * BLOCK_SIZE;


const size_t MAX_FILE_SIZE = MAX_LEVEL_1_FILE_SIZE + NADDR_BLOCK * NADDR_BLOCK * BLOCK_SIZE;


const unsigned int BLOCK_NUM = 400;


const size_t INODE_SIZE = 128;


const unsigned int INODE_NUM = 64;


const unsigned int INODE_BLOCK_START = 2;


const unsigned int DATA_BLOCK_START = INODE_BLOCK_START + INODE_NUM * INODE_SIZE / BLOCK_SIZE;

const size_t DATA_BLOCK_STACK_SIZE = 100;


const size_t FILE_NAME_LENGTH = 32 - sizeof(unsigned int);


const size_t USER_NAME_LENGTH = 32;


const size_t USER_PASSWORD_LENGTH = 32;


const size_t GROUP_NAME_LENGTH = 32;


const size_t USER_DATA_LENGTH = USER_NAME_LENGTH + USER_PASSWORD_LENGTH + GROUP_NAME_LENGTH;


const unsigned short MAX_FILE_PERMISSION = 0777;


const unsigned short MAX_DIRECTORY_PERMISSION = 0666;


const unsigned short DEFAULT_UMASK = 0022;


const unsigned short ISREG = 00000;        
const unsigned short ISDIR = 01000;        
const unsigned short ISCHR = 02000;        
const unsigned short ISBLK = 03000;        
const unsigned short ISFIFO = 04000;       
const unsigned short ISLNK = 05000;        
const unsigned short ISSOCK = 06000;       


const unsigned short IRWXU = 00700;       
const unsigned short IRUSR = 00400;       
const unsigned short IWUSR = 00200;       
const unsigned short IXUSR = 00100;       

const unsigned short IRWXG = 00070;       
const unsigned short IRGRP = 00040;       
const unsigned short IWGRP = 00020;       
const unsigned short IXGRP = 00010;       

const unsigned short IRWXO = 00007;       
const unsigned short IROTH = 00004;       
const unsigned short IWOTH = 00002;       
const unsigned short IXOTH = 00001;       


const unsigned int MAX_COMMAND_LENGTH = 200;

#endif
