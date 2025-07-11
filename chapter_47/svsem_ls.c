/* svsem_ls.c

   display a list of all System V semaphores on the system.

   this program is Linux-specific.
*/
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/sem.h>

#include "tlpi_hdr.h"
#include "semun.h"

int
main(int argc, char *argv[])
{
    int maxind, ind, semid;
    union semun arg;
    struct semid_ds ds;
    struct seminfo info;

    /* obtain size of kernel 'entries' array */

    arg.__buf = &info;
    maxind = semctl(0, 0, SEM_INFO, arg);
    if (maxind == -1)
        errExit("semctl-SEM_INFO");

    printf("maxind: %d\n\n", maxind);
    printf("index     id       key      nsems\n");

    /* retrieve and display information from each element of 'entries' array */

    for (ind = 0; ind <= maxind; ind++) {
        arg.buf = &ds;
        semid = semctl(ind, 0, SEM_STAT, arg);
        if (semid == -1) {
            if (errno != EINVAL && errno != EACCES)
                errMsg("semctl-SEM_STAT");  /* unexpected error */
            continue;                       /* ignore this item */
        }

        printf("%4d %8d  0x%08lx %7ld\n",
               ind, semid,
               (unsigned long) ds.sem_perm.__key,
               (long) ds.sem_nsems);
    }

    exit(EXIT_SUCCESS);
}
