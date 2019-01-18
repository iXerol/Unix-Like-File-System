#include <stdio.h>
#include "file.h"

int main(int argc, const char * argv[]) {
    printf("%lu\n%lu\n%lu\n", sizeof(struct superblock_t), sizeof(struct inode_t), sizeof(struct child_file_t));
    if (fopen(FILENAME, "r")) {
        mount_volume();
    } else {
        new_volume();
    }
    show();
    return 0;
}
