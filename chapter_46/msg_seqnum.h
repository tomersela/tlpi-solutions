#ifndef MSG_SEQNUM_H
#define MSG_SEQNUM_H

#include <sys/types.h>

#define KEY_PATH "/tmp/msg_seqnum_key"
#define KEY_ID 1234
#define REQ_MSG_TYPE 1

struct requestMsg {
    long mtype;      /* Must be REQ_MSG_TYPE */
    pid_t pid;
    int seqLen;
};

struct responseMsg {
    long mtype;      /* Must be pid of client */
    int seqNum;
};

#endif
