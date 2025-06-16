#define _BSD_SOURCE

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"
#include "helper.h"

/* Message structure for System V message queue */
struct msg_buf {
    long msg_type;
    char data[1];  // will be allocated dynamically
};

int
main(int argc, char *argv[])
{
    int msgq_id;
    pid_t child_pid;
    long block_num, block_size;
    struct msg_buf *msg_buffer;
    long long start_time, end_time, elapsed_ns;
    double elapsed_sec, total_bytes, bandwidth;
    int status;
    key_t key;
    size_t msg_buf_size;
    struct msg_buf sync_msg;

    if (argc != 3)
        show_usage(argv[0]);

    block_num = getLong(argv[1], GN_GT_0, "num-blocks");
    block_size = getLong(argv[2], GN_GT_0, "block-size");

    /* Skip test if block size exceeds 8192 bytes (POSIX mq limit) */
    if (block_size > 8192) {
        printf("  Block size %ld exceeds POSIX message queue limit (8192 bytes) - skipping\n", block_size);
        exit(EXIT_SUCCESS);
    }

    /* Calculate message buffer size */
    msg_buf_size = sizeof(long) + block_size;
    
    msg_buffer = malloc(msg_buf_size);
    if (msg_buffer == NULL)
        errExit("malloc");
    
    msg_buffer->msg_type = 1;
    memset(msg_buffer->data, 'A', block_size);

    /* Create unique key and message queue */
    key = ftok("/tmp", 'M');
    if (key == -1)
        errExit("ftok");

    msgq_id = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    if (msgq_id == -1)
        errExit("msgget");

    switch (child_pid = fork()) {
    case -1:
        errExit("fork");

    case 0: // child - writer
        set_cpu_affinity(0); // pin child to core 0

        sync_msg.msg_type = 999; // sync message type
        sync_msg.data[0] = 1;

        if (msgsnd(msgq_id, &sync_msg, 1, 0) == -1)
            errExit("msgsnd sync");

        // send data blocks as fast as possible
        for (long i = 0; i < block_num; i++) {
            if (msgsnd(msgq_id, msg_buffer, block_size, 0) == -1)
                errExit("msgsnd");
        }

        free(msg_buffer);
        _exit(EXIT_SUCCESS);

    default: // parent - reader
        set_cpu_affinity(1); // pin parent to core 1

        // wait for sync message
        if (msgrcv(msgq_id, &sync_msg, 1, 999, 0) == -1)
            errExit("msgrcv sync");

        start_time = get_current_time_ns(); // start timing after sync

        // read all data blocks
        long blocks_read = 0;
        while (blocks_read < block_num) {
            ssize_t bytes_received = msgrcv(msgq_id, msg_buffer, block_size, 1, 0);

            if (bytes_received == -1)
                errExit("msgrcv");

            if (bytes_received != block_size)
                errExit("incomplete message received");

            blocks_read++;
        }

        end_time = get_current_time_ns();

        if (wait(&status) == -1)
            errExit("wait");

        // remove message queue
        if (msgctl(msgq_id, IPC_RMID, NULL) == -1)
            errExit("msgctl");

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

    free(msg_buffer);
    exit(EXIT_SUCCESS);
}
