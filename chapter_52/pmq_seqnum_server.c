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

/* pmq_seqnum_server.c

   An example of a server using a message queue to handle client requests.
   The "service" provided is the allocation of unique sequential
   numbers. Each client submits a request consisting of its PID, and
   the length of the sequence it is to be allocated by the server.
   The PID is used by both the server and the client to construct
   the name of the messagq queue used by the client for receiving responses.

   The server reads each client request, and uses the client's message queue
   to send back the starting value of the sequence allocated to that
   client. The server then increments its counter of used numbers
   by the length specified in the client request.

   See pmq_seqnum.h for the format of request and response messages.

   The client is in pmq_seqnum_client.c.
*/
#include <signal.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pmq_seqnum.h"

int
main(int argc, char *argv[])
{
    mqd_t serverMqd, clientMqd;
    ssize_t numRead;
    char clientMq[CLIENT_MQ_NAME_LEN];
    struct request req;
    struct response resp;
    int seqNum = 0;                     /* This is our "service" */
    
    struct mq_attr mqattr;
    mqattr.mq_maxmsg = MQ_MAXMSG;
    mqattr.mq_msgsize = sizeof(struct request);

    /* Create well-known message queue, and open it for reading */
    
    umask(0);                           /* So we get the permissions we want */
    serverMqd = mq_open(SERVER_MQ, O_CREAT, S_IRUSR | S_IWUSR | S_IWGRP, &mqattr);
    if (serverMqd == (mqd_t) -1)
        errExit("mq_open (server)");
    
    for (;;) {                          /* Read requests and send responses */
        numRead = mq_receive(serverMqd, (void *) &req, sizeof(struct request), NULL);
        if (numRead == -1) {
            fprintf(stderr, "Error reading request; discarding\n");
            continue;                   /* Either partial read or error */
        }
        /* Open client message queue (previously created by client) */

        snprintf(clientMq, CLIENT_MQ_NAME_LEN, CLIENT_MQ_TEMPLATE,
                (long) req.pid);
        clientMqd = mq_open(clientMq, O_WRONLY);
        if (clientMqd == (mqd_t) -1) {
            errMsg("mq_open %s", clientMq);
            continue;
        }

        /* Send response and close message queue */

        resp.seqNum = seqNum;
        if (mq_send(clientMqd, (void *) &resp, sizeof(struct response), 0) == -1)
            fprintf(stderr, "Error writing to message queue %s\n", clientMq);
        if (mq_close(clientMqd) == -1)
            errMsg("close");

        seqNum += req.seqLen;           /* Update our sequence number */
    }
}
