/* us_seqnum_client.c

   A simple client that uses a UNIX domain stream socket to request a (trivial)
   "sequence number service". The client connects to the server, sends a request
   consisting of its PID and the length of the sequence it wishes to be allocated
   by the server. The client then reads the server's response and displays it on stdout.

   See us_seqnum.h for the format of request and response messages.

   The server is in us_seqnum_server.c.
*/
#include "us_seqnum.h"

int
main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd;
    struct request req;
    struct response resp;

    if (argc > 1 && strcmp(argv[1], "--help") == 0)
        usageErr("%s [seq-len]\n", argv[0]);

    /* Create client socket */

    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        errExit("socket");

    /* Construct server address, and make the connection */

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1)
        errExit("connect");

    /* Send request to server */

    req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    if (write(sfd, &req, sizeof(struct request)) != sizeof(struct request))
        fatal("Can't write to server");

    /* Read and display response */

    if (read(sfd, &resp, sizeof(struct response)) != sizeof(struct response))
        fatal("Can't read response from server");

    printf("%d\n", resp.seqNum);
    exit(EXIT_SUCCESS);
}
