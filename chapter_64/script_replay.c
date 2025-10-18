#include <sys/stat.h>
#include <fcntl.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 2048

int
main(int argc, char *argv[])
{
    FILE *fp;
    char line[BUF_SIZE];
    char unescaped[BUF_SIZE];
    long long timestamp, prevTimestamp = 0;
    char *content;
    int unescapedLen;

    // open timed file
    const char *filename = (argc > 1) ? argv[1] : "typescript.timed";
    fp = fopen(filename, "r");
    if (fp == NULL)
        errExit("fopen");

    // read and replay each line
    while (fgets(line, BUF_SIZE, fp) != NULL) {
        // parse timestamp (everything before first space)
        timestamp = strtoll(line, &content, 10);
        if (*content != ' ')
            fatal("invalid format: expected space after timestamp");
        content++;  // skip the space

        // calculate delay since last output
        long long delayMs = timestamp - prevTimestamp;
        if (delayMs > 0)
            usleep(delayMs * 1000);  // convert ms to microseconds

        // unescape content and write to stdout
        unescapedLen = 0;
        for (char *p = content; *p != '\0' && *p != '\n'; p++) {
            if (*p == '\\' && *(p + 1) != '\0') {
                p++;  // skip backslash, look at next char
                if (*p == 'n')
                    unescaped[unescapedLen++] = '\n';
                else if (*p == 'r')
                    unescaped[unescapedLen++] = '\r';
                else if (*p == '\\')
                    unescaped[unescapedLen++] = '\\';
                else {
                    // unknown escape sequence, write both chars
                    unescaped[unescapedLen++] = '\\';
                    unescaped[unescapedLen++] = *p;
                }
            } else {
                unescaped[unescapedLen++] = *p;
            }
        }

        if (write(STDOUT_FILENO, unescaped, unescapedLen) != unescapedLen)
            fatal("partial/failed write");

        prevTimestamp = timestamp;
    }

    fclose(fp);
    exit(EXIT_SUCCESS);
}
