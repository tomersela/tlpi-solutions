/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Listing 46-9 */

/* svmsg_file_client.c

   Send a message to the server svmsg_file_server.c requesting the
   contents of the file named on the command line, and then receive the
   file contents via a series of messages sent back by the server. Display
   the total number of bytes and messages received. The server and client
   communicate using System V message queues.
*/
#include "svmsg_file_mod.h"

#define TIMEOUT_SEC 5

static int clientId;

static void
sigalrmHandler(int sig) {
    // Just interrupt system calls
}

static void setAlarmHandler(void) {
    struct sigaction sa;
    sa.sa_handler = sigalrmHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; // Important: DO NOT set SA_RESTART

    if (sigaction(SIGALRM, &sa, NULL) == -1)
        errExit("sigaction - SIGALRM");
}

static void
removeQueue(void)
{
    if (msgctl(clientId, IPC_RMID, NULL) == -1)
        errExit("msgctl");
}

static int
safeMsgsnd(int qid, const void *msgp, size_t msgsz, int timeoutSec) {
    alarm(timeoutSec);
    int res = msgsnd(qid, msgp, msgsz, 0);
    alarm(0);

    if (res == -1) {
        if (errno == EINTR)
            errExit("msgsnd() timed out");
        errExit("msgsnd()");
    }

    return res;
}

static ssize_t
safeMsgrcv(int qid, void *msgp, size_t msgsz, long type, int timeoutSec) {
    alarm(timeoutSec);
    ssize_t len = msgrcv(qid, msgp, msgsz, type, 0);
    alarm(0);

    if (len == -1) {
        if (errno == EINTR)
            errExit("msgrcv() timed out");
        errExit("msgrcv()");
    }

    return len;
}

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    struct responseMsg resp;
    int serverId, numMsgs;
    ssize_t msgLen, totBytes;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    if (strlen(argv[1]) > sizeof(req.pathname) - 1)
        cmdLineErr("pathname too long (max: %ld bytes)\n",
                (long) sizeof(req.pathname) - 1);

    setAlarmHandler();

    /* Get server's queue identifier; create queue for response */

    FILE *keyFile = fopen(KEY_FILE, "r");
    if (keyFile == NULL)
        errExit("fopen");

    if (fscanf(keyFile, "%d", &serverId) != 1)
        errExit("fscanf - server message queue");

    fclose(keyFile);

    clientId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWGRP);
    if (clientId == -1)
        errExit("msgget - client message queue");

    if (atexit(removeQueue) != 0)
        errExit("atexit");

    /* Send message asking for file named in argv[1] */

    req.mtype = 1;                      /* Any type will do */
    req.clientId = clientId;
    strncpy(req.pathname, argv[1], sizeof(req.pathname) - 1);
    req.pathname[sizeof(req.pathname) - 1] = '\0';
                                        /* Ensure string is terminated */

    safeMsgsnd(serverId, &req, REQ_MSG_SIZE, TIMEOUT_SEC);

    /* Get first response, which may be failure notification */

    msgLen = safeMsgrcv(clientId, &resp, RESP_MSG_SIZE, 0, TIMEOUT_SEC);

    if (resp.mtype == RESP_MT_FAILURE) {
        printf("%s\n", resp.data);      /* Display msg from server */
        exit(EXIT_FAILURE);
    }

    /* File was opened successfully by server; process messages
       (including the one already received) containing file data */

    totBytes = msgLen;                  /* Count first message */
    for (numMsgs = 1; resp.mtype == RESP_MT_DATA; numMsgs++) {
        msgLen = safeMsgrcv(clientId, &resp, RESP_MSG_SIZE, 0, TIMEOUT_SEC);

        totBytes += msgLen;
    }

    printf("Received %ld bytes (%d messages)\n", (long) totBytes, numMsgs);

    exit(EXIT_SUCCESS);
}
