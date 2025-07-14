#include "tlpi_hdr.h"
#include "dir_service.h"

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>


#define MAX_ENTRIES 128


#define SHM_KEY 0x1234
#define SEM_KEY 0x5678


typedef struct {
    char name[NAME_LEN];
    char value[VALUE_LEN];
    int in_use;
} Entry;

typedef struct {
    Entry entries[MAX_ENTRIES];
} Directory;


static int semid = -1;
static struct sembuf sem_lock = {0, -1, SEM_UNDO};
static struct sembuf sem_unlock = {0, 1, SEM_UNDO};


static int
ensure_semaphore()
{
    if (semid != -1)
        return semid;

    semid = semget(SEM_KEY, 1, 0);
    if (semid == -1) {
        perror("semget");
    }
    return semid;
}


static Directory*
attach_directory(int* shmid_out)
{
    int shmid = shmget(SHM_KEY, sizeof(Directory), 0);
    if (shmid == -1) {
        perror("shmget");
        return NULL;
    }

    void* addr = shmat(shmid, NULL, 0);
    if (addr == (void*) -1) {
        perror("shmat");
        return NULL;
    }

    *shmid_out = shmid;
    return (Directory*) addr;
}


int
dir_init()
{
    int shmid = shmget(SHM_KEY, sizeof(Directory), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        if (errno != EEXIST) {
            perror("shmget");
            return -1;
        }
        // already exists, nothing to do

    } else {
        // new directory
        void* addr = shmat(shmid, NULL, 0);
        if (addr == (void*) -1) {
            perror("shmat");
            return -1;
        }

        memset(addr, 0, sizeof(Directory)); // initialize contents
        shmdt(addr);
    }

    // create semaphore
    int semid = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | (S_IRUSR | S_IWUSR));
    if (semid == -1) {
        if (errno != EEXIST) {
            perror("semget");
            return -1;
        }
        // already exists, nothing to do

    } else {
        // set initial semaphore value to 1
        if (semctl(semid, 0, SETVAL, 1) == -1) {
            perror("semctl - SETVAL");
            return -1;
        }
    }

    return 0;
}


void
dir_cleanup()
{
    int shmid = shmget(SHM_KEY, sizeof(Directory), (S_IRUSR | S_IWUSR));
    int semid = semget(SEM_KEY, 1, (S_IRUSR | S_IWUSR));
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);
}


int
dir_set(const char* name, const char* value)
{
    int shmid;
    Directory* dir = attach_directory(&shmid);
    if (!dir)
        return -1;

    int semid = ensure_semaphore();
    if (semid == -1) {
        shmdt(dir);
        return -1;
    }

    if (semop(semid, &sem_lock, 1) == -1) {
        perror("semop - lock");
        shmdt(dir);
        return -1;
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (dir->entries[i].in_use && strncmp(dir->entries[i].name, name, NAME_LEN) == 0) {
            strncpy(dir->entries[i].value, value, VALUE_LEN - 1);
            dir->entries[i].value[VALUE_LEN - 1] = '\0';
            semop(semid, &sem_unlock, 1);
            shmdt(dir);
            return 0; // updated
        }
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (!dir->entries[i].in_use) {
            strncpy(dir->entries[i].name, name, NAME_LEN - 1);
            dir->entries[i].name[NAME_LEN - 1] = '\0';
            strncpy(dir->entries[i].value, value, VALUE_LEN - 1);
            dir->entries[i].value[VALUE_LEN - 1] = '\0';
            dir->entries[i].in_use = 1;
            semop(semid, &sem_unlock, 1);
            shmdt(dir);
            return 1; // created
        }
    }

    semop(semid, &sem_unlock, 1);
    shmdt(dir);
    return -1; // no space
}


int
dir_get(const char* name, char* value_out)
{
    int shmid;
    Directory* dir = attach_directory(&shmid);
    if (!dir)
        return -1;

    int semid = ensure_semaphore();
    if (semid == -1) {
        shmdt(dir);
        return -1;
    }

    if (semop(semid, &sem_lock, 1) == -1) {
        perror("semop - lock");
        shmdt(dir);
        return -1;
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (dir->entries[i].in_use && strncmp(dir->entries[i].name, name, NAME_LEN) == 0) {
            strncpy(value_out, dir->entries[i].value, VALUE_LEN);
            value_out[VALUE_LEN] = '\0';
            semop(semid, &sem_unlock, 1);
            shmdt(dir);
            return 0;
        }
    }

    semop(semid, &sem_unlock, 1);
    shmdt(dir);
    return -1;
}


int
dir_delete(const char* name)
{
    int shmid;
    Directory* dir = attach_directory(&shmid);
    if (!dir)
        return -1;

    int semid = ensure_semaphore();
    if (semid == -1) {
        shmdt(dir);
        return -1;
    }

    if (semop(semid, &sem_lock, 1) == -1) {
        perror("semop - lock");
        shmdt(dir);
        return -1;
    }

    for (int i = 0; i < MAX_ENTRIES; i++) {
        if (dir->entries[i].in_use && strncmp(dir->entries[i].name, name, NAME_LEN) == 0) {
            dir->entries[i].in_use = 0;
            semop(semid, &sem_unlock, 1);
            shmdt(dir);
            return 0;
        }
    }

    semop(semid, &sem_unlock, 1);
    shmdt(dir);
    return -1;
}
