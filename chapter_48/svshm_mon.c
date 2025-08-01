#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/shm.h>
#include <time.h>
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    struct shmid_ds ds;
    int shmid;

    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s <shmid>\n", argv[0]);

    shmid = getInt(argv[1], 0, "shmid");

    if (shmctl(shmid, IPC_STAT, &ds) == -1)
        errExit("shmctl-IPC_STAT");

    /* Permissions */
    printf("Owner UID / GID          %ld / %ld\n",
           (long) ds.shm_perm.uid, (long) ds.shm_perm.gid);
    printf("Creator UID / GID        %ld / %ld\n",
           (long) ds.shm_perm.cuid, (long) ds.shm_perm.cgid);
    printf("Mode (octal)             %o\n", ds.shm_perm.mode & 0777);

#ifdef SHM_DEST
    printf("SHM_DEST flag            %s\n",
           (ds.shm_perm.mode & SHM_DEST) ? "on" : "off");
#endif
#ifdef SHM_LOCKED
    printf("SHM_LOCKED flag          %s\n",
           (ds.shm_perm.mode & SHM_LOCKED) ? "on" : "off");
#endif

    /* Core statistics */
    printf("Segment size             %zu bytes\n", ds.shm_segsz);

    printf("Last shmat():            %s", ctime(&ds.shm_atime));
    printf("Last shmdt():            %s", ctime(&ds.shm_dtime));
    printf("Last change:             %s", ctime(&ds.shm_ctime));

    printf("Creator PID              %ld\n", (long) ds.shm_cpid);
    printf("Last-op PID              %ld\n", (long) ds.shm_lpid);
    printf("Attached processes       %ld\n", (long) ds.shm_nattch);

    exit(EXIT_SUCCESS);
}
