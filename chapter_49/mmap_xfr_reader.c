/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Excersize 49-2 */

/* mmap_xfr_reader.c

   Read data from shared memory using a binary semaphore lock-step
   protocol; see mmap_xfr_writer.c
*/
#include <fcntl.h>
#include <sys/mman.h>
#include "mmap_xfr.h"

int
main(int argc, char *argv[])
{
    int semid, xfrs, bytes;
    int tmp_file_fd;
    struct sharedseg *shmp;

    /* Get IDs for semaphore set and shared memory created by writer */

    semid = semget(SEM_KEY, 0, 0);
    if (semid == -1)
        errExit("semget");

    tmp_file_fd = open(MMAP_FILE, O_RDONLY);
    if (tmp_file_fd == -1)
        errExit("open tmp");
    
    if ((shmp = (struct sharedseg *) mmap(NULL, sizeof(struct sharedseg),
                                            PROT_READ, MAP_SHARED,
                                            tmp_file_fd, 0)) == MAP_FAILED)
        errExit("mmap - buffer");

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

    if (munmap((void *) shmp, sizeof(struct sharedseg)) == -1)
        errExit("munmap - tmp file");

    if (close(tmp_file_fd) == -1)
        errExit("close");

    /* Give writer one more turn, so it can clean up */

    if (releaseSem(semid, WRITE_SEM) == -1)
        errExit("releaseSem");

    fprintf(stderr, "Received %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}
