
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "kv_store.h"
#include "tlpi_hdr.h"


static struct kv_record *records[MAX_RECORDS];
static int record_cnt = 0;

static pthread_mutex_t records_mutex = PTHREAD_MUTEX_INITIALIZER;


// compare only IP addresses, ignore ports
static int
same_ip_address(const struct sockaddr_storage *addr1, const struct sockaddr_storage *addr2) {
    if (addr1->ss_family != addr2->ss_family) {
        return 0; // different address families
    }
    
    if (addr1->ss_family == AF_INET) {
        struct sockaddr_in *ipv4_1 = (struct sockaddr_in *)addr1;
        struct sockaddr_in *ipv4_2 = (struct sockaddr_in *)addr2;
        return ipv4_1->sin_addr.s_addr == ipv4_2->sin_addr.s_addr;
    } else if (addr1->ss_family == AF_INET6) {
        struct sockaddr_in6 *ipv6_1 = (struct sockaddr_in6 *)addr1;
        struct sockaddr_in6 *ipv6_2 = (struct sockaddr_in6 *)addr2;
        return memcmp(&ipv6_1->sin6_addr, &ipv6_2->sin6_addr, sizeof(struct in6_addr)) == 0;
    }
    
    return 0; // unsupported address family
}


// WARNING: not thread safe! callers to this function should take care of thread safety
static int
kv_store_get_idx(const char *key, int key_len) {
    for (int i = 0; i < record_cnt; i++) {
        struct kv_record *curr = records[i];
        if (key_len == curr->key_len && memcmp(key, curr->data, key_len) == 0) {
            return i;
        }
    }
    return -1;
}


void
kv_store_init() {
    kv_store_cleanup();
}


void
kv_store_cleanup() {
    pthread_mutex_lock(&records_mutex);
    for (int i = 0; i < record_cnt; i++) {
        free(records[i]);
    }
    record_cnt = 0;  // reset counter after freeing
    pthread_mutex_unlock(&records_mutex);
}


int
kv_store_set(const char *key, int key_len,const char *value, int value_len, const struct sockaddr_storage *client_addr) {
    pthread_mutex_lock(&records_mutex);

    // check for existing record
    int maybe_record_idx = kv_store_get_idx(key, key_len);
    if (maybe_record_idx >= 0) { // record exists
        struct kv_record *existing = records[maybe_record_idx];
        // validate the original creator is the same as the current user (compare IP only, not port)
        if (!same_ip_address(client_addr, &existing->client_addr)) {
            pthread_mutex_unlock(&records_mutex);
            return KV_ERR_PERM;
        }
    } else if (record_cnt >= MAX_RECORDS) { // max capacity reached
        pthread_mutex_unlock(&records_mutex);
        return KV_ERR_FULL;
    }

    struct kv_record *new_record = (struct kv_record *) malloc(sizeof(struct kv_record) + key_len + value_len);
    if (new_record == NULL) { // malloc failed
        pthread_mutex_unlock(&records_mutex);
        return KV_ERR_NOMEM;
    }
    new_record->key_len = key_len;
    new_record->value_len = value_len;
    new_record->client_addr = *client_addr;
    memcpy(new_record->data, key, key_len);
    memcpy(&new_record->data[key_len], value, value_len);

    // check for existing record
    if(maybe_record_idx == -1) {
        records[record_cnt] = new_record;
        record_cnt++;
    } else {
        free(records[maybe_record_idx]); // deallocate existing record
        records[maybe_record_idx] = new_record; // persist new one
    }

    pthread_mutex_unlock(&records_mutex);

    return KV_OK;
}


int
kv_store_get(const char *key, int key_len, struct kv_record **result) {
    pthread_mutex_lock(&records_mutex);

    int maybe_record_idx = kv_store_get_idx(key, key_len);
    if (maybe_record_idx == -1) {
        pthread_mutex_unlock(&records_mutex);
        return KV_ERR_NOTFOUND;
    }

    *result = records[maybe_record_idx];

    pthread_mutex_unlock(&records_mutex);

    return KV_OK;
}


int
kv_store_delete(const char *key, int key_len, const struct sockaddr_storage *client_addr) {
    pthread_mutex_lock(&records_mutex);

    int maybe_record_idx = kv_store_get_idx(key, key_len);
    if (maybe_record_idx == -1) {
        pthread_mutex_unlock(&records_mutex);
        return KV_ERR_NOTFOUND;
    }

    struct kv_record *existing = records[maybe_record_idx];
    // validate the original creator is the same as the current user (compare IP only, not port)
    if (!same_ip_address(client_addr, &existing->client_addr)) {
        pthread_mutex_unlock(&records_mutex);
        return KV_ERR_PERM;
    }

    int last_record_idx = record_cnt - 1;
    if (maybe_record_idx != last_record_idx) { // we're deleting a record within the array (not the last)
        // in order to keep record continuity (other methods in this unit rely on that), we replace the freed record with the last one
        records[maybe_record_idx] = records[last_record_idx];
    }

    record_cnt--;
    free(existing);

    pthread_mutex_unlock(&records_mutex);

    return KV_OK;
}
