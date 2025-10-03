#ifndef SENDFILE_H
#define SENDFILE_H

ssize_t us_sendfile(int out_fd, int in_fd, off_t *offset, size_t count);

#endif
