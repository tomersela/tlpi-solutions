#include "tlpi_hdr.h"
#include "rdwrn.h"
#include "sendfile.h"

#define BUF_SIZE 4096


ssize_t
us_sendfile(int out_fd, int in_fd, off_t *offset, size_t count)
{
    char buf[BUF_SIZE];
    ssize_t total_written = 0;
    ssize_t bytes_read, bytes_written;
    size_t remaining = count;

    while (remaining > 0) {
        size_t read_size = (remaining < BUF_SIZE) ? remaining : BUF_SIZE;

        // sendfile() offset semantics:
        // - if offset is NULL: read from current file position and advance it (use read())
        // - if offset is not NULL: read from specified offset without changing file position (use pread())
        //   and update *offset to reflect bytes read
        if (offset == NULL) {
            bytes_read = read(in_fd, buf, read_size);
        } else {
            bytes_read = pread(in_fd, buf, read_size, *offset + total_written);
        }

        if (bytes_read == -1)
            return -1;

        if (bytes_read == 0) // EOF
            break;

        bytes_written = writen(out_fd, buf, bytes_read);
        if (bytes_written == -1)
            return -1;

        total_written += bytes_written;
        remaining -= bytes_written;

        if (offset != NULL)
            *offset += bytes_written;
    }

    return total_written;
}
