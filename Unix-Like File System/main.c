#include <stdio.h>
#include "file.h"
#include "UI.h"

int main(int argc, const char * argv[]) {
    if (fopen(FILENAME, "r")) {
        mount_volume();
    } else {
        new_volume();
    }

    while (true) {
        UI_login();
        printf("Login successfully.\n");
        UI_command();
        save();
    }
    return 0;
}
