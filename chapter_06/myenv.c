#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tlpi_hdr.h"

extern char **environ;

int mysetenv(char *name, char *value, int overwrite);
int myunsetenv(const char *name);

int
mysetenv(char *name, char *value, int overwrite) {
    if (name == NULL || *name == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }
    
    if (overwrite || !getenv(name)) {
        int envstrlen = strlen(name) + strlen(value) + 2;
        char* envstr = malloc(envstrlen);
        strcpy(envstr, name);
        strcat(envstr, "=");
        strcat(envstr, value);
        envstr[envstrlen] = '\n';
        if (putenv(envstr)) // putenv returns positive integer on error
            return -1;
    }

    return 0;
}

int
myunsetenv(const char *name) {
    if (name == NULL || *name == '\0' || strchr(name, '=') != NULL) {
        errno = EINVAL;
        return -1;
    }
    
    size_t namelen = strlen(name);
    int envcnt = 0;
    char **dest = environ;
    char **current = environ;

    while (*current != NULL) {
        if (strncmp(*current, name, namelen) || (*current)[namelen] != '=') {
            *dest = *current;
            dest++;
            envcnt++;
        }
        current++;
    }
    environ[envcnt] = NULL;

    return 0;
}

static void
printenv(void) {
    if (environ == NULL)
        return;

    for (char **ep = environ; *ep != NULL; ep++) {
        printf("%s\n", *ep);
    }
}

int
main(int argc, char* argv[], char* env[]) {
    clearenv();

    mysetenv("testing", "1, 2", 0);
    mysetenv("testing2", "3, 4", 0);
    mysetenv("testing3", "5, 6", 0);
    
    printf("Initial env variables:\n");
    printenv();
    
    assert(!strcmp(getenv("testing"), "1, 2"));
    assert(!strcmp(getenv("testing2"), "3, 4"));
    assert(!strcmp(getenv("testing3"), "5, 6"));
    
    myunsetenv("testing2");

    assert(getenv("testing2") == NULL);


    mysetenv("testing3", "shoud not change", 0);
    assert(!strcmp(getenv("testing3"), "5, 6"));

    mysetenv("testing", "one two", 1);
    assert(!strcmp(getenv("testing"), "one two"));

    printf("\n");
    printf("Final env variables:\n");
    printenv();
    return EXIT_SUCCESS;
}
