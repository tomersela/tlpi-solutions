#define _XOPEN_SOURCE 600
#include <sys/select.h>
#include <sys/wait.h>
#include <ctype.h>
#include "pty_fork.h"
#include "tlpi_hdr.h"

#define BUF_SIZE 256
#define MAX_SNAME 1000
#define CHAR_DELAY_US 1000  // 1ms between characters

// parse special key sequences like <ESC>, <ENTER>, etc.
// returns length consumed from input, writes character to *ch
static int
parseSpecialKey(const char *str, char *ch)
{
    if (str[0] != '<')
        return 0;

    const char *end = strchr(str, '>');
    if (end == NULL)
        return 0;

    int len = end - str + 1;
    char key[32];
    int keyLen = (end - str - 1);
    
    if (keyLen >= sizeof(key))
        return 0;
    
    strncpy(key, str + 1, keyLen);
    key[keyLen] = '\0';

    // convert to lowercase for comparison
    for (int i = 0; key[i]; i++)
        key[i] = tolower(key[i]);

    if (strcmp(key, "esc") == 0 || strcmp(key, "escape") == 0) {
        *ch = 27;  // ESC
    } else if (strcmp(key, "enter") == 0 || strcmp(key, "cr") == 0) {
        *ch = '\n';
    } else if (strcmp(key, "tab") == 0) {
        *ch = '\t';
    } else if (strcmp(key, "backspace") == 0 || strcmp(key, "bs") == 0) {
        *ch = 8;  // backspace
    } else {
        return 0;  // unrecognized
    }

    return len;
}

// send a command string to vi character by character with delays
static void
sendCommand(int masterFd, const char *cmd)
{
    for (const char *p = cmd; *p != '\0'; ) {
        char ch;
        int consumed = parseSpecialKey(p, &ch);
        
        if (consumed > 0) {
            // special key parsed
            p += consumed;
        } else {
            // regular character
            ch = *p++;
        }

        if (write(masterFd, &ch, 1) != 1)
            fatal("write to pty master");
        
        usleep(CHAR_DELAY_US);
    }
}

// drain output from vi without blocking
static void
drainOutput(int masterFd)
{
    char buf[BUF_SIZE];
    fd_set readFds;
    struct timeval tv;

    for (;;) {
        FD_ZERO(&readFds);
        FD_SET(masterFd, &readFds);
        
        tv.tv_sec = 0;
        tv.tv_usec = 0;  // non-blocking poll

        int ready = select(masterFd + 1, &readFds, NULL, NULL, &tv);
        if (ready <= 0)
            break;  // no data available or error
        
        ssize_t n = read(masterFd, buf, BUF_SIZE);
        if (n <= 0)
            break;
        
        // discard output (could optionally log for debugging)
    }
}

int
main(int argc, char *argv[])
{
    int masterFd;
    pid_t childPid;
    FILE *scriptFp;
    char line[1024];
    int status;

    if (argc != 3)
        usageErr("%s filename script-file\n", argv[0]);

    const char *viFile = argv[1];
    const char *scriptFile = argv[2];

    // open script file
    scriptFp = fopen(scriptFile, "r");
    if (scriptFp == NULL)
        errExit("fopen %s", scriptFile);

    // create pty and fork
    childPid = ptyFork(&masterFd, NULL, MAX_SNAME, NULL, NULL);
    if (childPid == -1)
        errExit("ptyFork");

    if (childPid == 0) {
        // child: exec vi with the file
        execlp("vi", "vi", viFile, (char *) NULL);
        errExit("execlp vi");
    }

    // parent: send script commands to vi

    // give vi time to initialize
    usleep(100000);  // 100ms
    drainOutput(masterFd);

    // read and send each line from script
    while (fgets(line, sizeof(line), scriptFp) != NULL) {
        // remove trailing newline if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        // skip empty lines and comments
        if (line[0] == '\0' || line[0] == '#')
            continue;

        sendCommand(masterFd, line);
        drainOutput(masterFd);
    }

    fclose(scriptFp);

    // give vi time to complete
    usleep(100000);  // 100ms
    drainOutput(masterFd);

    // close master to signal EOF to vi
    close(masterFd);

    // wait for vi to exit
    if (waitpid(childPid, &status, 0) == -1)
        errExit("waitpid");

    if (WIFEXITED(status)) {
        int exitStatus = WEXITSTATUS(status);
        if (exitStatus != 0) {
            fprintf(stderr, "vi exited with status %d\n", exitStatus);
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "vi terminated abnormally\n");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
