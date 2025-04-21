#define _POSIX_C_SOURCE 199309

#include <signal.h>

#include "tlpi_hdr.h"

static void
handler(int sig)
{
    const char message[] = "\nGot SIGTTIN!\n";
    write(STDOUT_FILENO, message, sizeof(message));
}

int
main(int argc, char *argv[])
{
    struct sigaction sa;
    pid_t pid;

    switch(pid = fork()) {
        case -1:
            errExit("fork");

        case 0: // child

            printf("Child: PID=%ld PGID=%ld\n", (long) getpid(), (long) getpgrp());

            if (argc > 1) {
                printf("set handler for SIGTTIN\n");

                sigemptyset(&sa.sa_mask);
                sa.sa_flags = 0;
                sa.sa_handler = handler;

                if (sigaction(SIGTTIN, &sa, NULL) == -1)
                    errExit("sigaction");
            }

            alarm(30);      /* Ensure each process eventually terminates */
            
            for (;;)
                pause();        /* Wait for signals */

        default: // parent
            printf("Parent: PID=%ld PGID=%ld\n", (long) getpid(), (long) getpgrp());
            sleep(5);
            printf("Parent exiting...");
            _exit(EXIT_SUCCESS);
    }

}
