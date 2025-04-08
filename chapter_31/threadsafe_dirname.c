#include <string.h>
#include <pthread.h>

#include <assert.h>

#include "tlpi_hdr.h"


char *dirname(char *path);


#define BUF_SIZE 4096


static __thread char buf[BUF_SIZE];


char *
dirname(char *path)
{
    if (path == NULL || *path == '\0') {
        return ".";
    }

    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    // remove trailing slashes (unless the path is just "/")
    size_t len = strlen(buf);
    while (len > 1 && buf[len - 1] == '/') {
        buf[--len] = '\0';
    }

    char *slash = strrchr(buf, '/');
    if (slash == NULL) {
        return ".";   // no slash
    }

    // remove the filename part
    while (slash > buf && *slash == '/') {
        *slash-- = '\0';
    }

    if (slash == buf && *slash == '/') { // buf is "/"
        buf[1] = '\0';
        return buf;
    }

    *++slash = '\0';
    if (buf[0] == '\0') { // empty string after removing all trailing "/"
        return ".";
    }

    return buf;
}


static void *
thread_func(void *arg)
{
    printf("Other thread about to call dirname()\n");
    char *str = dirname("/usr/lib");
    printf("Other thread: str (%p) = %s\n", str, str);
    return NULL;
}


int
main(int argc, char* argv[])
{
    pthread_t t;
    int res;
    char *str;

    assert(strcmp(dirname("/"), "/") == 0);
    assert(strcmp(dirname(""), ".") == 0);
    assert(strcmp(dirname("/usr/lib"), "/usr") == 0);
    assert(strcmp(dirname("/usr//"), "/") == 0);


    str = dirname("/etc/hosts");

    if ((res = pthread_create(&t, NULL, thread_func, NULL)) != 0) {
        errExitEN(res, "pthread_create");
    }
    
    if ((res = pthread_join(t, NULL)) != 0)
        errExitEN(res, "pthread_join");

    printf("Main thread: str (%p) = %s\n", str, str);

    exit(EXIT_SUCCESS);
}
