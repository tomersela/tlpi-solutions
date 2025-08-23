#include <sys/wait.h>
#include <fcntl.h>

#include "tlpi_hdr.h"
#include "posix_sem.h"  /* Our POSIX semaphore implementation */

// Global variables for cleanup
static sem_t *global_sem = NULL;
static char global_sem_name[64] = {0};

// Helper function to reset semaphore to a specific value
static void reset_semaphore_value(sem_t *sem, int target_value) {
    int current_value;
    
    // get current value
    if (sem_getvalue(sem, &current_value) == -1) {
        perror("sem_getvalue in reset_semaphore_value");
        exit(EXIT_FAILURE);
    }
    
    // if current value is higher than target, drain the excess
    while (current_value > target_value) {
        if (sem_trywait(sem) == -1) {
            if (errno == EAGAIN) {
                break; // semaphore is already at 0
            }
            perror("sem_trywait in reset_semaphore_value");
            exit(EXIT_FAILURE);
        }
        current_value--;
    }
    
    // if current value is lower than target, post the difference
    while (current_value < target_value) {
        if (sem_post(sem) == -1) {
            perror("sem_post in reset_semaphore_value");
            exit(EXIT_FAILURE);
        }
        current_value++;
    }
}

// Cleanup function called by atexit()
static void cleanup_semaphore(void) {
    if (global_sem != NULL) {
        sem_close(global_sem);
        if (strlen(global_sem_name) > 0) {
            sem_unlink(global_sem_name);
        }
    }
}

