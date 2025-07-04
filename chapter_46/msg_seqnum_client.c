#include <sys/msg.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"
#include "msg_seqnum.h"

int main(int argc, char *argv[]) {
    struct requestMsg req;
    struct responseMsg resp;
    int msqid;

    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == -1)
        errExit("ftok");

    msqid = msgget(key, S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget");

    req.mtype = REQ_MSG_TYPE;
    req.pid = getpid();
    req.seqLen = (argc > 1) ? getInt(argv[1], GN_GT_0, "seq-len") : 1;

    if (msgsnd(msqid, &req, sizeof(struct requestMsg) - sizeof(long), 0) == -1)
        errExit("msgsnd");

    if (msgrcv(msqid, &resp, sizeof(struct responseMsg) - sizeof(long),
               req.pid, 0) == -1)
        errExit("msgrcv");

    printf("%d\n", resp.seqNum);
    exit(EXIT_SUCCESS);
}
