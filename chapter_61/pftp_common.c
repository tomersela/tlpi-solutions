#include <time.h>
#include "pftp.h"

int
validate_filename(const char *filename)
{
    // basic security checks
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }

    // check for directory traversal attempts
    if (strstr(filename, "..") != NULL) {
        printf("security: blocked directory traversal attempt: %s\n", filename);
        return 0;
    }

    // check for absolute paths
    if (filename[0] == '/') {
        printf("security: blocked absolute path attempt: %s\n", filename);
        return 0;
    }

    // check for hidden files (starting with .)
    if (filename[0] == '.') {
        printf("security: blocked hidden file access: %s\n", filename);
        return 0;
    }

    // check filename length
    if (strlen(filename) > MAX_FILENAME_LEN) {
        printf("security: filename too long: %s\n", filename);
        return 0;
    }

    return 1;
}

void
generate_cookie(char *cookie, size_t cookie_size)
{
    // generate a simple random cookie
    srand(time(NULL) + getpid());
    snprintf(cookie, cookie_size, "COOKIE-%08x%08x",
             (unsigned int)rand(), (unsigned int)rand());
}

int
parse_connect_command(const char *message, char *cookie, char *port_str)
{
    // expected format: "CONNECT:COOKIE-abc123:PORT-54321"
    // note: we expect the full cookie including "COOKIE-" prefix
    if (sscanf(message, "CONNECT:%63[^:]:%15s", cookie, port_str) != 2) {
        return 0;
    }
    return 1;
}
