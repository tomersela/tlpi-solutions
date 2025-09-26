/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 60-2 */

/* is_echo_sv_mod2.c

   An implementation of the TCP "echo" service that can be run either
   as a standalone daemon or as an inetd service.

   Usage:
   - Standalone mode: ./is_echo_sv_mod2
   - inetd mode: ./is_echo_sv_mod2 -i

   NOTE: this program must be run under a root login, in order to allow the
   "echo" port (7) to be bound. Alternatively, for test purposes, you can
   replace the SERVICE name below with a suitable unreserved port number
   (e.g., "51000"), and make a corresponding change in the client.

   See also is_echo_cl.c.
*/
#include <signal.h>
#include <syslog.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "become_daemon.h"
#include "inet_sockets.h"       /* Declarations of inet*() socket functions */
#include "tlpi_hdr.h"

#define SERVICE "echo"          /* Name of TCP service */
#define BUF_SIZE 4096

static void             /* SIGCHLD handler to reap dead child processes */
grimReaper(int sig)
{
    int savedErrno;             /* Save 'errno' in case changed here */

    savedErrno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

/* Handle a client request: copy input to output */

static void
handleRequest(int rfd, int wfd)
{
    char buf[BUF_SIZE];
    ssize_t numRead;

    while ((numRead = read(rfd, buf, BUF_SIZE)) > 0) {
        if (write(wfd, buf, numRead) != numRead) {
            syslog(LOG_ERR, "write() failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (numRead == -1) {
        syslog(LOG_ERR, "Error from read(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char *argv[])
{
    int lfd, cfd;               /* Listening and connected sockets */
    struct sigaction sa;
    int inetdMode = 0;

    /* Check for -i option (inetd mode) */
    if (argc == 2 && strcmp(argv[1], "-i") == 0) {
        inetdMode = 1;
    } else if (argc != 1) {
        fprintf(stderr, "Usage: %s [-i]\n", argv[0]);
        fprintf(stderr, "  -i  run in inetd mode\n");
        exit(EXIT_FAILURE);
    }

    if (inetdMode) {
        /* inetd mode: handle single client on stdin/stdout */
        handleRequest(STDIN_FILENO, STDOUT_FILENO);
        exit(EXIT_SUCCESS);
    }

    /* Standalone mode: run as daemon */
    if (becomeDaemon(0) == -1)
        errExit("becomeDaemon");

    /* Establish SIGCHLD handler to reap terminated child processes */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error from sigaction(): %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    lfd = inetListen(SERVICE, 10, NULL);
    if (lfd == -1) {
        syslog(LOG_ERR, "Could not create server socket (%s)", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (;;) {
        cfd = accept(lfd, NULL, NULL);  /* Wait for connection */
        if (cfd == -1) {
            syslog(LOG_ERR, "Failure in accept(): %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* Handle each client request in a new child process */

        switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Can't create child (%s)", strerror(errno));
            close(cfd);                 /* Give up on this client */
            break;                      /* May be temporary; try next client */

        case 0:                         /* Child */
            close(lfd);                 /* Unneeded copy of listening socket */
            handleRequest(cfd, cfd);
            _exit(EXIT_SUCCESS);

        default:                        /* Parent */
            close(cfd);                 /* Unneeded copy of connected socket */
            break;                      /* Loop to accept next connection */
        }
    }
}
