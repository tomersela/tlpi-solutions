#include <assert.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"
#include "npipe_sem.h"

#define SEM_PATH "/tmp/npipe_sync_test"
#define N_PROCS 5


int
main(int argc, char* argv[])
{
    // disable buffering on stdout to prevent interleaved output
    setvbuf(stdout, NULL, _IONBF, 0);

    npipe_sem_t sem;
    assert(sem_init(&sem, SEM_PATH) == 0);

    for (int i = 0; i < N_PROCS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            npipe_sem_t child_sem = sem;
            sem_reserve(&child_sem);

            printf("[%d] BEGIN\n", getpid());
            sleep(1);  // simulate critical section
            printf("[%d] END\n", getpid());

            sem_release(&child_sem);
            _exit(EXIT_SUCCESS);
        }
    }

    for (int i = 0; i < N_PROCS; i++) {
        wait(NULL);
    }

    sem_destroy(&sem);
    unlink(SEM_PATH);

    printf("done\n");
    return 0;
}
