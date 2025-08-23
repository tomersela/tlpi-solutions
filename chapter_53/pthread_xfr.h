/* pthread_xfr.h
   
   Header file for the threaded version of the data transfer program.
   Uses POSIX threads and POSIX semaphores instead of System V IPC.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#ifndef BUF_SIZE                /* Allow "cc -D" to override definition */
#define BUF_SIZE 1024           /* Size of transfer buffer */
#endif

/* Structure for the shared buffer between threads */
struct shared_buffer {
    int cnt;                    /* Number of bytes used in 'buf' */
    char buf[BUF_SIZE];         /* Data being transferred */
};

/* Global variables for synchronization */
extern struct shared_buffer g_buffer;
extern sem_t write_sem;         /* Writer can write to buffer */
extern sem_t read_sem;          /* Reader can read from buffer */


void *writer_thread(void *arg);
void *reader_thread(void *arg);
