#include <stdio.h>
#include "file.h"

int main(int argc, const char * argv[]) {
    if (fopen(FILENAME, "r")) {
        mount_volume();
    } else {
        new_volume();
    }
    show();

    save();
    return 0;
}
