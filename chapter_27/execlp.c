#include <unistd.h>
#include <stdarg.h>
#include <limits.h>
#include <sys/wait.h>

#include "tlpi_hdr.h"
#include "ename.c.inc"

#define SHELL_PATH "/bin/sh"

typedef void (*func_ptr)(void);

extern char **environ;

int myexeclp(const char *file, const char *arg, ... /*, (char *) NULL */);


static int
execute(const char *full_path, char **args_vector)
{
    // calculate the size of environ
    int env_cnt = 0;
    while (environ[env_cnt] != NULL) 
        env_cnt++;

    // duplicate environment variables
    char **envp = (char **) calloc(sizeof(char *), env_cnt + 1);
    if (envp == NULL) {
        return -1;
    }

    int j;
    for (j = 0; environ[j] != NULL; j++)
        envp[j] = strdup(environ[j]);
    envp[j] = NULL;

    return execve(full_path, (char * const*) args_vector, envp);
}


static void
exec_sh_script(int argc, char *argv[])
{
    char *args[argc + 1];

    args[0] = SHELL_PATH;
    for (int j = 0; j <= argc; j++)
        args[j + 1] = argv[j];
    execute(SHELL_PATH, args);
}


int
myexeclp(const char *file, const char *arg, ... /*, (char *) NULL */)
{
    char const *str;
    char path_copy[PATH_MAX];
    va_list args;
    int found_eaccess_err;


    // count the number of arguments
    va_start(args, arg);
    int arg_cnt = 1;
    while (va_arg(args, const char *)) {
        arg_cnt++;
    }
    va_end(args);

    // allocate memory of the arguments vector
    char **args_vector = malloc((arg_cnt + 1) * sizeof(const char *));

    // fill the argument vector with pointers
    va_start(args, arg);
    str = arg;
    for (int i = 0; i < arg_cnt; ++i, str = va_arg(args, const char *)) {
        args_vector[i] = (char *) str;
    }
    va_end(args);
    args_vector[arg_cnt] = NULL;

    if (strchr(file, '/') != NULL) {
        execute(file, args_vector);
        if (errno == ENOEXEC) {
            exec_sh_script(arg_cnt, args_vector);
        }
        free(args_vector);
        return -1;
    }
    
    char *path = getenv("PATH");
    if (path == NULL) {
        confstr(_CS_PATH, path_copy, PATH_MAX); // if PATH isn't available, use the _CS_PATH configuration variable
    } else {
        strncpy(path_copy, path, PATH_MAX);
        path_copy[PATH_MAX - 1] = '\0';
    }

    // iterate PATH entries, try to execute the given file within each directory
    char *saveptr;
    char full_path[PATH_MAX];
    char *path_entry = strtok_r(path_copy, ":", &saveptr);
    int return_value = 0;
    int saved_errno;
    while (path_entry != NULL) {
        snprintf(full_path, PATH_MAX, "%s/%s", path_entry, file);
        return_value = execute(full_path, args_vector);
        saved_errno = errno;
        /*
            According to the manpage:
                If permission is denied for a file (the attempted execve(2) failed
                with the error EACCES), these functions will continue searching
                the rest of the search path.  If no other file is found, however,
                they will return with errno set to EACCES.
            
            Means that if at least one of the entries resulted in a EACCES error,
             and there was no successful execution - we should set errno to EACCES.
        */
        if (errno == EACCES)
            found_eaccess_err = TRUE;
        else if (errno == ENOEXEC)
            exec_sh_script(arg_cnt, args_vector);

        if (return_value == 0)
            return return_value;
            
        path_entry = strtok_r(NULL, ":", &saveptr); // next path entry
    }

    free(args_vector);

    errno = found_eaccess_err ? EACCES : saved_errno;

    return return_value;
}


static void
run_in_child_process(func_ptr func)
{
    pid_t pid = fork();

    if (pid == -1) {
        errExit("fork");
    } else if (pid == 0) {
        func();
        _exit(EXIT_SUCCESS);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("child exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("child did not exit normally\n");
        }
    }
}


static void
say_hello()
{
    printf("** Testing say_hello:\n");
    myexeclp("echo", "echo", "Hello!", (char *) NULL);
}


static void
sleep2()
{
    printf("** Testing sleep2:\n");
    printf("Sleeping for 2 seconds...\n");
    myexeclp("sleep", "sleep", "2", (char *) NULL);
}


static void
absolute_path()
{
    printf("** Testing absolute_path:\n");
    myexeclp("/bin/echo", "echo", "Absolut path - Still works!", (char *) NULL);
}


static void
use_sh_script()
{
    printf("** Testing use_sh_script:\n");
    myexeclp("./magic8ball.sh", "./magic8ball.sh", (char *) NULL);
}


static void
permission_denied()
{
    printf("** Testing permission_denied:\n");
    myexeclp("require_root.sh", "require_root.sh", (char *) NULL);
    printf("errno = %s\n", ename[errno]);
}


int
main()
{
    run_in_child_process(say_hello);
    run_in_child_process(sleep2);
    run_in_child_process(absolute_path);
    run_in_child_process(use_sh_script);
    run_in_child_process(permission_denied);
    
    return 0;
}
