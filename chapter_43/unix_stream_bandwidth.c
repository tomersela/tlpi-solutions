#define _BSD_SOURCE
#define _GNU_SOURCE

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"
#include "helper.h"

int
main(int argc, char *argv[])
{
    int sockfd[2];
    pid_t child_pid;
    long block_num, block_size;
    char *buffer;
    long long start_time, end_time, elapsed_ns;
    double elapsed_sec, total_bytes, bandwidth;
    int status;
    char sync_byte;

    if (argc != 3)
        show_usage(argv[0]);

    block_num = getLong(argv[1], GN_GT_0, "num-blocks");
    block_size = getLong(argv[2], GN_GT_0, "block-size");

    buffer = malloc(block_size);
    if (buffer == NULL)
        errExit("malloc");
    memset(buffer, 'A', block_size);

    /* Create UNIX domain socket pair */
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) == -1)
        errExit("socketpair");

    switch (child_pid = fork()) {
    case -1:
        errExit("fork");

    case 0: // child - writer
        set_cpu_affinity(0); // pin child to core 0

        if (close(sockfd[0]) == -1) // close read end
            errExit("close child read");

        sync_byte = 1; // send sync byte
        if (write(sockfd[1], &sync_byte, 1) != 1)
            errExit("write sync");

        // send data blocks as fast as possible
        for (long i = 0; i < block_num; i++) {
            ssize_t bytes_written = 0;
            while (bytes_written < block_size) {
                ssize_t result = write(sockfd[1], buffer + bytes_written, 
                                     block_size - bytes_written);
                if (result == -1)
                    errExit("write");
                bytes_written += result;
            }
        }

        if (close(sockfd[1]) == -1) // close write end
            errExit("close child write");

        free(buffer);
        _exit(EXIT_SUCCESS);

    default: // parent - reader
        set_cpu_affinity(1); // pin parent to core 1

        if (close(sockfd[1]) == -1) // close write end
            errExit("close parent write");

        // wait for sync byte
        if (read(sockfd[0], &sync_byte, 1) != 1)
            errExit("read sync");

        start_time = get_current_time_ns(); // start timing after sync

        // read all data blocks
        long blocks_read = 0;
        while (blocks_read < block_num) {
            ssize_t bytes_read = 0;
            while (bytes_read < block_size) {
                ssize_t result = read(sockfd[0], buffer + bytes_read, 
                                    block_size - bytes_read);
                if (result == -1)
                    errExit("read");
                if (result == 0)
                    break; // EOF
                bytes_read += result;
            }

            if (bytes_read == block_size)
                blocks_read++;
            else
                break; // incomplete block due to EOF
        }

        end_time = get_current_time_ns();

        if (close(sockfd[0]) == -1) // close read end
            errExit("close parent read");

        if (wait(&status) == -1)
            errExit("wait");

        elapsed_ns = end_time - start_time;
        elapsed_sec = elapsed_ns / 1000000000.0;
        total_bytes = (double)(blocks_read * block_size);
        bandwidth = total_bytes / elapsed_sec;

        printf("  Blocks read: %ld\n", blocks_read);
        printf("  Bytes transferred: %.0f\n", total_bytes);
        printf("  Elapsed time: %.6f seconds\n", elapsed_sec);
        printf("  Bandwidth: %.2f MB/second\n", bandwidth / (1024.0 * 1024.0));

        break;
    }

    free(buffer);
    exit(EXIT_SUCCESS);
}
