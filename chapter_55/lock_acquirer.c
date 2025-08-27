/* lock_acquirer.c

   Program to acquire 40,001 write locks on alternating bytes of a file.
   Locks are placed on bytes 0, 2, 4, 6, ... up to byte 80,000.
   After acquiring locks, the process sleeps to allow testing.

   Usage: ./lock_acquirer filename
*/

#include <sys/file.h>
#include <fcntl.h>
#ifdef __APPLE__
#include <sys/types.h>
#include <unistd.h>
#endif
#include "curr_time.h"
#include "tlpi_hdr.h"

#define NUM_LOCKS 40001
#define MAX_BYTE 80000


int
main(int argc, char *argv[])
{
    int fd;
    struct flock fl;
    int lock_count = 0;
    
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename\n", argv[0]);

    printf("Starting lock acquisition at %s\n", currTime("%T"));
    printf("Acquiring %d locks on alternating bytes (0, 2, 4, ... %d)\n", 
           NUM_LOCKS, MAX_BYTE);

    // open file for writing, create if doesn't exist
    fd = open(argv[1], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open %s", argv[1]);

    // ensure file is large enough
    if (lseek(fd, MAX_BYTE, SEEK_SET) == -1)
        errExit("lseek");
    if (write(fd, "x", 1) == -1)
        errExit("write");

    // acquire locks on alternating bytes
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_len = 1;  // lock single byte

    for (int byte_pos = 0; byte_pos <= MAX_BYTE; byte_pos += 2) {
        fl.l_start = byte_pos;
        
        if (fcntl(fd, F_SETLK, &fl) == -1) {
            if (errno == EACCES || errno == EAGAIN) {
                printf("Lock conflict at byte %d (unexpected)\n", byte_pos);
                continue;
            } else {
                errExit("fcntl F_SETLK at byte %d", byte_pos);
            }
        }
        
        lock_count++;
        
        // progress indicator
        if (lock_count % 5000 == 0) {
            printf("Acquired %d locks so far... (current byte: %d)\n", 
                   lock_count, byte_pos);
        }
    }

    printf("Successfully acquired %d locks at %s\n", lock_count, currTime("%T"));
    printf("Process PID: %ld\n", (long) getpid());
    printf("Sleeping indefinitely... Press Ctrl+C to exit\n");

    // sleep indefinitely to keep locks active
    while (1) {
        sleep(3600);  // sleep for 1 hour at a time
    }

    // this won't be reached, but good practice
    close(fd);
    return 0;
}
