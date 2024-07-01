#include <sys/stat.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <limits.h>
#include "tlpi_hdr.h"

ssize_t myreadv(int fd, const struct iovec *iovector, int iovcnt);
ssize_t mywritev(int fd, const struct iovec *iov, int iovcnt);

int free_and_err(char *buffer_to_free);

int
main (int argc, char *argv[]) {
    if (argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s pathname\n", argv[0]);

    char *filename = argv[1];

    int fd;
    if ((fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) == -1)
        errExit("open");

    char first_name[10];
    char last_name[10];

    struct iovec vecs[] = {
        {
            .iov_base = &first_name,
            .iov_len = 10
        },
        {
            .iov_base = &last_name,
            .iov_len = 10
        },
    };

    ssize_t total_bytes = myreadv(fd, vecs, 2);
    if (total_bytes == -1) errExit("myreadv");
    first_name[9] = '\0';
    last_name[9] = '\0';

    printf("total bytes read: %ld\n", (long) total_bytes);
    printf("first name: %s\n", first_name);
    printf("last name: %s\n", last_name);

    printf("mywritev test:\n");
    total_bytes = mywritev(STDOUT_FILENO, vecs, 2);
    printf("\n");
    if (total_bytes == -1) errExit("mywritev");
}

ssize_t myreadv(int fd, const struct iovec *iov, int iovcnt) {
    size_t acc_size = 0;
    for (int i = 0; i < iovcnt; i++) {
        acc_size += iov[i].iov_len;
    }

    char *buffer = NULL;
    if ((buffer = malloc(acc_size)) == NULL) return free_and_err(buffer);

    int bytes_read;
    if ((bytes_read = read(fd, buffer, acc_size)) == -1) return free_and_err(buffer);
    
    int buffer_offset = 0;
    for (int i = 0; i < iovcnt; i++) {
        int curr_iov_len = iov[i].iov_len;
        memcpy(iov[i].iov_base, buffer + buffer_offset, curr_iov_len);
        buffer_offset += curr_iov_len;
    }
    return bytes_read;
}

ssize_t mywritev(int fd, const struct iovec *iov, int iovcnt) {
    size_t acc_size = 0;
    for (int i = 0; i < iovcnt; i++) {
        acc_size += iov[i].iov_len;
    }

    char *buffer = NULL;
    if ((buffer = malloc(acc_size)) == NULL) return free_and_err(buffer);

    int buffer_offset = 0;
    for (int i = 0; i < iovcnt; i++) {
        int curr_iov_len = iov[i].iov_len;
        memcpy(buffer + buffer_offset, iov[i].iov_base, curr_iov_len);
        buffer_offset += curr_iov_len;
    }


    int bytes_written;
    if ((bytes_written = write(fd, buffer, acc_size)) == -1) return free_and_err(buffer);

    return bytes_written;
}

int free_and_err(char *buffer_to_free) {
    free(buffer_to_free);
    return -1;
}
