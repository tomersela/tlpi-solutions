#include <sys/types.h>
#include <sys/sem.h>

#include "tlpi_hdr.h"
#include "semun.h"                      /* Definition of semun union */
#include "vms_flags.h"

int
setEventFlag(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = IPC_NOWAIT; // We don't want to block in case the flag is already set

    int res = semop(semId, &sops, 1);

    if (res == -1 && errno == EAGAIN) {
        // Already set
        return 0;
    }

    return res;
}


int
clearEventFlag(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}


int
waitForEventFlag(int semId, int semNum)
{
    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 0; // wait for semaphore to become 0
    sops.sem_flg = 0;

    return semop(semId, &sops, 1);
}


int
getFlagState(int semId, int semNum)
{

    struct sembuf sops;

    sops.sem_num = semNum;
    sops.sem_op = 0; // check if semaphore is equal to 0
    sops.sem_flg = IPC_NOWAIT; // but don't wait

    if (semop(semId, &sops, 1) == -1) {
        if (errno == EAGAIN) {
            // semaphore is not (yet) zero -> flag is not set
            return 0;
        }

        return -1; // unable to determine flag status
    }

    // flag is set
    return 1;
}
