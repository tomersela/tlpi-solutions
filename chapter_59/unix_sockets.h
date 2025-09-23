#ifndef UNIX_SOCKETS_H
#define UNIX_SOCKETS_H          /* Prevent accidental double inclusion */

#include <sys/socket.h>
#include <sys/un.h>

/* Connect to UNIX domain socket at 'path' with specified 'type' 
   (SOCK_STREAM or SOCK_DGRAM). Return socket descriptor on success, 
   or -1 on error */
int unixConnect(const char *path, int type);

/* Create UNIX domain socket bound to 'path'. Make it a listening socket
   with specified 'backlog'. Return socket descriptor on success, 
   or -1 on error */
int unixListen(const char *path, int backlog);

/* Create UNIX domain socket bound to 'path' with specified 'type'.
   Return socket descriptor on success, or -1 on error */
int unixBind(const char *path, int type);

/* Format UNIX domain socket address as a string. Return formatted string
   in 'addrStr' buffer of size 'addrStrLen'. Also returns 'addrStr' as
   function result */
char *unixAddressStr(const struct sockaddr *addr, socklen_t addrlen,
                     char *addrStr, int addrStrLen);

/* Remove socket file from filesystem. Should be called by server
   when shutting down to clean up socket file */
int unixRemove(const char *path);

#define US_ADDR_STR_LEN 1024    /* Suggested length for string buffer that
                                   caller should pass to unixAddressStr() */
#endif
