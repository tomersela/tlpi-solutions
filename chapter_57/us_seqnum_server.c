/* us_seqnum_server.c

   An example of a server using a UNIX domain stream socket to handle client requests.
   The "service" provided is the allocation of unique sequential
   numbers. Each client submits a request consisting of its PID, and
   the length of the sequence it is to be allocated by the server.

   The server reads each client request from the socket connection,
   and sends back the starting value of the sequence allocated to that
   client. The server then increments its counter of used numbers
   by the length specified in the client request.

   See us_seqnum.h for the format of request and response messages.

   The client is in us_seqnum_client.c.
*/
#include "us_seqnum.h"

#define BACKLOG 5

int
main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    int sfd, cfd;
    struct request req;
    struct response resp;
    int seqNum = 0;                     /* This is our "service" */

    /* Create server socket */
    
    sfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sfd == -1)
        errExit("socket");

    /* Construct server socket address, bind socket to it,
       and make this a listening socket */

    if (strlen(SV_SOCK_PATH) > sizeof(addr.sun_path) - 1)
        fatal("Server socket path too long: %s", SV_SOCK_PATH);

    if (remove(SV_SOCK_PATH) == -1 && errno != ENOENT)
        errExit("remove-%s", SV_SOCK_PATH);

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) - 1);

    if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
        errExit("bind");

    if (listen(sfd, BACKLOG) == -1)
        errExit("listen");

    for (;;) { // handle client connections

        /* Accept a connection */

        cfd = accept(sfd, NULL, NULL);
        if (cfd == -1)
            errExit("accept");

        /* Read request */

        if (read(cfd, &req, sizeof(struct request)) != sizeof(struct request)) {
            errMsg("Error reading request; closing connection");
            if (close(cfd) == -1)
                errMsg("close");
            continue;
        }

        /* Send response */

        resp.seqNum = seqNum;
        if (write(cfd, &resp, sizeof(struct response)) != sizeof(struct response))
            errMsg("Error writing response");

        seqNum += req.seqLen;           /* Update our sequence number */

        /* Close connection */

        if (close(cfd) == -1)
            errMsg("close");
    }
}
