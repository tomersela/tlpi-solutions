#include "svmsg_file_mod.h"

int
main(int argc, char *argv[])
{
    struct requestMsg req;
    int serverId, clientId;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s pathname\n", argv[0]);
        return 1;
    }

    // Read server's message queue ID
    FILE *keyFile = fopen(KEY_FILE, "r");
    if (!keyFile) {
        perror("fopen");
        return 1;
    }

    if (fscanf(keyFile, "%d", &serverId) != 1) {
        perror("fscanf");
        return 1;
    }
    fclose(keyFile);

    // Create client's private message queue
    clientId = msgget(IPC_PRIVATE, S_IRUSR | S_IWUSR | S_IWGRP);
    if (clientId == -1) {
        perror("msgget");
        return 1;
    }

    // Fill out and send the request
    req.mtype = 1;
    req.clientId = clientId;
    strncpy(req.pathname, argv[1], sizeof(req.pathname) - 1);
    req.pathname[sizeof(req.pathname) - 1] = '\0';

    if (msgsnd(serverId, &req, REQ_MSG_SIZE, 0) == -1) {
        perror("msgsnd");
        return 1;
    }

    // Simulate being "alive but unresponsive"
    sleep(6);  // Server should timeout at 5s

    return 0;
}
