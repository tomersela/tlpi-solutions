/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2024.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU Lesser General Public License as published   *
* by the Free Software Foundation, either version 3 or (at your option)   *
* any later version. This program is distributed without any warranty.    *
* See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.           *
\*************************************************************************/

/* Listing 59-1 */

/* read_line_mod.c

   Implementation of readLineBuf().
*/
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "read_line_mod.h"                  /* Declaration of readLineBuf() */
#include <stdlib.h>

/* Read characters from 'fd' until a newline is encountered. If a newline
  character is not encountered in the first (n - 1) bytes, then the excess
  characters are discarded. The returned string placed in 'buf' is
  null-terminated and includes the newline character if it was read in the
  first (n - 1) bytes. The function return value is the number of bytes
  placed in buffer (which includes the newline character if encountered,
  but excludes the terminating null byte). */

ssize_t
readLine(int fd, void *buffer, size_t n)
{
    ssize_t numRead;                    /* # of bytes fetched by last read() */
    size_t totRead;                     /* Total bytes read so far */
    char *buf;
    char ch;

    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }

    buf = buffer;                       /* No pointer arithmetic on "void *" */

    totRead = 0;
    for (;;) {
        numRead = read(fd, &ch, 1);

        if (numRead == -1) {
            if (errno == EINTR)         /* Interrupted --> restart read() */
                continue;
            else
                return -1;              /* Some other error */

        } else if (numRead == 0) {      /* EOF */
            if (totRead == 0)           /* No bytes read; return 0 */
                return 0;
            else                        /* Some bytes read; add '\0' */
                break;

        } else {                        /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }

            if (ch == '\n')
                break;
        }
    }

    *buf = '\0';
    return totRead;
}

void
readLineBufInit(int fd, struct rl_buf *rlbuf)
{
    rlbuf->fd = fd;
    rlbuf->next = NULL;
    rlbuf->size = BUF_SIZE;
    rlbuf->buf = malloc(BUF_SIZE);
    rlbuf->end_of_valid_data = NULL;
}


ssize_t
readLineBuf(struct rl_buf *rlbuf, char *buffer, size_t n)
{
    size_t read_size;
    size_t totRead = 0;

    char *buf_curr = buffer;
    
    while (totRead < n - 1) {
        if (rlbuf->next == NULL) {
            // rlbuf is empty
            read_size = read(rlbuf->fd, rlbuf->buf, rlbuf->size);
            if (read_size == -1) return -1;
            if (read_size == 0) { // EOF
                if (totRead > 0) { // we've read some data so far
                    *buf_curr = '\0';
                    return totRead;
                }
                return 0;
            }
            rlbuf->next = rlbuf->buf;
            rlbuf->end_of_valid_data = rlbuf->buf + read_size;
        }

        // find the end of next line in the buffer
        char *newline_pos = memchr(rlbuf->next, '\n', rlbuf->end_of_valid_data - rlbuf->next);
        if (newline_pos) {
            // copy the data up to the newline
            memcpy(buf_curr, rlbuf->next, newline_pos - rlbuf->next + 1);
            totRead += newline_pos - rlbuf->next + 1;
            buf_curr += newline_pos - rlbuf->next + 1;
            *buf_curr = '\0';
            rlbuf->next = newline_pos + 1;
            return totRead;
        }

        // no newline in the buffer
        // copy the entire buffer and set buffer to empty
        size_t available_in_buffer = rlbuf->end_of_valid_data - rlbuf->next;
        size_t space_in_output = n - 1 - totRead;
        size_t max_copy = (available_in_buffer > space_in_output) ? space_in_output : available_in_buffer;
        memcpy(buf_curr, rlbuf->next, max_copy);
        totRead += max_copy;
        buf_curr += max_copy;
        rlbuf->next = NULL; // need refill; set buffer to empty   
    }

    *buf_curr = '\0';
    return totRead - 1;
}


void
resetLineBufInit(int fd, struct rl_buf *rlbuf)
{
    rlbuf->fd = fd;
    rlbuf->next = NULL;
}

void
readLineBufCleanup(struct rl_buf *rlbuf)
{
    if (rlbuf->buf != NULL) {
        free(rlbuf->buf);
        rlbuf->buf = NULL;
    }
}
