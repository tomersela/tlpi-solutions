/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Exercise 59-3 */

/* us_xfr_sv_mod.c

   An example UNIX stream socket server. Accepts incoming connections
   and copies data sent from clients to stdout.

   See also us_xfr_cl_mod.c.
*/
#include "us_xfr.h"
#include "unix_sockets.h"
#define BACKLOG 5

int
main(int argc, char *argv[])
{
    int sfd, cfd;
    ssize_t numRead;
    char buf[BUF_SIZE];

    sfd = unixListen(SV_SOCK_PATH, BACKLOG);
    if (sfd == -1)
        errExit("unixListen");

    for (;;) {          /* Handle client connections iteratively */

        /* Accept a connection. The connection is returned on a new
           socket, 'cfd'; the listening socket ('sfd') remains open
           and can be used to accept further connections. */

        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1)
            errExit("accept");

        /* Transfer data from connected socket to stdout until EOF */

        while ((numRead = read(cfd, buf, BUF_SIZE)) > 0)
            if (write(STDOUT_FILENO, buf, numRead) != numRead)
                fatal("partial/failed write");

        if (numRead == -1)
            errExit("read");

        if (close(cfd) == -1)
            errMsg("close");
    }

    /* Clean up socket file on exit (never reached, but good practice) */
    unixRemove(SV_SOCK_PATH);
}
