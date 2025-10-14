/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2025.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 63-4 - Modified to use poll() instead of select() */

/*  select_self_pipe.c

   Employ the self-pipe trick so that we can avoid race conditions while both
   selecting on a set of file descriptors and also waiting for a signal.

   Usage as shown in synopsis below; for example:

        select_self_pipe - 0
*/
#include <sys/time.h>
#if ! defined(__hpux)   /* HP-UX 11 doesn't have this header file */
#include <sys/select.h>
#endif
#include <poll.h>
#include <fcntl.h>
#include <signal.h>
#include "tlpi_hdr.h"

static int pfd[2];                      /* File descriptors for pipe */

static void
handler(int sig)
{
    int savedErrno;                     /* In case we change 'errno' */

    savedErrno = errno;
    if (write(pfd[1], "x", 1) == -1 && errno != EAGAIN)
        errExit("write");
    errno = savedErrno;
}

int
main(int argc, char *argv[])
{
    struct pollfd *pollFds;
    int ready, nfds, flags;
    struct timeval timeout;
    int pollTimeout;
    struct timeval *pto;
    struct sigaction sa;
    char ch;
    int fd, j;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s {timeout|-} fd...\n"
                "\t\t('-' means infinite timeout)\n", argv[0]);

    /* Initialize 'timeout', 'pollFds', and 'nfds' for poll() */

    if (strcmp(argv[1], "-") == 0) {
        pto = NULL;                     /* Infinite timeout */
        pollTimeout = -1;
    } else {
        pto = &timeout;
        timeout.tv_sec = getLong(argv[1], 0, "timeout");
        timeout.tv_usec = 0;            /* No microseconds */
        pollTimeout = timeout.tv_sec * 1000;
    }

    nfds = argc - 2 + 1;                /* fd arguments + pipe */

    /* Allocate pollfd array */

    pollFds = calloc(nfds, sizeof(struct pollfd));
    if (pollFds == NULL)
        errExit("calloc");

    /* Build the 'pollFds' from the fd numbers given in command line */

    for (j = 2; j < argc; j++) {
        fd = getInt(argv[j], 0, "fd");
        pollFds[j - 2].fd = fd;
        pollFds[j - 2].events = POLLIN;
    }

    /* Create pipe before establishing signal handler to prevent race */

    if (pipe(pfd) == -1)
        errExit("pipe");

    pollFds[nfds - 1].fd = pfd[0];      /* Add read end of pipe to 'pollFds' */
    pollFds[nfds - 1].events = POLLIN;

    /* Make read and write ends of pipe nonblocking */

    flags = fcntl(pfd[0], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make read end nonblocking */
    if (fcntl(pfd[0], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    flags = fcntl(pfd[1], F_GETFL);
    if (flags == -1)
        errExit("fcntl-F_GETFL");
    flags |= O_NONBLOCK;                /* Make write end nonblocking */
    if (fcntl(pfd[1], F_SETFL, flags) == -1)
        errExit("fcntl-F_SETFL");

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;           /* Restart interrupted reads()s */
    sa.sa_handler = handler;
    if (sigaction(SIGINT, &sa, NULL) == -1)
        errExit("sigaction");

    while ((ready = poll(pollFds, nfds, pollTimeout)) == -1 &&
            errno == EINTR)
        continue;                       /* Restart if interrupted by signal */
    if (ready == -1)                    /* Unexpected error */
        errExit("poll");

    if (pollFds[nfds - 1].revents & POLLIN) {   /* Handler was called */
        printf("A signal was caught\n");

        for (;;) {                      /* Consume bytes from pipe */
            if (read(pfd[0], &ch, 1) == -1) {
                if (errno == EAGAIN)
                    break;              /* No more bytes */
                else
                    errExit("read");    /* Some other error */
            }
        }

        /* Perform any actions that should be taken in response to signal */
        printf("Handling signal now!\n");
    }

    /* Examine file descriptor sets returned by poll() to see
       which other file descriptors are ready */

    printf("ready = %d\n", ready);
    for (j = 2; j < argc; j++) {
        fd = getInt(argv[j], 0, "fd");
        if (pollFds[j - 2].revents & POLLIN) {
            char buffer[256];
            read(fd, buffer, sizeof(buffer));   /* Drain input */
            printf("%d: r\n", fd);
        } else {
            printf("%d: \n", fd);
        }
    }

    /* And check if read end of pipe is ready */

    printf("%d: %s   (read end of pipe)\n", pfd[0],
            (pollFds[nfds - 1].revents & POLLIN) ? "r" : "");

    if (pto != NULL)
        printf("timeout after select(): %ld.%03ld\n",
               (long) timeout.tv_sec, (long) timeout.tv_usec / 1000);

    free(pollFds);
    exit(EXIT_SUCCESS);
}
