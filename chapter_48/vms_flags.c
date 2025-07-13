#include <sys/types.h>
#include <sys/sem.h>

#include "tlpi_hdr.h"
#include "semun.h"                      /* Definition of semun union */
#include "vms_flags.h"


/* Event-flag implementation (VMS semantics)
   0 == flag set (signalled)
   1 == flag clear (reset)
   waitForEventFlag() blocks until set, then auto-clears.
*/


int
setEventFlag(int semId, int semNum)
{
    struct sembuf sops;
    int ret;

    sops.sem_num = semNum;
    sops.sem_op  = -1; // try to drive value to 0 (set)
    sops.sem_flg = IPC_NOWAIT; // we don't want to block in case the flag is already set

    do {
        ret = semop(semId, &sops, 1);
    } while (ret == -1 && errno == EINTR);

    if (ret == -1 && errno == EAGAIN) // already set
        return 0;

    return ret;
}


int
clearEventFlag(int semId, int semNum)
{
    struct sembuf sops;
    int ret;

    sops.sem_num = semNum;
    sops.sem_op  = 1; // set value to 1 (clear)
    sops.sem_flg = 0;

    do {
        ret = semop(semId, &sops, 1);
    } while (ret == -1 && errno == EINTR);

    return ret;
}



int
waitForEventFlag(int semId, int semNum)
{
    struct sembuf sops[2];
    int ret;

    // wait until sem == 0 (flag is set)
    sops[0].sem_num = semNum;
    sops[0].sem_op  = 0;
    sops[0].sem_flg = 0;

    // put it back to 1 (clear the flag)
    sops[1].sem_num = semNum;
    sops[1].sem_op  = 1;
    sops[1].sem_flg = 0;

    do {
        ret = semop(semId, sops, 2);
    } while (ret == -1 && errno == EINTR);

    return ret;
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
