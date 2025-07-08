/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 24-6 */

/* fork_sig_sync.c

   Demonstrate how signals can be used to synchronize the actions
   of a parent and child process.
*/
#include <sys/ipc.h>
#include <sys/sem.h>
#include "curr_time.h"                  /* Declaration of currTime() */
#include "tlpi_hdr.h"
#include "binary_sems.h"

int
main(int argc, char *argv[])
{
    int semid;
    pid_t childPid;

    // create a new semaphore
    if ((semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600)) == -1)
        errExit("semget");

    // mark semaphore as in-use
    initSemInUse(semid, 0);

    setbuf(stdout, NULL);               /* Disable buffering of stdout */

    switch (childPid = fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child */

        /* Child does some required action here... */

        printf("[%s %ld] Child started - doing some work\n",
                currTime("%T"), (long) getpid());
        sleep(2);               /* Simulate time spent doing some work */

        /* And then signals parent that it's done */

        printf("[%s %ld] Child about to unblock the parent\n",
                currTime("%T"), (long) getpid());

        releaseSem(semid, 0); // release the semaphore

        /* Now child can do other things... */

        _exit(EXIT_SUCCESS);

    default: /* Parent */

        /* Parent may do some work here, and then waits for child to
           complete the required action */

        printf("[%s %ld] Parent about to wait for child\n",
                currTime("%T"), (long) getpid());
        
        reserveSem(semid, 0); // block until child release the semaphore
        printf("[%s %ld] Parent continues\n", currTime("%T"), (long) getpid());

        /* Parent carries on to do other things... */

        exit(EXIT_SUCCESS);
    }
}
