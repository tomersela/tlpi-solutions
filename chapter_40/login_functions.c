#define _GNU_SOURCE
#include <utmpx.h>
#include <paths.h>
#include <time.h>
#include <pwd.h>
#include <fcntl.h>
#include <lastlog.h>

#include "tlpi_hdr.h"


void login(const struct utmp *ut);
int logout(const char *line);
void logwtmp(const char *line, const char *name, const char *host);


void
login(const struct utmp *ut)
{
    struct utmpx utx;
    struct lastlog ll;
    int fd;

    memset(&utx, 0, sizeof(struct utmpx));
    utx.ut_type = USER_PROCESS;
    strncpy(utx.ut_user, ut->ut_name, sizeof(utx.ut_user));  // ut_name in utmp, ut_user in utmpx
    strncpy(utx.ut_line, ut->ut_line, sizeof(utx.ut_line));
    strncpy(utx.ut_host, ut->ut_host, sizeof(utx.ut_host));
    utx.ut_pid = getpid();
    utx.ut_tv.tv_sec = time(NULL);

    // update utmp file
    setutxent();
    if (pututxline(&utx) == NULL) {
        endutxent();
        return;
    }
    endutxent();

    // update wtmp file
    updwtmpx(_PATH_WTMP, &utx);

    // update lastlog file
    memset(&ll, 0, sizeof(struct lastlog));
    ll.ll_time = utx.ut_tv.tv_sec;
    strncpy(ll.ll_line, utx.ut_line, sizeof(ll.ll_line));
    strncpy(ll.ll_host, utx.ut_host, sizeof(ll.ll_host));

    fd = open(_PATH_LASTLOG, O_RDWR | O_CREAT, 
             S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);  // rw-r--r--
    if (fd == -1)
        errExit("open");

    struct passwd *pwd = getpwnam(ut->ut_name);
    if (pwd != NULL) {
        if (lseek(fd, pwd->pw_uid * sizeof(struct lastlog), SEEK_SET) != -1) {
            if (write(fd, &ll, sizeof(struct lastlog)) != sizeof(struct lastlog)) {
                close(fd);
                errExit("write");
            }
        }
    }
    close(fd);
}


int
logout(const char *line)
{
    struct utmpx *ut;
    int status = 0;

    // search for the entry with the given line
    setutxent();
    while ((ut = getutxent()) != NULL) {
        if (ut->ut_type == USER_PROCESS && 
            strncmp(ut->ut_line, line, sizeof(ut->ut_line)) == 0) {
            struct utmpx temp = *ut;
            temp.ut_type = DEAD_PROCESS;
            temp.ut_tv.tv_sec = time(NULL);
            memset(temp.ut_user, 0, sizeof(temp.ut_user));

            if (pututxline(&temp) == NULL) {
                status = -1;
            } else {
                status = 1;
                // update wtmp file
                updwtmpx(_PATH_WTMP, &temp);
            }
            break;
        }
    }
    endutxent();

    return status;
}


void
logwtmp(const char *line, const char *name, const char *host)
{
    struct utmpx ut;

    memset(&ut, 0, sizeof(struct utmpx));
    ut.ut_pid = getpid();
    strncpy(ut.ut_line, line, sizeof(ut.ut_line));
    if (name[0]) {
        ut.ut_type = USER_PROCESS;
        strncpy(ut.ut_user, name, sizeof(ut.ut_user));
    } else {
        ut.ut_type = DEAD_PROCESS;
    }
    strncpy(ut.ut_host, host, sizeof(ut.ut_host));
    ut.ut_tv.tv_sec = time(NULL);

    updwtmpx(_PATH_WTMP, &ut);
}


int
main(int argc, char *argv[])
{
    struct utmp ut;
    int result;

    if (argc != 2)
        usageErr("%s username\n", argv[0]);

    printf("Testing login() function...\n");
    memset(&ut, 0, sizeof(struct utmp));
    strncpy(ut.ut_name, argv[1], sizeof(ut.ut_name));
    strncpy(ut.ut_line, "pts/0", sizeof(ut.ut_line));
    strncpy(ut.ut_host, "localhost", sizeof(ut.ut_host));

    login(&ut);
    printf("login() completed\n");

    printf("Sleeping for 2 seconds...\n");
    sleep(2);

    printf("\nTesting logout() function...\n");
    result = logout("pts/0");
    printf("logout() returned: %d\n", result);
    if (result == -1)
        errExit("logout");

    printf("\nTesting logwtmp() function...\n");
    printf("Writing login record...\n");
    logwtmp("pts/0", argv[1], "localhost");
    
    sleep(1);
    
    printf("Writing logout record...\n");
    logwtmp("pts/0", "", "");

    printf("\nTest complete. You can examine the following files:\n");
    printf("\t%s (utmp)\n", _PATH_UTMP);
    printf("\t%s (wtmp)\n", _PATH_WTMP);
    printf("\t%s (lastlog)\n", _PATH_LASTLOG);

    exit(EXIT_SUCCESS);
}
