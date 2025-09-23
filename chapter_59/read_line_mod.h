/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Header file for Exercise 59-1 */

/* read_line_mod.h

   Header file for read_line_mod.c.
*/
#ifndef READ_LINE_MOD_H
#define READ_LINE_MOD_H

#include <sys/types.h>

#define BUF_SIZE 1024

struct rl_buf {
   int fd;
   int size;
   char *buf;
   char *next;
   char *end_of_valid_data;
};

ssize_t readLine(int fd, void *buffer, size_t n);

void readLineBufInit(int fd, struct rl_buf *rlbuf);

ssize_t readLineBuf(struct rl_buf *rlbuf, char *buffer, size_t n);

void resetLineBufInit(int fd, struct rl_buf *rlbuf);

void readLineBufCleanup(struct rl_buf *rlbuf);

#endif
