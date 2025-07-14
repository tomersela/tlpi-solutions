/* svshm_ls.c

   Display a list of all System V shared-memory instances on the system.

   This program is Linux-specific.
*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "tlpi_hdr.h"


int
main(void)
{
    int maxind, ind, shmid;
    struct shmid_ds ds;
    struct shm_info shm_info;

    /* Get system-wide shared memory info */
    maxind = shmctl(0, SHM_INFO, (struct shmid_ds *) &shm_info);
    if (maxind == -1) {
        perror("shmctl-SHM_INFO");
        exit(EXIT_FAILURE);
    }

    printf("maxind: %d\n\n", maxind);
    printf("index     id       key      size\n");

    for (ind = 0; ind <= maxind; ind++) {
        shmid = shmctl(ind, SHM_STAT, &ds);
        if (shmid == -1) {
            if (errno != EINVAL && errno != EACCES)
                perror("shmctl-SHM_STAT");
            continue;
        }

        printf("%5d %8d  0x%08lx %7ld\n", ind, shmid,
               (unsigned long) ds.shm_perm.__key, (long) ds.shm_segsz);
    }

    exit(EXIT_SUCCESS);
}
