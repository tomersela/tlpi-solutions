/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 52-1 */

/* pmsg_receive_to.c

   Usage as shown in usageError().

   Receive a message from a POSIX message queue, and write it on
   standard output.

   See also pmsg_send.c.

   Linux supports POSIX message queues since kernel 2.6.6.
*/
#define _POSIX_C_SOURCE 199309
#include <time.h>
#include <mqueue.h>
#include <fcntl.h>              /* For definition of O_NONBLOCK */
#include "tlpi_hdr.h"

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s [options] mq-name\n", progName);
    fprintf(stderr, "options:\n");
    fprintf(stderr, "    -n                          Use O_NONBLOCK flag\n");
    fprintf(stderr, "    -t [timeout in seconds]     Timeout for receiving a message\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    int flags, opt;
    mqd_t mqd;
    unsigned int prio;
    void *buffer;
    struct mq_attr attr;
    ssize_t numRead;
    struct timespec timeout;
    bool timeout_set = FALSE;
    timeout.tv_sec = 0;
    timeout.tv_nsec = 0;

    flags = O_RDONLY;
    while ((opt = getopt(argc, argv, "nt:")) != -1) {
        switch (opt) {
        case 'n':   flags |= O_NONBLOCK;        break;
        case 't':
            timeout_set = TRUE;
            int seconds = getInt(optarg, GN_GT_0, "timeout");
            if (clock_gettime(CLOCK_REALTIME, &timeout) == -1)
                errExit("clock_gettime");
            timeout.tv_sec += seconds;
            break;
        default:    usageError(argv[0]);
        }
    }

    if (optind >= argc)
        usageError(argv[0]);

    mqd = mq_open(argv[optind], flags);
    if (mqd == (mqd_t) -1)
        errExit("mq_open");

    /* We need to know the 'mq_msgsize' attribute of the queue in
       order to determine the size of the buffer for mq_receive() / mq_timedreceive() */

    if (mq_getattr(mqd, &attr) == -1)
        errExit("mq_getattr");

    buffer = malloc(attr.mq_msgsize);
    if (buffer == NULL)
        errExit("malloc");

    if (timeout_set) {
        numRead = mq_timedreceive(mqd, buffer, attr.mq_msgsize, &prio, &timeout);
        if (numRead == -1)
            errExit("mq_timedreceive");
    } else {
        numRead = mq_receive(mqd, buffer, attr.mq_msgsize, &prio);
        if (numRead == -1)
            errExit("mq_timedreceive");
    }

    printf("Read %ld bytes; priority = %u\n", (long) numRead, prio);
    if (write(STDOUT_FILENO, buffer, numRead) == -1)
        errExit("write");
    write(STDOUT_FILENO, "\n", 1);

    exit(EXIT_SUCCESS);
}
