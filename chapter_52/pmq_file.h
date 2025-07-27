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

/* pmq_file.h

   Header file for pmq_file_server.c and pmq_file_client.c.
*/
#include <stddef.h>                     /* For definition of offsetof() */
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include "tlpi_hdr.h"

#define SERVER_MQ "/file_server"

struct requestMsg {                     /* Requests (client to server) */
    pid_t pid;                          /* PID of the client process */
    char pathname[PATH_MAX];            /* File to be returned */
};

#define MQ_MAXMSG 10

#define CLIENT_MQ_TEMPLATE "/file_cl.%ld"
                                /* Template for building client message queue name */
#define CLIENT_MQ_NAME_LEN (sizeof(CLIENT_MQ_TEMPLATE) + 20)

#define RESP_DATA_SIZE 4096

struct responseMsg {                    /* Responses (server to client) */
    long mtype;                         /* One of RESP_MT_* values below */
    char data[RESP_DATA_SIZE];           /* File content / response message */
};

#define RES_HEADER_SIZE (offsetof(struct responseMsg, data))

/* Types for response messages sent from server to client */

#define RESP_MT_FAILURE 1               /* File couldn't be opened */
#define RESP_MT_DATA    2               /* Message contains file data */
#define RESP_MT_END     3               /* File data complete */
