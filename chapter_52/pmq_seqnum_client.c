/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 52-2 */

/* pmq_seqnum_client.c

   A simple client that uses a well-known message queue to request a (trivial)
   "sequence number service". This client creates its own message queue (using a
   convention agreed upon by client and server) which is used to receive a reply
   from the server. The client then sends a request to the server consisting of
   its PID and the length of the sequence it wishes to be allocated. The client
   then reads the server's response and displays it on stdout.

   See pmq_seqnum.h for the format of request and response messages.

   The server is in pmq_seqnum_server.c.
*/
#include <mqueue.h>
#include "pmq_seqnum.h"

static char clientMq[CLIENT_MQ_NAME_LEN];

static void             /* Invoked on exit to delete client message queue */
removeMessageQueue(void)
{
    mq_unlink(clientMq);
}

int
main(int argc, char *argv[])
{
    mqd_t serverMqd, clientMqd;
    struct request req;
    struct response resp;
    struct mq_attr mqattr;
    mqattr.mq_maxmsg = MQ_MAXMSG;
    mqattr.mq_msgsize = sizeof(struct response);

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [seq-len]\n", argv[0]);

    /* Create our nessage queue (before sending request, to avoid a race) */

    umask(0);                   /* So we get the permissions we want */
    snprintf(clientMq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
            (long) getpid());
    clientMqd = mq_open(clientMq, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR | S_IWGRP, &mqattr);
    if (clientMqd == (mqd_t) -1)
        errExit("mq_open");

    if (atexit(removeMessageQueue) != 0)
        errExit("atexit");

    /* Construct request message, open server message queue, and send message */

    req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    serverMqd = mq_open(SERVER_MQ, O_WRONLY);
    if (serverMqd == (mqd_t) -1)
        errExit("mq_open %s (server)", SERVER_MQ);

    if (mq_send(serverMqd, (void *) &req, sizeof(struct request), 0) == -1)
        fatal("Can't write to server");

    /* read and display response */

    if (mq_receive(clientMqd, (void *) &resp, sizeof(struct response), 0) == -1)
        fatal("Can't read response from server");

    printf("%d\n", resp.seqNum);
    exit(EXIT_SUCCESS);
}
