#ifndef NPIPE_SEM_H
#define NPIPE_SEM_H

typedef struct {
    const char *path;
    int internal_fd;  // for keeping O_RDWR writer open
} npipe_sem_t;

int sem_init(npipe_sem_t *sem, const char *fifo_path);
void sem_reserve(npipe_sem_t *sem);
void sem_release(npipe_sem_t *sem);
int  sem_try_reserve(npipe_sem_t *sem);
void sem_destroy(npipe_sem_t *sem);

#endif
