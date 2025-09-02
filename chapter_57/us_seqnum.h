/* us_seqnum.h

   Header file used by us_seqnum_server.c and us_seqnum_client.c

   These programs employ a socket in /tmp. This makes it easy to compile
   and run the programs. However, for a security reasons, a real-world
   application should never create sensitive files in /tmp.
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "tlpi_hdr.h"

#define SV_SOCK_PATH "/tmp/us_seqnum"

struct request {
    pid_t pid; // pid of client
    int seqLen; // length of desired sequence
};

struct response {
    int seqNum; // start of sequence
};
