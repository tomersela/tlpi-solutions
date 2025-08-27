/* lock_tester.c

   Program to test fcntl() lock checking performance.
   Attempts to lock a specific byte N*2 that should already be locked
   by lock_acquirer, repeating the attempt many times to measure timing.

   Usage: ./lock_tester filename N iterations
   Where:
   - filename: same file used by lock_acquirer
   - N: the position multiplier (tests locking byte N*2)
   - iterations: number of lock attempts to make (default 10000)
*/

#include <sys/file.h>
#include <fcntl.h>
#ifdef __APPLE__
#include <sys/types.h>
#include <unistd.h>
#endif
#include "curr_time.h"
#include "tlpi_hdr.h"

int
main(int argc, char *argv[])
{
    int fd;
    struct flock fl;
    int n, iterations = 10000;
    int byte_pos;
    int attempts = 0, failures = 0;
    
    if (argc < 3 || strcmp(argv[1], "--help") == 0)
        usageErr("%s filename N [iterations]\n"
                 "  filename: file with existing locks\n"
                 "  N: position multiplier (tests byte N*2)\n"
                 "  iterations: number of attempts (default 10000)\n", argv[0]);

    n = getInt(argv[2], GN_NONNEG, "N");
    if (argc > 3)
        iterations = getInt(argv[3], GN_GT_0, "iterations");

    byte_pos = n * 2;

    printf("Testing lock attempts on byte %d (%d iterations)\n", byte_pos, iterations);
    printf("Starting test at %s\n", currTime("%T"));

    // open file
    fd = open(argv[1], O_RDWR);
    if (fd == -1)
        errExit("open %s", argv[1]);

    // set up lock structure
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = byte_pos;
    fl.l_len = 1;  // lock single byte

    // perform lock attempts in a tight loop
    for (int i = 0; i < iterations; i++) {
        attempts++;
        
        if (fcntl(fd, F_SETLK, &fl) == -1) {
            if (errno == EACCES || errno == EAGAIN) {
                failures++;  // expected - byte should be locked
            } else {
                errExit("fcntl F_SETLK");
            }
        } else {
            // unexpected success - unlock immediately
            fl.l_type = F_UNLCK;
            fcntl(fd, F_SETLK, &fl);
            fl.l_type = F_WRLCK;  // reset for next attempt
        }
    }

    printf("Completed test at %s\n", currTime("%T"));
    printf("Attempts: %d, Failures (expected): %d, Success: %d\n", 
           attempts, failures, attempts - failures);

    close(fd);
    return 0;
}
