/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 26-4 */

/* make_zombie.c

   Demonstrate how a child process becomes a zombie in the interval between
   the time it exits, and the time its parent performs a wait (or exits, at
   which time it is adopted by init(8), which does a wait, thus releasing
   the zombie).
*/
#include <signal.h>
#include <libgen.h>             /* For basename() declaration */
#include <sys/wait.h>
#include "tlpi_hdr.h"

#define CMD_SIZE 200

#define SYNC_SIG SIGUSR1

static void             /* Signal handler - does nothing but return */
handler(int sig)
{
}

int
main(int argc, char *argv[])
{
    char cmd[CMD_SIZE];
    pid_t childPid;
    struct sigaction sa;
    sigset_t sig_mask, old_mask, wait_mask;
    siginfo_t sig_info;

    setbuf(stdout, NULL);       /* Disable buffering of stdout */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = handler;
    if (sigaction(SYNC_SIG, &sa, NULL) == -1)
        errExit("sigaction");
    
    // block the sync signal before forking
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SYNC_SIG);
    sigprocmask(SIG_BLOCK, &sig_mask, &old_mask);

    printf("Parent PID=%ld\n", (long) getpid());

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0:     /* Child: signal parent, exits to become zombie */
        // signal parent
        if (kill(getppid(), SYNC_SIG) == -1)
            errExit("kill");
        
        printf("Child (PID=%ld) exiting\n", (long) getpid());
        _exit(EXIT_SUCCESS);

    default:    /* Parent */
        wait_mask = old_mask;
        sigdelset(&wait_mask, SYNC_SIG);
        
        // waiting for child to signal parent
        if (sigsuspend(&wait_mask) == -1 && errno != EINTR)
            errExit("sigsuspend");
        
        snprintf(cmd, CMD_SIZE, "ps -f | grep %s  | grep -v grep", basename(argv[0]));
        system(cmd);            /* View zombie child */

        /* Now send the "sure kill" signal to the zombie */

        if (kill(childPid, SIGKILL) == -1)
            errMsg("kill");

        if (waitid(P_PGID, getpgrp(), &sig_info,
            WEXITED | WSTOPPED | WCONTINUED |
                WNOWAIT // WNOWAIT is important here as we don't want to remove the zombie process from the process list
         ) == -1)
            errExit("waitid");
        printf("After sending SIGKILL to zombie (PID=%ld):\n", (long) childPid);
        system(cmd);            /* View zombie child again */

        exit(EXIT_SUCCESS);
    }
}
