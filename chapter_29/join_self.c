#include <pthread.h>

#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int error;
    if ((error = pthread_join(pthread_self(), NULL)) != 0)
        errExitEN(error, "pthread_join");
    
    exit(EXIT_SUCCESS);
}
