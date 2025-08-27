#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <signal.h>
#include <getopt.h>

#include "tlpi_hdr.h"


static char **created_files = NULL;
static int num_created = 0;
static int max_files = 0;


// cleanup function for signal handling
static void
cleanup_and_exit(int sig)
{
    for (int i = 0; i < num_created; i++) {
        unlink(created_files[i]);
    }
    exit(EXIT_FAILURE);
}


// add a file to the cleanup list
static void
add_created_file(const char *filename)
{
    if (num_created >= max_files) {
        max_files = max_files ? max_files * 2 : 16;
        created_files = realloc(created_files, max_files * sizeof(char*));
        if (!created_files)
            errExit("realloc");
    }
    created_files[num_created] = strdup(filename);
    if (!created_files[num_created])
        errExit("strdup");
    num_created++;
}


// check if lockfile is stale (older than timeout)
static int
is_stale_lockfile(const char *filename, int timeout)
{
    struct stat st;
    time_t now;

    if (stat(filename, &st) == -1)
        return 0; // file doesn't exist, not stale

    time(&now);
    return (now - st.st_mtime) > timeout;
}


// try to create a single lockfile atomically
// returns: 0 = success, 1 = already exists, -1 = other error
static int
try_create_lockfile(const char *filename)
{
    int fd;

    // try to create file exclusively (atomic operation)
    fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        if (errno == EEXIST)
            return 1; // lockfile already exists
        else
            return -1; // other error
    }

    // write our PID to the lockfile
    dprintf(fd, "%ld\n", (long) getpid());
    close(fd);
    return 0; // success
}


// usage information
static void
usage(const char *progname)
{
    fprintf(stderr, "Usage: %s [options] filename...\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -s sleeptime    Sleep time between retries (default: 8)\n");
    fprintf(stderr, "  -r retries      Number of retries (-1 = forever, default: -1)\n");
    fprintf(stderr, "  -l timeout      Lock timeout in seconds (force removal)\n");
    fprintf(stderr, "  -!              Invert exit status\n");
    fprintf(stderr, "  -h              Show this help\n");
}


int
main(int argc, char *argv[])
{
    int sleeptime = 8;
    int retries = -1;
    int locktimeout = 0;
    int invert_status = 0;
    int opt;
    int success = 1;

    // install signal handlers for cleanup
    signal(SIGINT, cleanup_and_exit);
    signal(SIGTERM, cleanup_and_exit);

    // parse command line options
    while ((opt = getopt(argc, argv, "s:r:l:!h")) != -1) {
        switch (opt) {
            case 's':
                sleeptime = atoi(optarg);
                if (sleeptime < 0) sleeptime = 8;
                break;
            case 'r':
                retries = atoi(optarg);
                break;
            case 'l':
                locktimeout = atoi(optarg);
                if (locktimeout < 0) locktimeout = 0;
                break;
            case '!':
                invert_status = !invert_status;
                break;
            case 'h':
                usage(argv[0]);
                exit(EXIT_SUCCESS);
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: No filenames specified\n");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    // try to create all specified lockfiles
    for (int i = optind; i < argc; i++) {
        const char *filename = argv[i];
        int attempts = 0;
        int created = 0;


        // retry logic
        while (!created && (retries == -1 || attempts < retries)) {
            int result = try_create_lockfile(filename);

            if (result == 0) {
                // successfully created
                add_created_file(filename);
                created = 1;
                break;
            } else if (result == -1) {
                // error other than EEXIST
                fprintf(stderr, "Error creating lockfile '%s': %s\n",
                        filename, strerror(errno));
                success = 0;
                break;
            }

            // result == 1: lockfile already exists, continue -  check if it's stale and retry again

            if (locktimeout > 0 && is_stale_lockfile(filename, locktimeout)) {
                printf("Forcing lock on \"%s\"\n", filename);
                if (unlink(filename) == -1) {
                    if (errno == EACCES || errno == EPERM) {
                        fprintf(stderr, "Forced unlock denied on \"%s\"\n", filename);
                        success = 0;
                        break;
                    }
                }
                continue; // try again after removing stale lock
            }

            attempts++;
            if (retries != -1 && attempts >= retries) {
                fprintf(stderr, "Sorry, giving up on \"%s\"\n", filename);
                success = 0;
                break;
            }

            sleep(sleeptime);
        }

        if (!created) {
            success = 0;
            break;
        }
    }

    // if we failed, clean up all created files
    if (!success) {
        for (int i = 0; i < num_created; i++) {
            unlink(created_files[i]);
        }
    }

    // cleanup memory
    for (int i = 0; i < num_created; i++) {
        free(created_files[i]);
    }
    free(created_files);

    int exit_status = success ? EXIT_SUCCESS : EXIT_FAILURE;
    return invert_status ? !exit_status : exit_status;
}
