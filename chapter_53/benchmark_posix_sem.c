#include <sys/time.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include "tlpi_hdr.h"

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s num-operations\n", progName);
    fprintf(stderr, "  num-operations: number of increment/decrement cycles to perform\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageError(argv[0]);

    long numOps = getLong(argv[1], GN_GT_0, "num-operations");
    
    // create unique semaphore name
    char semName[64];
    snprintf(semName, sizeof(semName), "/posix_bench_%d_%ld", getpid(), (long) time(NULL));
    
    printf("POSIX Semaphore Benchmark\n");
    printf("=========================\n");
    printf("Operations: %ld increment/decrement cycles\n", numOps);
    
    // create semaphore with initial value 1
    sem_t *sem = sem_open(semName, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED)
        errExit("sem_open");
    
    // get start time
    struct timeval startTime, endTime;
    if (gettimeofday(&startTime, NULL) == -1)
        errExit("gettimeofday");
    
    // perform the benchmark: increment and decrement semaphore numOps times
    for (long i = 0; i < numOps; i++) {
        // decrement (wait)
        if (sem_wait(sem) == -1)
            errExit("sem_wait");
        
        // increment (post)
        if (sem_post(sem) == -1)
            errExit("sem_post");
    }
    
    // get end time
    if (gettimeofday(&endTime, NULL) == -1)
        errExit("gettimeofday");
    
    // calculate elapsed time
    double elapsed = (endTime.tv_sec - startTime.tv_sec) + 
                    (endTime.tv_usec - startTime.tv_usec) / 1000000.0;
    
    printf("Elapsed time: %.6f seconds\n", elapsed);
    printf("Operations per second: %.0f\n", (numOps * 2) / elapsed); // *2 because each cycle is wait+post
    printf("Average time per operation: %.9f seconds\n", elapsed / (numOps * 2));
    
    // cleanup
    if (sem_close(sem) == -1)
        errExit("sem_close");
    
    if (sem_unlink(semName) == -1)
        errExit("sem_unlink");
    
    exit(EXIT_SUCCESS);
}
