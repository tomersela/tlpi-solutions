/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Solution for Exercise 54-1 */

/* pshm_xfr_reader.c

   Read data from a POSIX shared memory using a binary semaphore lock-step
   protocol; see pshm_xfr_writer.c
*/
#include <fcntl.h>
#include <sys/mman.h>

#include "pshm_xfr.h"

int
main(int argc, char *argv[])
{
    int semid, shmfd, xfrs, bytes;
    struct stat sb;
    struct shmseg *shmp;

    /* Get IDs for semaphore set and shared memory created by writer */

    semid = semget(SEM_KEY, 0, 0);
    if (semid == -1)
        errExit("semget");

    shmfd  = shm_open(SHM_NAME, O_RDONLY, 0);
    if (shmfd == -1)
        errExit("shmget");

    /* Attach shared memory read-only, as we will only read */

    if (fstat(shmfd, &sb) == -1)
        errExit("fstat");

    shmp = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, shmfd, 0);
    if (shmp == MAP_FAILED)
        errExit("mmap");

    /* Transfer blocks of data from shared memory to stdout */

    for (xfrs = 0, bytes = 0; ; xfrs++) {
        if (reserveSem(semid, READ_SEM) == -1)          /* Wait for our turn */
            errExit("reserveSem");

        if (shmp->cnt == 0)                     /* Writer encountered EOF */
            break;
        bytes += shmp->cnt;

        if (write(STDOUT_FILENO, shmp->buf, shmp->cnt) != shmp->cnt)
            fatal("partial/failed write");

        if (releaseSem(semid, WRITE_SEM) == -1)         /* Give writer a turn */
            errExit("releaseSem");
    }

    if (munmap(shmp, sb.st_size) == -1)
        errExit("munmap");

    /* Give writer one more turn, so it can clean up */

    if (releaseSem(semid, WRITE_SEM) == -1)
        errExit("releaseSem");

    fprintf(stderr, "Received %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}
