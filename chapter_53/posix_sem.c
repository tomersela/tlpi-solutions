/* posix_sem.c

  Solution for Exercise 53.3
  
  Implementation of POSIX semaphores using System V semaphores.
  
  This implementation provides the POSIX semaphore API on top of System V
  semaphore primitives. Named semaphores are managed using a combination
  of a System V semaphore for the actual counting and a shared memory
  segment for metadata.
*/

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <time.h>
#include <limits.h>
#include <stdarg.h>
#include <signal.h>

#include "tlpi_hdr.h"
#include "semun.h"      // system V semaphore union definition
#include "posix_sem.h"  // our POSIX semaphore API header


/* Internal structure to represent a POSIX semaphore */
struct posix_sem {
    int semid;          // system V semaphore ID
    int shmid;          // shared memory ID for metadata
    char *name;         // semaphore name (for unlink)
    int ref_count;      // reference count for this process
};

/* Metadata stored in shared memory */
struct sem_metadata {
    int initialized;     // magic number to verify initialization
    int unlinked;        // set to 1 if sem_unlink() was called
    int ref_count;       // total reference count across all processes
    mode_t mode;         // permission bits
    char name[NAME_MAX]; // semaphore name
};

#define SEM_MAGIC 0x53454D00  // magic number for initialized semaphores
#define SEM_FAILED_INTERNAL ((struct posix_sem *) -1)

/* Dummy signal handler for SIGALRM - does nothing but allows signal to interrupt system calls */
static void
dummy_alarm_handler(int sig)
{
    // Do nothing - we just need the signal to interrupt semop()
    (void) sig;  // Suppress unused parameter warning
}


/* Generate System V IPC key from semaphore name */
static key_t
name_to_key(const char *name)
{
    key_t key;
    char pathname[256];
    int fd;
    
    // create a pathname based on the semaphore name
    // we'll use /tmp as the base directory and the semaphore name as filename
    snprintf(pathname, sizeof(pathname), "/tmp/posix_sem_%s", name);
    
    // ensure the file exists for ftok() - create it if it doesn't exist
    fd = open(pathname, O_CREAT | O_EXCL, 0644);
    if (fd != -1) {
        close(fd);
    } else if (errno != EEXIST) {
        // if we can't create the file and it's not because it exists,
        // fall back to using a well-known existing file
        strcpy(pathname, "/tmp");
    }
    
    // use ftok() to generate the key - this is the standard System V way
    // we use project ID based on first char of name for some uniqueness
    key = ftok(pathname, name[0] ? name[0] : 'P');
    
    return key;
}


