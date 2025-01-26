#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    switch(vfork()) {
        case -1:
            errExit("vfork");
        case 0:
            printf("Child process is about to close fd 1\n");
            close(1);
            _exit(0);
        default:
            printf("Parent says: You can still see this!\n");
            exit(0);
    }
}
