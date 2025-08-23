#include "tlpi_hdr.h"
#include "pthread_xfr.h"


/* Global shared buffer and synchronization primitives */
struct shared_buffer g_buffer;
sem_t write_sem;                /* Writer can write to buffer */
sem_t read_sem;                 /* Reader can read from buffer */


/* Writer thread: reads from stdin and writes to shared buffer */
void *
writer_thread(void *arg)
{
    int bytes = 0, xfrs = 0;
    
    for (;;) {
        /* Wait for our turn to write */
        if (sem_wait(&write_sem) == -1)
            errExit("sem_wait write_sem");
        
        /* Read data from stdin into the shared buffer */
        g_buffer.cnt = read(STDIN_FILENO, g_buffer.buf, BUF_SIZE);
        if (g_buffer.cnt == -1)
            errExit("read");
        
        bytes += g_buffer.cnt;
        xfrs++;
        
        /* Signal the reader that data is available */
        if (sem_post(&read_sem) == -1)
            errExit("sem_post read_sem");
        
        /* If we reached EOF, break out of the loop */
        if (g_buffer.cnt == 0)
            break;
    }
    
    fprintf(stderr, "Writer: sent %d bytes (%d xfrs)\n", bytes, xfrs);
    return NULL;
}


/* Reader thread: reads from shared buffer and writes to stdout */
void *
reader_thread(void *arg)
{
    int bytes = 0, xfrs = 0;
    
    for (;;) {
        /* Wait for data to be available */
        if (sem_wait(&read_sem) == -1)
            errExit("sem_wait read_sem");
        
        /* Check if writer encountered EOF */
        if (g_buffer.cnt == 0)
            break;
        
        bytes += g_buffer.cnt;
        xfrs++;
        
        /* Write data from shared buffer to stdout */
        if (write(STDOUT_FILENO, g_buffer.buf, g_buffer.cnt) != g_buffer.cnt)
            fatal("partial/failed write");
        
        /* Signal the writer that we're done with this buffer */
        if (sem_post(&write_sem) == -1)
            errExit("sem_post write_sem");
    }
    
    fprintf(stderr, "Reader: received %d bytes (%d xfrs)\n", bytes, xfrs);
    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t writer_tid, reader_tid;
    int s;
    
    /* Initialize semaphores:
       - write_sem starts at 1 (writer can start immediately)
       - read_sem starts at 0 (reader must wait for data)
    */
    if (sem_init(&write_sem, 0, 1) == -1)
        errExit("sem_init write_sem");
    
    if (sem_init(&read_sem, 0, 0) == -1)
        errExit("sem_init read_sem");
    
    /* Create writer thread */
    s = pthread_create(&writer_tid, NULL, writer_thread, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create writer");
    
    /* Create reader thread */
    s = pthread_create(&reader_tid, NULL, reader_thread, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create reader");
    
    /* Wait for both threads to complete */
    s = pthread_join(writer_tid, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join writer");
    
    s = pthread_join(reader_tid, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join reader");
    
    /* Clean up semaphores */
    if (sem_destroy(&write_sem) == -1)
        errExit("sem_destroy write_sem");
    
    if (sem_destroy(&read_sem) == -1)
        errExit("sem_destroy read_sem");
    
    exit(EXIT_SUCCESS);
}
