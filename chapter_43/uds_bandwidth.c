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
    int block_num, block_size;
    char *buffer;
    long long start_time, end_time, elapsed_ns;
    double elapsed_sec, total_bytes, bandwidth;
    int status;
    char sync_byte;

    if (argc != 3)
        show_usage(argv[0]);

    block_num = getInt(argv[1], GN_GT_0, "num-blocks");
    block_size = getInt(argv[2], GN_GT_0, "block-size");

    buffer = malloc(block_size);
    if (buffer == NULL)
        errExit("malloc");
    memset(buffer, 'A', block_size);

    /* Create UNIX domain datagram socket pair */
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sockfd) == -1)
        errExit("socketpair");

    switch (child_pid = fork()) {
    case -1:
        errExit("fork");

    case 0: // child - writer
        set_cpu_affinity(0);  // pin child to core 0
        
        if (close(sockfd[0]) == -1)  // close read end
            errExit("close child read");

        /* Send sync byte */
        sync_byte = 1;
        if (send(sockfd[1], &sync_byte, 1, 0) != 1)
            errExit("send sync");

        /* Send data blocks as fast as possible */
        for (long i = 0; i < block_num; i++) {
            if (send(sockfd[1], buffer, block_size, 0) != block_size)
                errExit("send");
        }

        if (close(sockfd[1]) == -1)  // close write end
            errExit("close child write");

        free(buffer);
        _exit(EXIT_SUCCESS);

    default: // parent - reader
        set_cpu_affinity(1);  // pin parent to core 1
        
        if (close(sockfd[1]) == -1)  // close write end
            errExit("close parent write");

        /* Wait for sync byte */
        if (recv(sockfd[0], &sync_byte, 1, 0) != 1)
            errExit("recv sync");

        /* Start timing after sync */
        start_time = get_current_time_ns();

        /* Read all data blocks */
        int blocks_read = 0;
        while (blocks_read < block_num) {
            ssize_t bytes_received = recv(sockfd[0], buffer, block_size, 0);
            
            if (bytes_received == -1)
                errExit("recv");
            
            if (bytes_received == 0)
                break;  // connection closed
            
            if (bytes_received != block_size)
                errExit("incomplete datagram received");
                
            blocks_read++;
        }

        end_time = get_current_time_ns();

        if (close(sockfd[0]) == -1)  // close read end
            errExit("close parent read");

        /* Wait for child to complete */
        if (wait(&status) == -1)
            errExit("wait");

        elapsed_ns = end_time - start_time;
        elapsed_sec = elapsed_ns / 1000000000.0;
        total_bytes = (double)(blocks_read * block_size);
        bandwidth = total_bytes / elapsed_sec;

        printf("  Blocks read: %d\n", blocks_read);
        printf("  Bytes transferred: %.0f\n", total_bytes);
        printf("  Elapsed time: %.6f seconds\n", elapsed_sec);
        printf("  Bandwidth: %.2f MB/second\n", bandwidth / (1024.0 * 1024.0));

        break;
    }

    free(buffer);
    exit(EXIT_SUCCESS);
}
