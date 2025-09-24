#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include "kv_proto.h"
#include "tlpi_hdr.h"

static void
print_usage(const char *progname) {
    fprintf(stderr, "Usage: %s [-c client_ip] [-h server_host] <operation> <key> [value]\n", progname);
    fprintf(stderr, "Operations: GET, SET, DELETE\n");
    fprintf(stderr, "  -c client_ip   Client IP to bind to (default: 127.0.0.1)\n");
    fprintf(stderr, "  -h server_host Server hostname/IP (default: localhost)\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s GET mykey\n", progname);
    fprintf(stderr, "  %s -c 127.0.0.2 SET mykey myvalue\n", progname);
    fprintf(stderr, "  %s -h server.com DELETE mykey\n", progname);
    exit(EXIT_FAILURE);
}

static void
handle_response(int cfd, const char *operation) {
    struct response res;
    if (read(cfd, &res, sizeof(res)) != sizeof(res))
        errExit("read response header");
    
    // convert from network byte order
    res.status = ntohl(res.status);
    res.value_len = ntohl(res.value_len);
    
    // handle response based on status
    if (res.status == RES_STATUS_OK) {
        if (strcmp(operation, "GET") == 0 && res.value_len > 0) {
            // read and display value for GET operation
            char *response_value = malloc(res.value_len + 1);
            if (read(cfd, response_value, res.value_len) != (ssize_t)res.value_len)
                errExit("read response value");
            response_value[res.value_len] = '\0';
            printf("Value: %s\n", response_value);
            free(response_value);
        } else {
            printf("Operation successful\n");
        }
    } else {
        // display error message
        printf("Error: ");
        switch (res.status) {
            case RES_STATUS_ERR_NOTFOUND:
                printf("Key not found\n");
                break;
            case RES_STATUS_ERR_PERM:
                printf("Permission denied\n");
                break;
            case RES_STATUS_ERR_NOMEM:
                printf("Server out of memory\n");
                break;
            case RES_STATUS_ERR_FULL:
                printf("Server storage full\n");
                break;
            case RES_STATUS_ERR_INVALID_REQ:
                printf("Invalid request\n");
                break;
            case RES_STATUS_ERR_INTERNAL:
                printf("Internal server error\n");
                break;
            default:
                printf("Unknown error (%u)\n", res.status);
                break;
        }
    }
}

int
main(int argc, char *argv[]) {
    char *client_ip = "127.0.0.1";
    char *server_host = "localhost";
    char *operation, *key, *value = NULL;
    int opt;
    
    // parse command line options
    while ((opt = getopt(argc, argv, "c:h:")) != -1) {
        switch (opt) {
            case 'c':
                client_ip = optarg;
                break;
            case 'h':
                server_host = optarg;
                break;
            default:
                print_usage(argv[0]);
        }
    }
    
    // check remaining arguments
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing operation\n");
        print_usage(argv[0]);
    }
    
    operation = argv[optind++];
    
    if (optind >= argc) {
        fprintf(stderr, "Error: Missing key\n");
        print_usage(argv[0]);
    }
    
    key = argv[optind++];
    
    // check if operation needs value
    if (strcmp(operation, "SET") == 0) {
        if (optind >= argc) {
            fprintf(stderr, "Error: SET operation requires a value\n");
            print_usage(argv[0]);
        }
        value = argv[optind++];
    }
    
    // Validate operation
    if (strcmp(operation, "GET") != 0 && 
        strcmp(operation, "SET") != 0 && 
        strcmp(operation, "DELETE") != 0) {
        fprintf(stderr, "Error: Invalid operation '%s'. Use GET, SET, or DELETE\n", operation);
        print_usage(argv[0]);
    }
    
    // connecting to the server with socket API (not inetConnect) so we can define
    // the client IP in case that option was specified (-c client_ip)
    struct sockaddr_storage client_addr;
    memset(&client_addr, 0, sizeof(client_addr));

    // try IPv4 first
    struct sockaddr_in *addr4 = (struct sockaddr_in *) &client_addr;
    if (inet_pton(AF_INET, client_ip, &addr4->sin_addr) == 1) {
        addr4->sin_family = AF_INET;
        addr4->sin_port = 0;
        client_addr.ss_family = AF_INET;
    } else {
        // try IPv6
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&client_addr;
        if (inet_pton(AF_INET6, client_ip, &addr6->sin6_addr) == 1) {
            addr6->sin6_family = AF_INET6;
            addr6->sin6_port = 0;
            client_addr.ss_family = AF_INET6;
        } else {
            errExit("Invalid IP address");
        }
    }

    int cfd = socket(client_addr.ss_family, SOCK_STREAM, 0);
    if (cfd == -1)
        errExit("socket");

    socklen_t client_len;
    if (client_addr.ss_family == AF_INET) {
        client_len = sizeof(struct sockaddr_in);
    } else {
        client_len = sizeof(struct sockaddr_in6);
    }

    // bind with specific IP
    if (bind(cfd, (struct sockaddr *) &client_addr, client_len) == -1)
        errExit("bind");
    
    // resolve server address
    struct addrinfo hints, *server_info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = client_addr.ss_family;  // Match client family (IPv4/IPv6)
    hints.ai_socktype = SOCK_STREAM;
    
    int gai_result = getaddrinfo(server_host, PORT_NUM, &hints, &server_info);
    if (gai_result != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(gai_result));
        exit(EXIT_FAILURE);
    }
    
    // connect to server
    if (connect(cfd, server_info->ai_addr, server_info->ai_addrlen) == -1)
        errExit("connect");
    
    freeaddrinfo(server_info);
    
    // compose and send request
    struct request_hdr req_hdr;
    
    // determine operation code
    if (strcmp(operation, "GET") == 0) {
        req_hdr.opcode = htonl(OP_GET);
    } else if (strcmp(operation, "SET") == 0) {
        req_hdr.opcode = htonl(OP_SET);
    } else if (strcmp(operation, "DELETE") == 0) {
        req_hdr.opcode = htonl(OP_DELETE);
    } else {
        // should never happen due to earlier validation
        errExit("Invalid operation (internal error)");
    }
    
    req_hdr.key_len = htonl(strlen(key));
    req_hdr.value_len = htonl(value ? strlen(value) : 0);
    
    // send request header
    if (write(cfd, &req_hdr, sizeof(req_hdr)) != sizeof(req_hdr))
        errExit("write request header");
    
    // send key
    if (write(cfd, key, strlen(key)) != (ssize_t)strlen(key))
        errExit("write key");
    
    // send value if SET operation
    if (value && write(cfd, value, strlen(value)) != (ssize_t) strlen(value))
        errExit("write value");
    
    // handle server response
    handle_response(cfd, operation);
    
    close(cfd);
    return 0;
}
