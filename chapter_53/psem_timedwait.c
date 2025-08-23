/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 53-2 */

/* psemtimed_wait.c

   Decrease the value of a POSIX named semaphore.

   See also psem_post.c.

   On Linux, named semaphores are supported with kernel 2.6 or later, and
   a glibc that provides the NPTL threading implementation.
*/
#define _XOPEN_SOURCE 600

#include <semaphore.h>
#include <time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    sem_t *sem;
    struct timespec ts;

    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s sem-name timeout\n", argv[0]);

    sem = sem_open(argv[1], 0);
    if (sem == SEM_FAILED)
        errExit("sem_open");

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        errExit("clock_gettime");

    ts.tv_sec += getInt(argv[2], GN_GT_0, "timeout");

    if (sem_timedwait(sem, &ts) == -1)
        errExit("sem_timedwait");

    printf("%ld sem_timedwait() succeeded\n", (long) getpid());
    exit(EXIT_SUCCESS);
}
