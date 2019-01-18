#ifndef inode_h
#define inode_h
#include <time.h>
#include "consts.h"

struct inode_t {
    unsigned int number;		//inode number
    unsigned int data_address[NADDR];		//Number of data blocks where the file stored.
    unsigned short link_count;		//连接数
    unsigned short mode;		//文件類型及权限
    char user[32];		//文件所属用户
    char group[32];		//文件所属组
    size_t size;		//文件大小
    time_t created_time;		//文件創建時間
    time_t modified_time;        //文件創建時間
    time_t accessed_time;        //文件創建時間
};

#endif /* inode_h */
