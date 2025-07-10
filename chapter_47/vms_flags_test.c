#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "tlpi_hdr.h"
#include "semun.h"
#include "vms_flags.h"

#define SEM_KEY 0x12345  // arbitrary key for the semaphore set
#define SEM_NUM 0        // use the first semaphore in the set

int main(void) {
    int semId;

    // create a set with 1 semaphore
    semId = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 0600);
    if (semId == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    // cnitialize it to 1 => flag is cleared
    union semun arg;
    arg.val = 1;
    if (semctl(semId, SEM_NUM, SETVAL, arg) == -1) {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }

    printf("Initialized semaphore to 1 (cleared)\n");

    printf("Test: getFlagState should return 0 (cleared): ");
    assert(getFlagState(semId, SEM_NUM) == 0);
    printf("OK\n");

    printf("Test: setEventFlag should succeed: ");
    assert(setEventFlag(semId, SEM_NUM) == 0);
    printf("OK\n");

    printf("Test: getFlagState should return 1 (set): ");
    assert(getFlagState(semId, SEM_NUM) == 1);
    printf("OK\n");

    printf("Test: waitForEventFlag should return 0 (not block): ");
    assert(waitForEventFlag(semId, SEM_NUM) == 0);
    printf("OK\n");

    printf("Test: clearEventFlag should succeed: ");
    assert(clearEventFlag(semId, SEM_NUM) == 0);
    printf("OK\n");

    printf("Test: getFlagState should return 0 (cleared again): ");
    assert(getFlagState(semId, SEM_NUM) == 0);
    printf("OK\n");

    // Clean up the semaphore
    if (semctl(semId, 0, IPC_RMID) == -1) {
        perror("semctl IPC_RMID");
        exit(EXIT_FAILURE);
    }

    printf("Test completed successfully.\n");

    return 0;
}
