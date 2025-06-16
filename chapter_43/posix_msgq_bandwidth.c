#define _BSD_SOURCE

#include <mqueue.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "tlpi_hdr.h"
#include "helper.h"

int
main(int argc, char *argv[])
{
    long block_size, block_num;
    char *buffer;
    char *sync_msg;
    char *sync_recv_buf;
    double elapsed_sec, total_bytes, bandwidth;
    long long start_time, end_time, elapsed_ns;
    int status;
    unsigned int prio = 0;
    mqd_t mq;
    struct mq_attr attr;
    const char *mq_name = "/bandwidth_test_mq";

    if (argc != 3)
        show_usage(argv[0]);

    block_num = getLong(argv[1], GN_GT_0, "num-blocks");
    block_size = getLong(argv[2], GN_GT_0, "block-size");

    /* Skip test if block size exceeds 8192 bytes (POSIX mq limit) */
    if (block_size > 8192) {
        printf("  Block size %ld exceeds POSIX message queue limit (8192 bytes) - skipping\n", block_size);
        exit(EXIT_SUCCESS);
    }

    /* Allocate buffers */
    buffer = malloc(block_size);
    sync_msg = malloc(block_size);
    sync_recv_buf = malloc(block_size);
    if (buffer == NULL || sync_msg == NULL || sync_recv_buf == NULL)
        errExit("malloc");

    /* Initialize buffer with test data */
    memset(buffer, 'A', block_size);
    memset(sync_msg, 'S', block_size);  /* Sync message with same size */

    /* Remove any existing message queue */
    mq_unlink(mq_name);

    /* Set message queue attributes - use smaller maxmsg to avoid system limits */
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;  /* Keep it small to avoid EINVAL */
    attr.mq_msgsize = block_size;
    attr.mq_curmsgs = 0;

    /* Create message queue before forking to avoid race condition */
    mq = mq_open(mq_name, O_CREAT | O_RDWR, 0666, &attr);
    if (mq == (mqd_t) -1)
        errExit("mq_open create");

    switch (fork()) {
    case -1:
        errExit("fork");

    case 0: /* Child - writer */
        set_cpu_affinity(0); // pin child to core 0
        
        // reopen queue for writing only
        if (mq_close(mq) == -1)
            errExit("mq_close child setup");
        
        mq = mq_open(mq_name, O_WRONLY);
        if (mq == (mqd_t) -1)
            errExit("mq_open child");

        // send sync message first
        if (mq_send(mq, sync_msg, block_size, 0) == -1)
            errExit("mq_send sync");

        // send all data blocks
        for (long i = 0; i < block_num; i++) {
            if (mq_send(mq, buffer, block_size, 0) == -1)
                errExit("mq_send");
        }

        // close message queue
        if (mq_close(mq) == -1)
            errExit("mq_close child");

        exit(EXIT_SUCCESS);

    default: /* Parent - reader */
        set_cpu_affinity(1); // pin parent to core 1
        
        // message queue already open from before fork, wait for sync message
        if (mq_receive(mq, sync_recv_buf, block_size, &prio) == -1)
            errExit("mq_receive sync");

        start_time = get_current_time_ns(); // start timing

        // read all data blocks
        long blocks_read = 0;
        while (blocks_read < block_num) {
            ssize_t bytes_received = mq_receive(mq, buffer, block_size, &prio);
            
            if (bytes_received == -1)
                errExit("mq_receive");
            
            if (bytes_received != block_size)
                errExit("incomplete message received");
                
            blocks_read++;
        }

        end_time = get_current_time_ns();

        // close and unlink message queue
        if (mq_close(mq) == -1)
            errExit("mq_close parent");
        
        if (mq_unlink(mq_name) == -1)
            errExit("mq_unlink");

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
    free(sync_msg);
    free(sync_recv_buf);
    exit(EXIT_SUCCESS);
}
