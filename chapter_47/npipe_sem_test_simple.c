#include <assert.h>

#include "tlpi_hdr.h"
#include "npipe_sem.h"

#define TEST_PATH "/tmp/npipe_sem_test"


int
main(int argc, char* argv[])
{
    npipe_sem_t sem;

    printf("[TEST] sem_init: ");
    assert(sem_init(&sem, TEST_PATH) == 0);
    printf("OK\n");

    printf("[TEST] sem_try_reserve (should succeed): ");
    assert(sem_try_reserve(&sem) == 0);
    printf("OK\n");

    printf("[TEST] sem_try_reserve (should fail): ");
    assert(sem_try_reserve(&sem) == 1);
    printf("OK\n");

    printf("[TEST] sem_release: ");
    sem_release(&sem);
    printf("OK\n");

    printf("[TEST] sem_reserve (blocking): ");
    sem_reserve(&sem);
    printf("OK\n");

    sem_destroy(&sem);
    unlink(TEST_PATH);
    return 0;
}
