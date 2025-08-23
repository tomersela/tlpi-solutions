/*************************************************************************\
*                  POSIX Semaphore Implementation Header                 *
*                  using System V semaphores                            *
\*************************************************************************/

#ifndef POSIX_SEM_H
#define POSIX_SEM_H

#include <time.h>

/* POSIX semaphore type - opaque to users */
typedef void sem_t;

/* POSIX semaphore constants */
#define SEM_FAILED ((sem_t *) -1)  /* Error return value */


/* Function declarations for POSIX semaphore API */
sem_t *sem_open(const char *name, int oflag, ...);
int sem_wait(sem_t *sem);
int sem_trywait(sem_t *sem);
int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
int sem_post(sem_t *sem);
int sem_close(sem_t *sem);
int sem_unlink(const char *name);
int sem_getvalue(sem_t *sem, int *sval);

#endif /* POSIX_SEM_H */
