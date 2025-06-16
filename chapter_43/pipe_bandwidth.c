#define _BSD_SOURCE
#define _GNU_SOURCE

#include <sys/time.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"
#include "helper.h"

#define WRITE_END   1
#define READ_END    0

int
main(int argc, char *argv[])
{
    int pipefd[2];
    pid_t child_pid;
    long block_num, block_size;
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

    if (pipe(pipefd) == -1)
        errExit("pipe");

    switch (child_pid = fork()) {
    case -1:
        errExit("fork");

    case 0: // child - writer
        set_cpu_affinity(0);  // pin child to core 0
        
        if (close(pipefd[READ_END]) == -1) // close read end of pipe
            errExit("close - child");

        // sync with parent - send ready signal
        sync_byte = 1;
        if (write(pipefd[WRITE_END], &sync_byte, 1) != 1)
            errExit("sync write");

        // write data blocks as fast as possible
        for (long i = 0; i < block_num; i++) {
            if (write(pipefd[WRITE_END], buffer, block_size) != block_size) {
                if (errno == EPIPE) {
                    // Parent has closed read end
                    break;
                } else {
                    errExit("write");
                }
            }
        }

        // close write end to signal EOF to parent
        if (close(pipefd[WRITE_END]) == -1)
            errExit("close - child write end");

        free(buffer);
        _exit(EXIT_SUCCESS);

    default: // parent - reader
        set_cpu_affinity(1);  // pin parent to core 1
        
        if (close(pipefd[WRITE_END]) == -1) // close write end of pipe
            errExit("close - parent");

        // sync with child - wait for ready signal
        if (read(pipefd[READ_END], &sync_byte, 1) != 1)
            errExit("sync read");

        // start timing after sync
        start_time = get_current_time_ns();

        // read all data blocks
        long blocks_read = 0;
        while (blocks_read < block_num) {
            ssize_t bytesRead = read(pipefd[READ_END], buffer, block_size);
            
            if (bytesRead == -1)
                errExit("read");
            
            if (bytesRead == 0)
                break;  // EOF (child has closed write end)
            
            if (bytesRead != block_size) { // partial read
                // continue reading
                int totalRead = bytesRead;
                while (totalRead < block_size) {
                    bytesRead = read(pipefd[READ_END], buffer + totalRead, 
                                   block_size - totalRead);
                    if (bytesRead == -1)
                        errExit("read");
                    if (bytesRead == 0)
                        break;  // EOF
                    totalRead += bytesRead;
                }

                if (totalRead == block_size)
                    blocks_read++;
                else
                    errExit("incomplete block due to EOF");
            } else {
                blocks_read++;
            }
        }

        end_time = get_current_time_ns();

        // close read end of pipe
        if (close(pipefd[READ_END]) == -1)
            errExit("close - parent read end");

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
