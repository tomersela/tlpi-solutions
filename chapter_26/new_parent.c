#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    setbuf(stdout, NULL); // make stdout unbuffered

    switch (fork()) {
        case -1:
            errExit("fork");

        case 0:
            printf("Child: Parent id is now %d\n", getppid());
            printf("Child: Let's check again in 2 seconds... %d\n", getppid());
            sleep(2);
            printf("Child: Parent id is now %d\n", getppid());
            exit(EXIT_SUCCESS);

        default:
            printf("Parent: pid %d is about to end now\n", getpid());
            _exit(EXIT_SUCCESS);
    }
}
