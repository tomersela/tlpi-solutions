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

/* is_seqnum_cl_mod2.c

   A simple Internet stream socket client. This client requests a sequence
   number from the server.

   See also is_seqnum_sv_mod.c.
*/
#include <netdb.h>
#include "is_seqnum_mod2.h"

int
main(int argc, char *argv[])
{
    char *reqLenStr;                    /* Requested length of sequence */
    char seqNumStr[INT_LEN];            /* Start of granted sequence */
    ssize_t numRead;
    struct rl_buf rlbuf;

    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s server-host [sequence-len]\n", argv[0]);

    int cfd = inetConnect(argv[1], PORT_NUM, SOCK_STREAM);
    if (cfd == -1)
        fatal("inetConnect() failed");

    /* Send requested sequence length, with terminating newline */

    reqLenStr = (argc > 2) ? argv[2] : "1";
    if (write(cfd, reqLenStr, strlen(reqLenStr)) !=  strlen(reqLenStr))
        fatal("Partial/failed write (reqLenStr)");
    if (write(cfd, "\n", 1) != 1)
        fatal("Partial/failed write (newline)");

    /* Read and display sequence number returned by server */
    readLineBufInit(cfd, &rlbuf);
    numRead = readLineBuf(&rlbuf, seqNumStr, INT_LEN);
    if (numRead == -1)
        errExit("readLine");
    if (numRead == 0)
        fatal("Unexpected EOF from server");

    printf("Sequence number: %s", seqNumStr);   /* Includes '\n' */

    readLineBufCleanup(&rlbuf);                 /* Clean up allocated buffer (not needed, but good practice) */
    exit(EXIT_SUCCESS);                         /* Closes 'cfd' */
}
