/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 52-3 */

/* pmq_file_server.c

   A file server that uses POSIX message queues to handle client requests
   (see pmq_file_client.c). The client sends an initial request containing
   the name of the desired file, and the identifier of the message queue to be
   used to send the file contents back to the child. The server attempts to
   open the desired file. If the file cannot be opened, a failure response is
   sent to the client, otherwise the contents of the requested file are sent
   in a series of messages.

   This application makes use of multiple message queues. The server maintains
   a queue (with a well-known key) dedicated to incoming client requests. Each
   client creates its own private queue, which is used to pass response
   messages from the server back to the client.

   This program operates as a concurrent server, forking a new child process to
   handle each client request while the parent waits for further client requests.
*/
#define _GNU_SOURCE
#include <mqueue.h>
#include <signal.h>

#include "pmq_file.h"

static void             /* SIGCHLD handler */
grimReaper(int sig)
{
    int savedErrno;

    savedErrno = errno;                 /* waitpid() might change 'errno' */
    while (waitpid(-1, NULL, WNOHANG) > 0)
        continue;
    errno = savedErrno;
}

static void             /* Executed in child process: serve a single client */
serveRequest(const struct requestMsg *req)
{
    int fd;
    ssize_t numRead;
    char clientMq[CLIENT_MQ_NAME_LEN];
    mqd_t clientMqd;
    struct responseMsg resp;

    /* Open client message queue (previously created by client) */

    snprintf(clientMq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
            (long) req->pid);
    clientMqd = mq_open(clientMq, O_WRONLY);
    if (clientMqd == (mqd_t) -1) {
        errMsg("Couldn't open client message queue - %s\n", clientMq);
        return;
    }   

    fd = open(req->pathname, O_RDONLY);
    if (fd == -1) {                     /* Open failed: send error text */
        resp.mtype = RESP_MT_FAILURE;
        snprintf(resp.data, sizeof(resp.data), "%s", "Couldn't open");
        mq_send(clientMqd, (void *) &resp, sizeof(struct responseMsg), 0);
        exit(EXIT_FAILURE);             /* and terminate */
    }

    /* Transmit file contents in messages with type RESP_MT_DATA. We don't
       diagnose read() and mq_send() errors since we can't notify client. */

    resp.mtype = RESP_MT_DATA;
    while ((numRead = read(fd, resp.data, RESP_DATA_SIZE)) > 0)
        if (mq_send(clientMqd, (void *) &resp, RES_HEADER_SIZE + numRead, 0) == -1)
            break;

    /* Send a message of type RESP_MT_END to signify end-of-file */

    resp.mtype = RESP_MT_END;
    mq_send(clientMqd, (void *) &resp, RES_HEADER_SIZE, 0);

    if (mq_close(clientMqd) == -1)
      errMsg("mq_close - %s (%d)", clientMq, (int) clientMqd);
}

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    pid_t pid;
    ssize_t msgLen;
    mqd_t serverMqd;
    struct sigaction sa;
    struct mq_attr mqattr;
    mqattr.mq_maxmsg = MQ_MAXMSG;
    mqattr.mq_msgsize = sizeof(struct requestMsg);

    /* Create server message queue */
    serverMqd = mq_open(SERVER_MQ, O_CREAT, S_IRUSR | S_IWUSR | S_IWGRP, &mqattr);
    if (serverMqd == (mqd_t) -1)
        errExit("mq_open");

    /* Establish SIGCHLD handler to reap terminated children */

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = grimReaper;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
        errExit("sigaction");

    /* Read requests, handle each in a separate child process */

    for (;;) {
        msgLen = mq_receive(serverMqd, (void *) &req, sizeof(struct requestMsg), NULL);
        if (msgLen == -1) {
            if (errno == EINTR)         /* Interrupted by SIGCHLD handler? */
                continue;               /* ... then restart mq_receive() */
            errMsg("mq_receive");           /* Some other error */
            break;                      /* ... so terminate loop */
        }

        pid = fork();                   /* Create child process */
        if (pid == -1) {
            errMsg("fork");
            break;
        }

        if (pid == 0) {                 /* Child handles request */
            serveRequest(&req);
            _exit(EXIT_SUCCESS);
        }

        /* Parent loops to receive next client request */
    }

    /* If mq_receive() or fork() fails, remove server MQ and exit */

    if (mq_unlink(SERVER_MQ) == -1)
        errExit("mq_unlink");
    
    exit(EXIT_SUCCESS);
}
