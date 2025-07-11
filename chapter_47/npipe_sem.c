#include <fcntl.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"
#include "npipe_sem.h"

#define SEM_TOKEN 'T'
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

int
sem_init(npipe_sem_t *sem, const char *fifo_path)
{
    if (mkfifo(fifo_path, SEM_PERMS) == -1 && errno != EEXIST) {
        perror("mkfifo");
        return -1;
    }

    sem->path = fifo_path;

    // keep the writer open for life of the semaphore to avoid EOF issues
    sem->internal_fd = open(fifo_path, O_RDWR | O_NONBLOCK);
    if (sem->internal_fd == -1) {
        perror("open for init");
        return -1;
    }

    // attempt to write initial token
    if (write(sem->internal_fd, &((char){SEM_TOKEN}), 1) != 1) {
        // if write fails because token already present, it's OK
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("write token");
            return -1;
        }
    }

    return 0;
}


void
sem_reserve(npipe_sem_t *sem)
{
    int fd = open(sem->path, O_RDONLY);
    if (fd == -1) {
        perror("open for reserve");
        exit(EXIT_FAILURE);
    }

    char token;
    if (read(fd, &token, 1) != 1) {
        perror("read failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}


void
sem_release(npipe_sem_t *sem)
{
    int fd = open(sem->path, O_WRONLY);
    if (fd == -1) {
        perror("open for release");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &((char){SEM_TOKEN}), 1) != 1) {
        perror("write failed");
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);
}


int
sem_try_reserve(npipe_sem_t *sem)
{
    int fd = open(sem->path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open for try_reserve");
        return -1;
    }

    char token;
    ssize_t res = read(fd, &token, 1);
    close(fd);

    if (res == 1)
        return 0;
    if (res == -1 && errno == EAGAIN)
        return 1;
    if (res == 0)
        return 1; // treat EOF as unavailable
    perror("read error");
    return -1;
}


void
sem_destroy(npipe_sem_t *sem)
{
    if (sem->internal_fd != -1) {
        close(sem->internal_fd);
        sem->internal_fd = -1;
    }
}
