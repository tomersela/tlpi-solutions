#define _GNU_SOURCE

#include <sys/wait.h>
#include <unistd.h>
#include <assert.h>

#include "tlpi_hdr.h"


static int fd_to_pid[1024];

FILE *mypopen(const char *command, const char *mode);
int mypclose(FILE *stream);

static void
restore_signals()
{
    // needed to 
    struct sigaction sa;
    sigaction(SIGINT, NULL, &sa);
    if (sa.sa_handler == SIG_IGN)
        signal(SIGINT, SIG_DFL);

    sigaction(SIGQUIT, NULL, &sa);
    if (sa.sa_handler == SIG_IGN)
        signal(SIGQUIT, SIG_DFL);
}


FILE *
mypopen(const char *command, const char *mode)
{
    FILE *file;
    int pipefd[2];
    int child_end, parent_end;
    int dup_target;
    pid_t pid;

    if (pipe(pipefd) == -1)
        errExit("pipe");
    
    if (strcmp(mode, "r") == 0) {
        child_end = pipefd[1];
        parent_end = pipefd[0];
        dup_target = STDOUT_FILENO;
        
    } else if (strcmp(mode, "w") == 0) {
        child_end = pipefd[0];
        parent_end = pipefd[1];
        dup_target = STDIN_FILENO;
        
    } else {
        close(pipefd[0]);
        close(pipefd[1]);
        errno = EINVAL;
        return NULL;
    }

    if ((file = fdopen(parent_end, mode)) == NULL) {
        close(pipefd[0]);
        close(pipefd[1]);
        errExit("fdopen");
    }

    switch (pid = fork()) {
        case -1:
            errExit("fork");

        case 0: // child
            restore_signals();
            close(parent_end);
            dup2(child_end, dup_target);
            close(child_end);
            
            execl("/bin/sh", "sh", "-c", command, (char *) NULL); // exec command via sh
            
            // if we got here, execution failed
            _exit(127); // using _exit() instead of exit() to avoid flushing stdio buffers inherited from the parent

        default: // parent
            close(child_end);
            fd_to_pid[fileno(file)] = pid; // save pipefd -> process id
            break;
    }

    return file;
}


int
mypclose(FILE *stream)
{
    int status;
    int fd = fileno(stream);
    int pid = fd_to_pid[fd];

    if (pid <= 0) {
        errno = ECHILD;
        return -1;
    }


    if (fclose(stream) == EOF)
        return -1;

    // wait for the child process
    while (waitpid(pid, &status, 0) == -1) {
        // automatically restart waitpid() if that call was interrupted by a signal handler
        if (errno != EINTR)
            return -1;
    }

    fd_to_pid[fd] = 0;

    return status;
}


static void
test_read()
{
    FILE *fp;
    char path[1035];
    int status;

    // prevent SIGPIPE from terminating the process if child exits early
    signal(SIGPIPE, SIG_IGN);

    if ((fp = mypopen("ls -l", "r")) == NULL) {
        errExit("mypopen");
    }

    // read the output a line at a time and print to stdout
    while (fgets(path, sizeof(path), fp) != NULL) {
        printf("%s", path);
    }

    // close the pipe
    if ((status = mypclose(fp)) == -1)
        errExit("mypclose");
        
    printf("\nCommand exited with status: %d\n", status);
}


static void
test_exec_failure()
{
    FILE *fp;
    int status;

    // invalid command
    if ((fp = mypopen("nonexistent_command_xyz123", "r")) == NULL) {
        errExit("mypopen");
    }

    // should produce no output (standard output. errors are expected)
    char buf[256];
    assert(fgets(buf, sizeof(buf), fp) == NULL);

    status = mypclose(fp);
    assert(status != -1);
    assert(WIFEXITED(status)); // child should have exited normally
    assert(WEXITSTATUS(status) == 127);
    
    printf("test_exec_failure passed\n");
}


static void
test_write()
{
    int status;
    FILE *fp;

    // prevent SIGPIPE from terminating the process if child exits early
    signal(SIGPIPE, SIG_IGN);

    // in order to see the output, we'll redirect to the terminal using /dev/tty
    if ((fp = mypopen("sort > /dev/tty", "w")) == NULL)
        errExit("mypopen");

    // write unsorted lines to the sort command
    fprintf(fp, "banana\n");
    fprintf(fp, "apple\n");
    fprintf(fp, "cherry\n");

    // close the pipe
    if ((status = mypclose(fp)) == -1)
        errExit("mypclose");
        
    printf("Command exited with status: %d\n", status);
}


int
main(int argc, char *argv[])
{
    printf("TEST read:\n");
    test_read();
    
    printf("TEST write:\n");
    test_write();

    printf("TEST exec failure:\n");
    test_exec_failure();

    exit(EXIT_SUCCESS);
}
