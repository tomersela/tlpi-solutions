#include <string.h>
#include <pthread.h>

#include <assert.h>

#include "tlpi_hdr.h"


char *basename(char *path);


#define BUF_SIZE 4096


static __thread char buf[BUF_SIZE];


char *
basename(char *path)
{
    if (path == NULL || *path == '\0') {
        return ".";
    }

    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    size_t len = strlen(buf);

    // remove trailing slashes (unless the path is just "/")
    while (len > 1 && buf[len - 1] == '/') {
        buf[--len] = '\0';
    }

    char *slash = strrchr(buf, '/');
    if (slash == NULL) {
        return buf;  // no slash, whole string is the basename
    }

    // if it's just slashes (e.g. "/", "///"), return "/"
    if (slash == buf && *(slash + 1) == '\0') {
        return "/";
    }

    // otherwise, return the part after the last slash
    return slash + 1;
}


static void *
thread_func(void *arg)
{
    printf("Other thread about to call basename()\n");
    char *str = basename("/usr/lib");
    printf("Other thread: str (%p) = %s\n", str, str);
    return NULL;
}


int
main(int argc, char* argv[])
{
    pthread_t t;
    int res;
    char *str;

    assert(strcmp(basename("/"), "/") == 0);
    assert(strcmp(basename(""), ".") == 0);
    assert(strcmp(basename("/usr/lib"), "lib") == 0);
    assert(strcmp(basename("/usr//"), "usr") == 0);


    str = basename("/etc/hosts");

    if ((res = pthread_create(&t, NULL, thread_func, NULL)) != 0) {
        errExitEN(res, "pthread_create");
    }
    
    if ((res = pthread_join(t, NULL)) != 0)
        errExitEN(res, "pthread_join");

    printf("Main thread: str (%p) = %s\n", str, str);

    exit(EXIT_SUCCESS);
}
