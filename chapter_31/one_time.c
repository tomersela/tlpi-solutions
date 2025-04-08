#include <pthread.h>

#include "tlpi_hdr.h"


typedef struct {
    Boolean initialized;
    pthread_mutex_t mtx;
} OneTimeControl;


#define ONE_TIME_INITIALIZER { FALSE, PTHREAD_MUTEX_INITIALIZER }


static OneTimeControl one_time = ONE_TIME_INITIALIZER;

void one_time_init(OneTimeControl *control, void (*init)(void));

void
one_time_init(OneTimeControl *control, void (*init)(void))
{
    int res;
    if ((res = pthread_mutex_lock(&control->mtx)) == -1)
        errExitEN(res, "pthread_mutex_lock");

    if (!control->initialized) {
        (*init)();
        control->initialized = true;
    }

    if ((res = pthread_mutex_unlock(&control->mtx)) == -1)
        errExitEN(res, "pthread_mutex_unlock");
}


static void
run_me_once(void)
{
    printf("One time!\n");
}


static void *
thread_func(void *arg)
{
    printf("thread_func called\n");
    one_time_init(&one_time, run_me_once);
    one_time_init(&one_time, run_me_once);
    return NULL;
}

int
main(int argc, char *argv[])
{
    pthread_t t1, t2;
    int s;

    s = pthread_create(&t1, NULL, thread_func, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create");

    s = pthread_create(&t2, NULL, thread_func, NULL);
    if (s != 0)
        errExitEN(s, "pthread_create");

    s = pthread_join(t1, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");

    s = pthread_join(t2, NULL);
    if (s != 0)
        errExitEN(s, "pthread_join");

    exit(EXIT_SUCCESS);
}
