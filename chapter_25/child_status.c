#include <sys/wait.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    int child_status = 0;
    switch (fork()) {
        case -1:
            errExit("fork");

        case 0:
            printf("Child is about to have a tantrum!\n");
            fflush(stdout);
            return -1;

        default:
            wait(&child_status);
            break;
    }

    printf("Parent thinks positive: %d (0x%x)\n", WEXITSTATUS(child_status), WEXITSTATUS(child_status));
}
