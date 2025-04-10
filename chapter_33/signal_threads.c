#include <pthread.h>
#include <signal.h>

#include "tlpi_hdr.h"
#include "signal_functions.h"


static int hold = 1;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void *
thread_func(void *arg)
{
    while (hold) {
        pthread_cond_wait(&cond, &mtx);
    }
    
    printPendingSigs(stdout, "Pending signals:\n");

    return NULL;
}


int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int res;

    sigset_t sigset;
    sigfillset(&sigset);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    if ((res = pthread_create(&t1, NULL, thread_func, NULL)) != 0) {
        errExitEN(res, "pthread_create");
    }
    
    if ((res = pthread_create(&t2, NULL, thread_func, NULL)) != 0) {
        errExitEN(res, "pthread_create");
    }

    pthread_kill(t1, SIGUSR1);
    pthread_kill(t2, SIGUSR2);


    hold = 0;
    pthread_cond_broadcast(&cond);

    
    if ((res = pthread_join(t1, NULL)) != 0)
        errExitEN(res, "pthread_join");
    
    if ((res = pthread_join(t2, NULL)) != 0)
        errExitEN(res, "pthread_join");

    exit(EXIT_SUCCESS);
}
