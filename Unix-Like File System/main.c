#include <stdio.h>
#include "file.h"
#include "UI.h"

int main(int argc, const char * argv[]) {
    if (fopen(FILENAME, "r")) {
        mount_volume();
    } else {
        new_volume();
    }

    UI_login();
    
//    show();

//    present_working_directory();
//    status(current_working_inode, "/");
//    list("/etc");
    save();
    return 0;
}
