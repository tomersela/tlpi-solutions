#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "unix_sockets.h"

#include "tlpi_hdr.h"


/* Create socket and connect it to the UNIX domain socket at 'path'.
   Return socket descriptor on success, or -1 on error */
int
unixConnect(const char *path, int type)
{
    struct sockaddr_un addr;
    int sfd;

    sfd = socket(AF_UNIX, type, 0);
    if (sfd == -1)
        return -1;

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    
    if (strlen(path) >= sizeof(addr.sun_path)) {
        close(sfd);
        errno = ENAMETOOLONG;
        return -1;
    }
    
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        int savedErrno = errno;
        close(sfd);
        errno = savedErrno;
        return -1;
    }

    return sfd;
}


/* Create a UNIX domain socket and bind it to 'path'.
   If 'doListen' is TRUE, make this a listening socket with 'backlog'.
   Return socket descriptor on success, or -1 on error */
static int
unixPassiveSocket(const char *path, int type, Boolean doListen, int backlog)
{
    struct sockaddr_un addr;
    int sfd;

    sfd = socket(AF_UNIX, type, 0);
    if (sfd == -1)
        return -1;

    /* Remove any existing socket file to avoid bind() failing */
    if (unlink(path) == -1 && errno != ENOENT) {
        int savedErrno = errno;
        close(sfd);
        errno = savedErrno;
        return -1;
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    
    if (strlen(path) >= sizeof(addr.sun_path)) {
        close(sfd);
        errno = ENAMETOOLONG;
        return -1;
    }
    
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1) {
        int savedErrno = errno;
        close(sfd);
        errno = savedErrno;
        return -1;
    }

    if (doListen) {
        if (listen(sfd, backlog) == -1) {
            int savedErrno = errno;
            close(sfd);
            unlink(path);  /* Clean up socket file */
            errno = savedErrno;
            return -1;
        }
    }

    return sfd;
}


/* Create stream socket, bound to 'path'. Make the socket a listening socket,
   with the specified 'backlog'. Return socket descriptor on success, 
   or -1 on error */
int
unixListen(const char *path, int backlog)
{
    return unixPassiveSocket(path, SOCK_STREAM, TRUE, backlog);
}


/* Create socket bound to 'path' with specified 'type'.
   Return socket descriptor on success, or -1 on error */
int
unixBind(const char *path, int type)
{
    return unixPassiveSocket(path, type, FALSE, 0);
}


/* Given a UNIX domain socket address in 'addr', return a null-terminated 
   string containing the socket path in the form "unix:path". The string is
   returned in the buffer pointed to by 'addrStr', and this value is also
   returned as the function result. The caller must specify the size of the
   'addrStr' buffer in 'addrStrLen' */
char *
unixAddressStr(const struct sockaddr *addr, socklen_t addrlen, char *addrStr, int addrStrLen)
{
    struct sockaddr_un *unAddr = (struct sockaddr_un *) addr;

    if (addr->sa_family == AF_UNIX) {
        if (unAddr->sun_path[0] == '\0') {
            /* Abstract socket */
            snprintf(addrStr, addrStrLen, "unix:@%.*s", 
                    (int)(addrlen - sizeof(sa_family_t) - 1), 
                    &unAddr->sun_path[1]);
        } else {
            /* Pathname socket */
            snprintf(addrStr, addrStrLen, "unix:%s", unAddr->sun_path);
        }
    } else {
        snprintf(addrStr, addrStrLen, "unix:?UNKNOWN?");
    }

    return addrStr;
}


/* Remove socket file from filesystem. Should be called by server
   when shutting down to clean up socket file */
int
unixRemove(const char *path)
{
    return unlink(path);
}
