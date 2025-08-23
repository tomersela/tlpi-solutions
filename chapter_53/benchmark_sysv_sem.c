#include <sys/time.h>
#include <sys/sem.h>
#include <time.h>
#include "tlpi_hdr.h"
#include "semun.h"

static void
usageError(const char *progName)
{
    fprintf(stderr, "Usage: %s num-operations\n", progName);
    fprintf(stderr, "  num-operations: number of increment/decrement cycles to perform\n");
    exit(EXIT_FAILURE);
}

// helper function to perform semaphore operation
static int
semOp(int semId, int semNum, int op)
{
    struct sembuf sop;
    
    sop.sem_num = semNum;
    sop.sem_op = op;
    sop.sem_flg = 0;  // no special flags for this benchmark
    
    return semop(semId, &sop, 1);
}

int
main(int argc, char *argv[])
{
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageError(argv[0]);

    long numOps = getLong(argv[1], GN_GT_0, "num-operations");
    
    printf("System V Semaphore Benchmark\n");
    printf("============================\n");
    printf("Operations: %ld increment/decrement cycles\n", numOps);
    
    // create semaphore set with 1 semaphore
    int semId = semget(IPC_PRIVATE, 1, IPC_CREAT | IPC_EXCL | 0644);
    if (semId == -1)
        errExit("semget");
    
    // initialize semaphore to value 1
    union semun arg;
    arg.val = 1;
    if (semctl(semId, 0, SETVAL, arg) == -1)
        errExit("semctl SETVAL");
    
    // get start time
    struct timeval startTime, endTime;
    if (gettimeofday(&startTime, NULL) == -1)
        errExit("gettimeofday");
    
    // perform the benchmark: increment and decrement semaphore numOps times
    for (long i = 0; i < numOps; i++) {
        // decrement (wait) - subtract 1
        if (semOp(semId, 0, -1) == -1)
            errExit("semop wait");
        
        // increment (post) - add 1
        if (semOp(semId, 0, 1) == -1)
            errExit("semop post");
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
    
    // cleanup - remove semaphore set
    if (semctl(semId, 0, IPC_RMID) == -1)
        errExit("semctl IPC_RMID");
    
    exit(EXIT_SUCCESS);
}