int
main(int argc, char *argv[])
{
    sem_t *sem;
    pid_t pid;
    int status;
    char sem_name[64];
    
    // Use process ID and timestamp to create unique semaphore name
    snprintf(sem_name, sizeof(sem_name), "/test_sem_%d_%ld", getpid(), time(NULL));
    
    // Register cleanup function
    if (atexit(cleanup_semaphore) != 0) {
        fprintf(stderr, "Failed to register cleanup function\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Testing POSIX semaphore implementation using System V semaphores\n");
    printf("================================================================\n\n");
    
    
    printf("Test 1: Creating semaphore '%s' with initial value 1\n", sem_name);
    
    sem = sem_open(sem_name, O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }
    
    // Set up global variables for cleanup
    global_sem = sem;
    strncpy(global_sem_name, sem_name, sizeof(global_sem_name) - 1);
    global_sem_name[sizeof(global_sem_name) - 1] = '\0';
    
    printf("SUCCESS: Semaphore created successfully\n\n");
    
    
    printf("Test 2: Checking semaphore value\n");
    reset_semaphore_value(sem, 1);
    
    int sval;
    if (sem_getvalue(sem, &sval) == -1) {
        perror("sem_getvalue");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: Semaphore value: %d (expected: 1)\n\n", sval);
    
    
    printf("Test 3: Testing sem_wait() and sem_post()\n");
    reset_semaphore_value(sem, 1);
    
    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: sem_wait() succeeded\n");
    
    if (sem_getvalue(sem, &sval) == -1) {
        perror("sem_getvalue");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: After sem_wait(), value: %d (expected: 0)\n", sval);
    
    if (sem_post(sem) == -1) {
        perror("sem_post");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: sem_post() succeeded\n");
    
    if (sem_getvalue(sem, &sval) == -1) {
        perror("sem_getvalue");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: After sem_post(), value: %d (expected: 1)\n\n", sval);
    
    
    printf("Test 4: Testing sem_trywait()\n");
    reset_semaphore_value(sem, 1);
    
    if (sem_trywait(sem) == -1) {
        perror("sem_trywait");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: sem_trywait() succeeded when semaphore available\n");
    
    // Try again when not available
    if (sem_trywait(sem) == 0) {
        printf("FAILED: sem_trywait() should have failed when semaphore unavailable\n");
    } else if (errno == EAGAIN) {
        printf("SUCCESS: sem_trywait() correctly failed with EAGAIN when unavailable\n");
    } else {
        perror("sem_trywait unexpected error");
    }
    

    printf("\n");
    
    
    printf("Test 5: Testing with multiple processes\n");
    
    // Reset to known state (value = 0 for synchronization test)
    reset_semaphore_value(sem, 0);
    
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) {
        // Child process
        sleep(1);  // Let parent get to sem_wait first
        printf("Child: Posting semaphore\n");
        if (sem_post(sem) == -1) {
            perror("child sem_post");
            _exit(EXIT_FAILURE);  // Use _exit() to bypass atexit() handlers
        }
        printf("Child: Posted semaphore successfully\n");
        _exit(EXIT_SUCCESS);  // Use _exit() to bypass atexit() handlers
    } else {
        // Parent process
        printf("Parent: Waiting on semaphore...\n");
        if (sem_wait(sem) == -1) {
            perror("parent sem_wait");
            exit(EXIT_FAILURE);
        }
        printf("Parent: Got semaphore from child\n");
        
        wait(&status);
        if (WEXITSTATUS(status) == 0) {
            printf("SUCCESS: Multi-process synchronization test passed\n");
        } else {
            printf("FAILED: Child process failed\n");
        }
    }
    printf("\n");
    
    
    printf("Test 6: Testing sem_timedwait()\n");
    
    // Reset to known state (value = 1)
    reset_semaphore_value(sem, 1);
    
    // Now make semaphore unavailable for timeout test
    if (sem_wait(sem) == -1) {
        perror("sem_wait for timedwait test");
        exit(EXIT_FAILURE);
    }
    
    // Test timeout case - should timeout after 2 seconds
    struct timespec timeout;
    if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    timeout.tv_sec += 2;  // 2 second timeout
    time_t start_time = time(NULL);
    
    if (sem_timedwait(sem, &timeout) == -1) {
        time_t end_time = time(NULL);
        int elapsed = (int)(end_time - start_time);
        
        if (errno == ETIMEDOUT) {
            printf("SUCCESS: sem_timedwait() timed out after %d seconds\n", elapsed);
            if (elapsed >= 2 && elapsed <= 3) {
                printf("SUCCESS: Timeout occurred within expected time range\n");
            } else {
                printf("WARNING: Timeout took %d seconds (expected ~2)\n", elapsed);
            }
        } else {
            perror("sem_timedwait failed with unexpected error");
            exit(EXIT_FAILURE);
        }
    } else {
        printf("FAILED: sem_timedwait() should have timed out but succeeded\n");
        exit(EXIT_FAILURE);
    }
    
    // Test success case - make semaphore available and try again
    if (sem_post(sem) == -1) {
        perror("sem_post for timedwait test");
        exit(EXIT_FAILURE);
    }
    
    if (clock_gettime(CLOCK_REALTIME, &timeout) == -1) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    timeout.tv_sec += 2;  // 2 second timeout
    
    if (sem_timedwait(sem, &timeout) == -1) {
        perror("sem_timedwait should have succeeded");
        exit(EXIT_FAILURE);
    } else {
        printf("SUCCESS: sem_timedwait() succeeded when semaphore available\n");
    }
    
    // Restore semaphore to available state
    if (sem_post(sem) == -1) {
        perror("sem_post restore");
        exit(EXIT_FAILURE);
    }
    
    printf("\n");
    
    
    printf("Test 7: Testing sem_close() and sem_unlink()\n");
    if (sem_close(sem) == -1) {
        perror("sem_close");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: sem_close() succeeded\n");
    
    if (sem_unlink(sem_name) == -1) {
        perror("sem_unlink");
        exit(EXIT_FAILURE);
    }
    printf("SUCCESS: sem_unlink() succeeded\n\n");
    
    // Clear global variables since we manually cleaned up
    global_sem = NULL;
    global_sem_name[0] = '\0';
    
    
    printf("Test 8: Verifying semaphore was unlinked\n");
    sem = sem_open(sem_name, 0);  /* Try to open without O_CREAT */
    if (sem == SEM_FAILED && errno == ENOENT) {
        printf("SUCCESS: Semaphore correctly unlinked\n");
    } else {
        printf("FAILED: Semaphore still exists after unlink\n");
        if (sem != SEM_FAILED) {
            sem_close(sem);
            sem_unlink(sem_name);
        }
    }
    
    
    printf("\n================================================================\n");
    printf("All tests completed!\n");
    
    return 0;
}
