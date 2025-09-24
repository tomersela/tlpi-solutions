#include <pthread.h>
#include <stddef.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>

#include "inet_sockets.h"
#include "kv_store.h"
#include "kv_proto.h"
#include "tlpi_hdr.h"

#define BACKLOG_SIZE 10

static size_t
read_exact(int fd, void *buf, size_t n) {
    size_t read_size;
    size_t total_cnt = 0;
    while (total_cnt < n) {
        read_size = read(fd, ((char *) buf) + total_cnt, n - total_cnt);
        if (read_size <= 0) return read_size; // error or EOF
        total_cnt += read_size;
    }
    return total_cnt;
}


static int
parse_request_header(int cfd, struct request_hdr* req_hdr) {
    size_t read_size = 0;
    read_size = read_exact(cfd, req_hdr, sizeof(struct request_hdr));
    if (read_size <= 0) { // Error reading or EOF
        return -1;
    }

    // Convert from network byte order
    req_hdr->opcode = ntohl(req_hdr->opcode);
    req_hdr->key_len = ntohl(req_hdr->key_len);
    req_hdr->value_len = ntohl(req_hdr->value_len);

    if (req_hdr->key_len > MAX_KEY_LEN || req_hdr->value_len > MAX_VALUE_LEN) {
        return RES_STATUS_ERR_INVALID_REQ;
    }

    switch (req_hdr->opcode) {
        case OP_GET:
        case OP_DELETE:
            if (req_hdr->key_len == 0 || req_hdr->value_len != 0)
                return RES_STATUS_ERR_INVALID_REQ;
            break;
        case OP_SET:
            if (req_hdr->key_len == 0 || req_hdr->value_len == 0)
                return RES_STATUS_ERR_INVALID_REQ;
            break;
        default:
            return RES_STATUS_ERR_INVALID_REQ;
    }

    return 0;
}


static void
send_response(int cfd, int kv_result, struct kv_record *record, int opcode) {
    struct response res;

    // Convert kv_store result to protocol response
    switch (kv_result) {
        case KV_OK:
            res.status = htonl(RES_STATUS_OK);
            break;
        case KV_ERR_NOTFOUND:
            res.status = htonl(RES_STATUS_ERR_NOTFOUND);
            break;
        case KV_ERR_NOMEM:
            res.status = htonl(RES_STATUS_ERR_NOMEM);
            break;
        case KV_ERR_PERM:
            res.status = htonl(RES_STATUS_ERR_PERM);
            break;
        case KV_ERR_FULL:
            res.status = htonl(RES_STATUS_ERR_FULL);
            break;
        default:
            errMsg("Unexpected kv_store result: %d", kv_result);
            res.status = htonl(RES_STATUS_ERR_INTERNAL);
            break;
    }
    
    if (kv_result == KV_OK && opcode == OP_GET) {
        // GET success - send value back
        res.value_len = htonl(record->value_len);
        write(cfd, &res, sizeof(res));
        write(cfd, &record->data[record->key_len], record->value_len);
    } else { // error or SET/DELETE success
        res.value_len = 0;
        write(cfd, &res, sizeof(res));
    }
}


static void *
handle_client(void *arg) {
    int cfd = *(int *)arg;
    free(arg); // clean up the malloc'd fd
    
    int hdr_parse_res;
    struct sockaddr_storage claddr;
    struct request_hdr req_hdr;
    struct response res;
    size_t read_size;
    struct kv_record *record;
    int kv_res;

    char key_buffer[MAX_KEY_LEN];
    char value_buffer[MAX_VALUE_LEN];

    // get client address for security checks
    socklen_t alen = sizeof(claddr);
    if (getpeername(cfd, (struct sockaddr *) &claddr, &alen) == -1) {
        errMsg("getpeername");
        close(cfd);
        return NULL;
    }

    // limit connection to 30s to hang slow/hanged clients
    struct timeval timeout = {.tv_sec = 30, .tv_usec = 0};
    setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    hdr_parse_res = parse_request_header(cfd, &req_hdr);
    if (hdr_parse_res == -1) {
        errMsg("parse_request_header");
        close(cfd);
        return NULL;
    } else if (hdr_parse_res != 0) {
        res.status = hdr_parse_res;
        res.value_len = 0;
        write(cfd, &res, sizeof(struct response));
        close(cfd);
        return NULL;
    }

    read_size = read_exact(cfd, key_buffer, req_hdr.key_len);
    if (read_size == 0) {
        fprintf(stderr, "read_exact (key) - EOF early than expected\n");
        close(cfd);
        return NULL;
    } else if (read_size == -1) {
        errMsg("read_exact (key)");
        close(cfd);
        return NULL;
    }

    switch (req_hdr.opcode) {
        case OP_GET:
            kv_res = kv_store_get(key_buffer, req_hdr.key_len, &record);
            send_response(cfd, kv_res, record, OP_GET);
            break;

        case OP_SET:
            read_size = read_exact(cfd, value_buffer, req_hdr.value_len);
            if (read_size == 0) {
                fprintf(stderr, "read_exact (value) - EOF early than expected\n");
                close(cfd);
                return NULL;
            } else if (read_size == -1) {
                errMsg("read_exact (value)");
                close(cfd);
                return NULL;
            }
            kv_res = kv_store_set(key_buffer, req_hdr.key_len, value_buffer, req_hdr.value_len, &claddr);
            send_response(cfd, kv_res, NULL, OP_SET);
            break;

        case OP_DELETE:
            kv_res = kv_store_delete(key_buffer, req_hdr.key_len, &claddr);
            send_response(cfd, kv_res, NULL, OP_DELETE);
            break;

        default:
            // shouldn't get here
            errExit("Invalid opcode received for processing");
    }

    close(cfd);
    return NULL;
}


int
main(int argc, char* argvp[]) {
    int cfd;
    struct sockaddr_storage claddr;
    socklen_t addrlen;

    kv_store_init(); // initialize our key/value store

    /* Ignore the SIGPIPE signal, so that we find out about broken connection
       errors via a failure from write(). */
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) errExit("signal");

    int lfd = inetListen(PORT_NUM, BACKLOG_SIZE, &addrlen);
    if (lfd == -1) {
        fatal("inetListen() failed");
    }

    for (;;) {
        socklen_t alen = sizeof(claddr);
        cfd = accept(lfd, (struct sockaddr *) &claddr, &alen);
        if (cfd == -1) {
            errMsg("accept");
            continue;
        }

        // create thread to handle this client
        pthread_t thread;
        int *client_fd = malloc(sizeof(int));
        if (client_fd == NULL) {
            errMsg("malloc for client_fd");
            close(cfd);
            continue;
        }
        *client_fd = cfd;
        
        if (pthread_create(&thread, NULL, handle_client, client_fd) != 0) {
            errMsg("pthread_create");
            close(cfd);
            free(client_fd);
            continue;
        }
        
        // detach thread so it cleans up automatically when done
        pthread_detach(thread);
    }

    
}
