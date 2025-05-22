#define _GNU_SOURCE

#include <pwd.h>
#include <shadow.h>
#include <crypt.h>
#include <grp.h>

#include "tlpi_hdr.h"


static void show_usage(char *program_name)
{
    usageErr("Usage: %s [-u user] program [args...]\n", program_name);
}


int
main(int argc, char *argv[])
{
    char *password, *encrypted, *p;
    struct passwd *pwd;
    struct spwd *spwd;
    Boolean auth_res;

    int opt;
    char *username = "root";

    while ((opt = getopt(argc, argv, "+u:")) != -1) {
        switch (opt) {
            case 'u':
                username = optarg;
                break;
            default:
                show_usage(argv[0]);
        }
    }

    // remaining arguments after options are the program and its arguments    
    if (optind >= argc) {
        show_usage(argv[0]);
    }

    // get user record in passwords file
    pwd = getpwnam(username);
    if (pwd == NULL)
        errExit("Couldn't find user record for %s", username);

    // check if there's a shadow password record
    spwd = getspnam(username);
    if (spwd == NULL && errno == EACCES)
        errExit("no permission to read shadow password file");

    if (spwd != NULL)           /* If there is a shadow password record */
        pwd->pw_passwd = spwd->sp_pwdp;     /* Use the shadow password */

    password = getpass("Password: ");

    encrypted = crypt(password, pwd->pw_passwd); /* Encrypt password and erase cleartext version immediately */
    p = password;
    while (*p != '\0')
        *p++ = '\0';

    if (encrypted == NULL)
        errExit("crypt");

    auth_res = strcmp(encrypted, pwd->pw_passwd) == 0;
    if (!auth_res) {
        errExit("Incorrect password\n");
    }

    // set group and user ids
    if (initgroups(pwd->pw_name, pwd->pw_gid) == -1)
        errExit("initgroups");

    if (setgid(pwd->pw_gid) == -1)
        errExit("setgid");

    if (setuid(pwd->pw_uid) == -1)
        errExit("setuid");

    // execute the program
    execvp(argv[optind], &argv[optind]);

    errExit("execvp"); // if execvp returns, there was an error

    exit(EXIT_SUCCESS);
}
