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

/* pmq_seqnum.h

   Header file used by pmq_seqnum_server.c and pmq_seqnum_client.c

   These programs create a POSIX message queue. This makes it easy to compile and
   run the programs.
*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define MQ_MAXMSG 10

#define SERVER_MQ "/seqnum_sv"
                                /* Well-known name for server's message queue */
#define CLIENT_MQ_TEMPLATE "/seqnum_cl.%ld"
                                /* Template for building client message queue name */
#define CLIENT_MQ_NAME_LEN (sizeof(CLIENT_MQ_TEMPLATE) + 20)
                                /* Space required for client queue name
                                  (+20 as a generous allowance for the PID) */

struct request {                /* Request (client --> server) */
    pid_t pid;                  /* PID of client */
    int seqLen;                 /* Length of desired sequence */
};

struct response {               /* Response (server --> client) */
    int seqNum;                 /* Start of sequence */
};
