#include <stdint.h>
#include <sys/socket.h>


#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 4096
#define MAX_RECORDS 1024

#define KV_OK           0
#define KV_ERR_FULL    -1
#define KV_ERR_NOTFOUND -2
#define KV_ERR_NOMEM   -3
#define KV_ERR_PERM    -4 

struct kv_record {
    uint32_t key_len;
    uint32_t value_len;
    struct sockaddr_storage client_addr;
    char data[]; // dynamically allocated [key bytes][value bytes]
};

void kv_store_init(void);
void kv_store_cleanup(void);
int kv_store_set(const char *key, int key_len,const char *value, int value_len, const struct sockaddr_storage *client_addr);
int kv_store_get(const char *key, int key_len, struct kv_record **result);
int kv_store_delete(const char *key, int key_len, const struct sockaddr_storage *client_addr);
