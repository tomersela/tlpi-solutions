/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* us_abs_xfr.h

   Header file for us_abs_xfr_sv.c and us_abs_xfr_cl.c.

   These programs employ an abstract socket. Abstract sockets (a Linux-specific
   feature) are similar to UNIX domain sockets, but don't have a pathname;
   instead the socket address is distinguished by a string in an abstract
   namespace.
*/
#include <sys/un.h>
#include <sys/socket.h>
#include "tlpi_hdr.h"

#define SV_SOCK_PATH "\0us_abs_xfr"  /* Abstract socket (starts with null byte) */

#define BUF_SIZE 100
