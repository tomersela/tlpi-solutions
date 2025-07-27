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

/* pmq_file_client.c

   Send a message to the server pmq_file_server.c requesting the
   contents of the file named on the command line, and then receive the
   file contents via a series of messages sent back by the server. Display
   the total number of bytes and messages received. The server and client
   communicate using POSIX message queues.
*/
#define _GNU_SOURCE
#include <mqueue.h>

#include "pmq_file.h"

static char clientMq[CLIENT_MQ_NAME_LEN];

static void
removeQueue(void)
{
    if (mq_unlink(clientMq) == -1)
        errExit("mq_unlink");
}

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    struct responseMsg resp;
    int numMsgs;
    mqd_t serverMqd, clientMqd;
    ssize_t msgLen, totBytes;
    struct mq_attr mqattr;
    mqattr.mq_maxmsg = MQ_MAXMSG;
    mqattr.mq_msgsize = sizeof(struct responseMsg);

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    if (strlen(argv[1]) > sizeof(req.pathname) - 1)
        cmdLineErr("pathname too long (max: %ld bytes)\n",
                (long) sizeof(req.pathname) - 1);

    /* Get server's queue identifier; create queue for response */

    serverMqd = mq_open(SERVER_MQ, O_WRONLY);
    if (serverMqd == (mqd_t) -1)
        errExit("mq_open - server message queue");

    snprintf(clientMq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
            (long) getpid());
    clientMqd = mq_open(clientMq, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, &mqattr);
    if (clientMqd == (mqd_t) -1)
        errExit("Couldn't open client message queue - %s\n", clientMq);

    if (atexit(removeQueue) != 0)
        errExit("atexit");

    /* Send message asking for file named in argv[1] */

    req.pid = getpid();
    strncpy(req.pathname, argv[1], sizeof(req.pathname) - 1);
    req.pathname[sizeof(req.pathname) - 1] = '\0';
                                        /* Ensure string is terminated */

    if (mq_send(serverMqd, (void *) &req, sizeof(struct requestMsg), 0) == -1)
        errExit("mq_send");

    /* Get first response, which may be failure notification */

    msgLen = mq_receive(clientMqd, (void *) &resp, sizeof(struct responseMsg), NULL);
    if (msgLen == -1)
        errExit("mq_receive");

    if (resp.mtype == RESP_MT_FAILURE) {
        printf("%s\n", resp.data);      /* Display msg from server */
        exit(EXIT_FAILURE);
    }

    /* File was opened successfully by server; process messages
       (including the one already received) containing file data */

    totBytes = msgLen - RES_HEADER_SIZE;                  /* Count first message */
    for (numMsgs = 1; resp.mtype == RESP_MT_DATA; numMsgs++) {
        msgLen = mq_receive(clientMqd, (void *) &resp, sizeof(struct responseMsg), NULL);
        if (msgLen == -1)
            errExit("msgrcv");

        totBytes += (msgLen - RES_HEADER_SIZE);
    }

    printf("Received %ld bytes (%d messages)\n", (long) totBytes, numMsgs);

    exit(EXIT_SUCCESS);
}
