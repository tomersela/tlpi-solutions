/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 59-2 */

/* is_seqnum_sv_mod2.c

   A simple Internet stream socket server. Our service is to provide
   unique sequence numbers to clients.

   Usage:  is_seqnum_sv_mod2 [init-seq-num]
                        (default = 0)

   See also is_seqnum_cl_mod2.c.
*/
#define _BSD_SOURCE             /* To get definitions of NI_MAXHOST and
                                   NI_MAXSERV from <netdb.h> */
#include <netdb.h>
#include "is_seqnum_mod2.h"

#define BACKLOG 50

int
main(int argc, char *argv[])
{
    uint32_t seqNum;
    char reqLenStr[INT_LEN];            /* Length of requested sequence */
    char seqNumStr[INT_LEN];            /* Start of granted sequence */
    int cfd, reqLen;
    socklen_t addrlen;
#define ADDRSTRLEN (NI_MAXHOST + NI_MAXSERV + 10)
    char addrStr[ADDRSTRLEN];
    struct rl_buf rlbuf;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [init-seq-num]\n", argv[0]);

    seqNum = (argc > 1) ? getInt(argv[1], 0, "init-seq-num") : 0;

    /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)    errExit("signal");

    int lfd = inetListen(PORT_NUM, 5, &addrlen);
    if (lfd == -1)
        fatal("inetListen() failed");
    
    struct sockaddr *claddr = malloc(addrlen);
    if (claddr == NULL)
        errExit("malloc");

    readLineBufInit(-1, &rlbuf);

    for (;;) {                  /* Handle clients iteratively */

        /* Accept a client connection, obtaining client's address */

        socklen_t alen = addrlen;
        cfd = accept(lfd, claddr, &alen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        inetAddressStr(claddr, alen, addrStr, IS_ADDR_STR_LEN);
        printf("Connection from %s\n", addrStr);

        /* Read client request, send sequence number back */

        resetLineBufInit(cfd, &rlbuf);
        if (readLineBuf(&rlbuf, reqLenStr, INT_LEN) <= 0) {
            close(cfd);
            continue;                   /* Failed read; skip request */
        }

        reqLen = atoi(reqLenStr);
        if (reqLen <= 0) {              /* Watch for misbehaving clients */
            close(cfd);
            continue;                   /* Bad request; skip it */
        }

        snprintf(seqNumStr, INT_LEN, "%d\n", seqNum);
        if (write(cfd, seqNumStr, strlen(seqNumStr)) != strlen(seqNumStr))
            fprintf(stderr, "Error on write");

        seqNum += reqLen;               /* Update sequence number */

        if (close(cfd) == -1)           /* Close connection */
            errMsg("close");
    }

    /* This line is never reached, but good practice to clean up */
    readLineBufCleanup(&rlbuf);
}