sem_t *
sem_open(const char *name, int oflag, ...)
{
    struct posix_sem *sem;
    key_t key;
    int semid, shmid;
    struct sem_metadata *metadata;
    union semun arg;
    mode_t mode = S_IRUSR | S_IWUSR;
    unsigned int value = 0;
    va_list ap;
    
    if (name == NULL || name[0] != '/') {
        errno = EINVAL;
        return SEM_FAILED;
    }
    
    // skip leading slash and check name length
    name++;
    if (strlen(name) >= NAME_MAX) {
        errno = ENAMETOOLONG;
        return SEM_FAILED;
    }
    
    // get additional arguments if O_CREAT is specified
    if (oflag & O_CREAT) {
        va_start(ap, oflag);
        mode = (mode_t) va_arg(ap, int);
        value = va_arg(ap, unsigned int);
        va_end(ap);
        
        if (value > SEM_VALUE_MAX) {
            errno = EINVAL;
            return SEM_FAILED;
        }
    }
    
    // generate key from name
    key = name_to_key(name);
    
    // allocate semaphore structure
    sem = malloc(sizeof(struct posix_sem));
    if (sem == NULL) {
        errno = ENOMEM;
        return SEM_FAILED;
    }
    
    // try to get existing semaphore first
    semid = semget(key, 1, 0);
    shmid = shmget(key, sizeof(struct sem_metadata), 0);
    
    if (semid == -1 || shmid == -1) {
        // semaphore doesn't exist
        if (!(oflag & O_CREAT)) {
            free(sem);
            errno = ENOENT;
            return SEM_FAILED;
        }
        
        // create new semaphore
        semid = semget(key, 1, IPC_CREAT | IPC_EXCL | (mode & 0777));
        if (semid == -1) {
            if (errno == EEXIST) {
                // race condition: another process created it
                if (oflag & O_EXCL) {
                    free(sem);
                    errno = EEXIST;
                    return SEM_FAILED;
                }
                // try to get the existing one
                semid = semget(key, 1, 0);
                shmid = shmget(key, sizeof(struct sem_metadata), 0);
                if (semid == -1 || shmid == -1) {
                    free(sem);
                    return SEM_FAILED;
                }
                goto existing_sem;
            }
            free(sem);
            return SEM_FAILED;
        }
        
        // create shared memory for metadata
        shmid = shmget(key, sizeof(struct sem_metadata), 
                       IPC_CREAT | IPC_EXCL | (mode & 0777));
        if (shmid == -1) {
            semctl(semid, 0, IPC_RMID);  // clean up semaphore
            free(sem);
            return SEM_FAILED;
        }
        
        // initialize semaphore value
        arg.val = value;
        if (semctl(semid, 0, SETVAL, arg) == -1) {
            semctl(semid, 0, IPC_RMID);
            shmctl(shmid, IPC_RMID, NULL);
            free(sem);
            return SEM_FAILED;
        }
        
        // initialize metadata
        metadata = shmat(shmid, NULL, 0);
        if (metadata == (void *) -1) {
            semctl(semid, 0, IPC_RMID);
            shmctl(shmid, IPC_RMID, NULL);
            free(sem);
            return SEM_FAILED;
        }
        
        metadata->initialized = SEM_MAGIC;
        metadata->unlinked = 0;
        metadata->ref_count = 1;
        metadata->mode = mode;
        strncpy(metadata->name, name, NAME_MAX - 1);
        metadata->name[NAME_MAX - 1] = '\0';
        
        shmdt(metadata);
        
    } else {
        // existing semaphore
        existing_sem:
        if (oflag & O_EXCL) {
            free(sem);
            errno = EEXIST;
            return SEM_FAILED;
        }
        
        // attach to metadata and verify
        metadata = shmat(shmid, NULL, 0);
        if (metadata == (void *) -1) {
            free(sem);
            return SEM_FAILED;
        }
        
        if (metadata->initialized != SEM_MAGIC) {
            shmdt(metadata);
            free(sem);
            errno = EINVAL;
            return SEM_FAILED;
        }
        
        if (metadata->unlinked) {
            shmdt(metadata);
            free(sem);
            errno = ENOENT;
            return SEM_FAILED;
        }
        
        // increment reference count
        metadata->ref_count++;
        shmdt(metadata);
    }
    
    // initialize our semaphore structure
    sem->semid = semid;
    sem->shmid = shmid;
    sem->name = strdup(name);
    sem->ref_count = 1;
    
    if (sem->name == NULL) {
        free(sem);
        errno = ENOMEM;
        return SEM_FAILED;
    }
    
    return (sem_t *) sem;
}


int
sem_wait(sem_t *sem_ptr)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    struct sembuf sop;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL) {
        errno = EINVAL;
        return -1;
    }
    
    sop.sem_num = 0;
    sop.sem_op = -1;    // decrement
    sop.sem_flg = 0;    // block if necessary
    
    while (semop(sem->semid, &sop, 1) == -1) {
        if (errno != EINTR)
            return -1;
    }
    
    return 0;
}


int
sem_trywait(sem_t *sem_ptr)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    struct sembuf sop;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL) {
        errno = EINVAL;
        return -1;
    }
    
    sop.sem_num = 0;
    sop.sem_op = -1;        // decrement
    sop.sem_flg = IPC_NOWAIT;  // don't block
    
    if (semop(sem->semid, &sop, 1) == -1) {
        if (errno == EAGAIN)
            errno = EAGAIN;  // POSIX expects EAGAIN for try operations
        return -1;
    }
    
    return 0;
}


