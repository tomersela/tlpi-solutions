#include <assert.h>
#include <sys/sem.h>

#include "tlpi_hdr.h"
#include "binary_sems_mod.h"

int
main(int argc, char* argv[])
{
    int semId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    assert(semId != -1);
    assert(initSemAvailable(semId, 0) != -1);

    printf("Test 1: reserveSemNB should succeed on available semaphore: ");
    assert(reserveSemNB(semId, 0) == 0);
    printf("OK\n");

    printf("Test 2: reserveSemNB should fail with EAGAIN on locked semaphore: ");
    assert(reserveSemNB(semId, 0) == -1);
    assert(errno == EAGAIN);
    printf("OK\n");

    printf("Test 3: reserveSemNB should succeed again after release: ");
    assert(releaseSem(semId, 0) != -1);
    assert(reserveSemNB(semId, 0) == 0);
    printf("OK\n");

    assert(semctl(semId, 0, IPC_RMID) != -1);
    return 0;
}
