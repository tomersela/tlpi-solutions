#include <stdint.h>

#define PORT_NUM "9005"

#define OP_GET 1
#define OP_SET 2
#define OP_DELETE 3

#define RES_STATUS_OK 0
#define RES_STATUS_ERR_FULL 1
#define RES_STATUS_ERR_NOTFOUND 2
#define RES_STATUS_ERR_NOMEM 3
#define RES_STATUS_ERR_PERM 4 
#define RES_STATUS_ERR_INVALID_REQ 5
#define RES_STATUS_ERR_INTERNAL 6


struct request_hdr {
    uint32_t opcode;
    uint32_t key_len;
    uint32_t value_len;
};


struct response {
    uint32_t status;
    uint32_t value_len;
    char data[];
};