int
sem_timedwait(sem_t *sem_ptr, const struct timespec *abs_timeout)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    struct timespec current_time;
    struct sembuf sop;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL || abs_timeout == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    if (abs_timeout->tv_nsec < 0 || abs_timeout->tv_nsec >= 1000000000) {
        errno = EINVAL;
        return -1;
    }
    
    // get current time
    if (clock_gettime(CLOCK_REALTIME, &current_time) == -1)
        return -1;
    
    // check if timeout has already passed
    if (current_time.tv_sec > abs_timeout->tv_sec ||
        (current_time.tv_sec == abs_timeout->tv_sec && 
         current_time.tv_nsec >= abs_timeout->tv_nsec)) {
        errno = ETIMEDOUT;
        return -1;
    }
    
    // try non-blocking first
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = IPC_NOWAIT;
    
    if (semop(sem->semid, &sop, 1) == 0)
        return 0;  // success
    
    if (errno != EAGAIN)
        return -1;  // real error
    
    // need to wait with timeout - System V semaphores don't support timeouts natively
    // we use a signal-based approach for better efficiency
    
    struct timespec remaining_time;
    remaining_time.tv_sec = abs_timeout->tv_sec - current_time.tv_sec;
    remaining_time.tv_nsec = abs_timeout->tv_nsec - current_time.tv_nsec;
    
    if (remaining_time.tv_nsec < 0) {
        remaining_time.tv_sec--;
        remaining_time.tv_nsec += 1000000000;
    }
    
    // set up alarm for timeout
    struct sigaction old_action, new_action;
    sigset_t old_mask, new_mask;
    
    // block SIGALRM initially
    sigemptyset(&new_mask);
    sigaddset(&new_mask, SIGALRM);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    
    // set up dummy signal handler - we don't need to do anything in it
    // but we need a real handler (not SIG_IGN) for the signal to interrupt semop()
    new_action.sa_handler = dummy_alarm_handler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;
    
    if (sigaction(SIGALRM, &new_action, &old_action) == -1) {
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
        return -1;
    }
    
    // set alarm
    alarm(remaining_time.tv_sec + (remaining_time.tv_nsec > 0 ? 1 : 0));
    
    // unblock SIGALRM so it can interrupt semop()
    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    
    // try blocking semop - it will be interrupted by SIGALRM
    sop.sem_flg = 0;  // blocking operation
    int result = semop(sem->semid, &sop, 1);
    int saved_errno = errno;
    
    // clean up alarm and signal handler
    alarm(0);
    sigaction(SIGALRM, &old_action, NULL);
    // signal mask already restored above
    
    if (result == 0) {
        return 0;  // success
    }
    
    if (saved_errno == EINTR) {
        // interrupted by alarm - check if timeout expired
        if (clock_gettime(CLOCK_REALTIME, &current_time) == -1)
            return -1;
            
        if (current_time.tv_sec > abs_timeout->tv_sec ||
            (current_time.tv_sec == abs_timeout->tv_sec && 
             current_time.tv_nsec >= abs_timeout->tv_nsec)) {
            errno = ETIMEDOUT;
            return -1;
        }
        
        // interrupted but timeout not reached - try once more non-blocking
        sop.sem_flg = IPC_NOWAIT;
        if (semop(sem->semid, &sop, 1) == 0)
            return 0;
            
        if (errno == EAGAIN) {
            errno = ETIMEDOUT;  // close enough to timeout
        }
        return -1;
    }
    
    errno = saved_errno;
    return -1;
}

/* Implementation of sem_post() */
int
sem_post(sem_t *sem_ptr)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    struct sembuf sop;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL) {
        errno = EINVAL;
        return -1;
    }
    
    sop.sem_num = 0;
    sop.sem_op = 1;     // increment
    sop.sem_flg = 0;
    
    return semop(sem->semid, &sop, 1);
}


int
sem_close(sem_t *sem_ptr)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    struct sem_metadata *metadata;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL) {
        errno = EINVAL;
        return -1;
    }
    
    // decrement reference count in shared memory
    metadata = shmat(sem->shmid, NULL, 0);
    if (metadata != (void *) -1) {
        metadata->ref_count--;
        
        // if this was the last reference and semaphore was unlinked,
        // clean up resources
        if (metadata->ref_count == 0 && metadata->unlinked) {
            shmdt(metadata);
            semctl(sem->semid, 0, IPC_RMID);
            shmctl(sem->shmid, IPC_RMID, NULL);
        } else {
            shmdt(metadata);
        }
    }
    
    // free our local resources
    free(sem->name);
    free(sem);
    
    return 0;
}


int
sem_unlink(const char *name)
{
    key_t key;
    int semid, shmid;
    struct sem_metadata *metadata;
    
    if (name == NULL || name[0] != '/') {
        errno = EINVAL;
        return -1;
    }
    
    // skip leading slash
    name++;
    if (strlen(name) >= NAME_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }
    
    // generate key from name
    key = name_to_key(name);
    
    // try to get existing semaphore
    semid = semget(key, 1, 0);
    shmid = shmget(key, sizeof(struct sem_metadata), 0);
    
    if (semid == -1 || shmid == -1) {
        errno = ENOENT;
        return -1;
    }
    
    // mark as unlinked in metadata
    metadata = shmat(shmid, NULL, 0);
    if (metadata == (void *) -1)
        return -1;
        
    if (metadata->initialized != SEM_MAGIC) {
        shmdt(metadata);
        errno = ENOENT;
        return -1;
    }
    
    metadata->unlinked = 1;
    
    // if no processes have it open, clean up immediately
    if (metadata->ref_count == 0) {
        shmdt(metadata);
        semctl(semid, 0, IPC_RMID);
        shmctl(shmid, IPC_RMID, NULL);
    } else {
        shmdt(metadata);
    }
    
    return 0;
}


int
sem_getvalue(sem_t *sem_ptr, int *sval)
{
    struct posix_sem *sem = (struct posix_sem *) sem_ptr;
    
    if (sem == NULL || sem == SEM_FAILED_INTERNAL || sval == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    *sval = semctl(sem->semid, 0, GETVAL);
    if (*sval == -1)
        return -1;
        
    return 0;
}
