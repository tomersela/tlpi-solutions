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

/*  svshm_xfr_writer.c

   Read buffers of data data from standard input into a System V shared memory
   segment from which it is copied by svshm_xfr_reader.c

   We use a pair of binary semaphores to ensure that the writer and reader have
   exclusive, alternating access to the shared memory. (I.e., the writer writes
   a block of text, then the reader reads, then the writer writes etc). This
   ensures that each block of data is processed in turn by the writer and
   reader.

   This program needs to be started before the reader process as it creates the
   shared memory and semaphores used by both processes.

   Together, these two programs can be used to transfer a stream of data through
   shared memory as follows:

        $ mmap_xfr_writer < infile &
        $ mmap_xfr_reader > out_file
*/
#include <fcntl.h>
#include <sys/mman.h>

#include "semun.h"              /* Definition of semun union */
#include "mmap_xfr.h"

int
main(int argc, char *argv[])
{
    int semid, bytes, xfrs;
    int tmp_file_fd;
    struct sharedseg *shmp;
    union semun dummy;

    /* Create set containing two semaphores; initialize so that
       writer has first access to shared memory. */

    semid = semget(SEM_KEY, 2, IPC_CREAT | OBJ_PERMS);
    if (semid == -1)
        errExit("semget");

    if (initSemAvailable(semid, WRITE_SEM) == -1)
        errExit("initSemAvailable");
    if (initSemInUse(semid, READ_SEM) == -1)
        errExit("initSemInUse");

    /* Create shared memory; attach at address chosen by system */

    tmp_file_fd = open(MMAP_FILE, O_RDWR | O_CREAT | O_TRUNC,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (tmp_file_fd == -1)
        errExit("open tmp");
    
    // set the size of the target file
    if (ftruncate(tmp_file_fd, sizeof(struct sharedseg)) == -1)
        errExit("ftruncate");
    
    if ((shmp = (struct sharedseg *) mmap(NULL, sizeof(struct sharedseg),
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED, tmp_file_fd, 0)) == MAP_FAILED)
        errExit("mmap - buffer");

    /* Transfer blocks of data from stdin to shared memory */

    for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt) {
        if (reserveSem(semid, WRITE_SEM) == -1)         /* Wait for our turn */
            errExit("reserveSem");

        shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
        if (shmp->cnt == -1)
            errExit("read");

        if (releaseSem(semid, READ_SEM) == -1)          /* Give reader a turn */
            errExit("releaseSem");

        /* Have we reached EOF? We test this after giving the reader
           a turn so that it can see the 0 value in shmp->cnt. */

        if (shmp->cnt == 0)
            break;
    }

    /* Wait until reader has let us have one more turn. We then know
       reader has finished, and so we can delete the IPC objects. */

    if (reserveSem(semid, WRITE_SEM) == -1)
        errExit("reserveSem");

    if (semctl(semid, 0, IPC_RMID, dummy) == -1)
        errExit("semctl");
    
    if (munmap((void *) shmp, sizeof(struct sharedseg)) == -1)
        errExit("munmap - tmp file");

    if (close(tmp_file_fd) == -1)
        errExit("close");
    

    fprintf(stderr, "Sent %d bytes (%d xfrs)\n", bytes, xfrs);
    exit(EXIT_SUCCESS);
}
