#include <sys/msg.h>
#include <sys/stat.h>

#include "tlpi_hdr.h"
#include "msg_seqnum.h"

int main(int argc, char *argv[]) {
    struct requestMsg req;
    struct responseMsg resp;
    int msqid, seqNum = 0;

    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == -1)
        errExit("ftok");

    msqid = msgget(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if (msqid == -1)
        errExit("msgget");

    for (;;) {
        if (msgrcv(msqid, &req, sizeof(struct requestMsg) - sizeof(long),
                   REQ_MSG_TYPE, 0) == -1) {
            errMsg("msgrcv");
            continue;
        }

        resp.mtype = req.pid;
        resp.seqNum = seqNum;

        if (msgsnd(msqid, &resp, sizeof(struct responseMsg) - sizeof(long), 0) == -1)
            errMsg("msgsnd");

        seqNum += req.seqLen;
    }
}
