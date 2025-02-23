/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 30-1 */

/* thread_incr.c

   This program employs two POSIX threads that increment the same global
   variable, without using any synchronization method. As a consequence,
   updates are sometimes lost.

   See also thread_incr_mutex.c.
*/
#include <pthread.h>
#include "tlpi_hdr.h"

struct thread_args {
    int id;
    int loops;
};

static volatile int glob = 0;   /* "volatile" prevents compiler optimizations
                                   of arithmetic operations on 'glob' */
static void *                   /* Loop 'arg' times incrementing 'glob' */
threadFunc(void *arg)
{
    struct thread_args targs = *((struct thread_args *) arg);
    int id = targs.id;
    int loops = targs.loops;
    int loc, j;

    for (j = 0; j < loops; j++) {
        loc = glob;
        loc++;
        glob = loc;
        printf("[%d] glob = %d\n", id, glob);
    }

    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int loops, s;
    struct thread_args t1_args, t2_args;

    loops = (argc > 1) ? getInt(argv[1], GN_GT_0, "num-loops") : 10000000;
    t1_args.id = 1;
    t2_args.id = 2;
    t1_args.loops = loops;
    t2_args.loops = loops;

    s = pthread_create(&t1, NULL, threadFunc, &t1_args);
    if (s != 0)
        errExitEN(s, "pthread_create");
    s = pthread_create(&t2, NULL, threadFunc, &t2_args);
    if (s != 0)
        errExitEN(s, "pthread_create");

    s = pthread_join(t1, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");
    s = pthread_join(t2, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");

    printf("glob = %d\n", glob);
    exit(EXIT_SUCCESS);
}
