#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    char buffer[100];

    switch(fork()) {
        case -1:
            errExit("fork");

        case 0: // child
            sleep(1);
            printf("child attempt to read...\n");
            if (read(STDIN_FILENO, buffer, sizeof(buffer)) == -1) {
                errExit("read");
            }
            _exit(EXIT_SUCCESS);
        default: // parent
            printf("parent exiting\n");
            exit(EXIT_SUCCESS);
    }
}
