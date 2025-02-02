#include <sys/wait.h>

#include "tlpi_hdr.h"


int
main(int argc, char *argv[])
{
    pid_t parent_pid;

    setbuf(stdout, NULL); // make stdout unbuffered
    printf("Grandparent: Creating parent process...\n");

    switch (parent_pid = fork()) {
        case -1:
            errExit("grandparent-fork");

        case 0:
            printf("Parent: Creating child process...\n");
            switch (fork()) {
                case -1:
                    errExit("parent-fork");

                case 0:
                    printf("Child: Sleeping for 1 second...\n");
                    sleep(1);
                    printf("Child: Parent id is now %d\n", getppid());
                    exit(EXIT_SUCCESS);

                default:
                    printf("Parent: bye bye!\n");
                    _exit(EXIT_SUCCESS);
            }

        default:
            printf("Grandparent: I'm going to take a 2 seconds nap...\n");
            sleep(2);
            printf("Grandparent: waiting for parent...\n");
            wait(NULL);
            sleep(1);
            printf("Grandparent: exiting...\n");
            exit(EXIT_SUCCESS);
    }
}
