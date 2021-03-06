#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "network.h"
#include "socket_utils.h"
#include "fifo_utils.h"

/******************************
 * Utility functions
 ******************************/


/******************************
 * Interface implementation
 ******************************/
Address AddressCreate(const char* ip, int port) {
    if (strlen(ip) != MAX_IP_LENGTH) {
        printf("IP length has to be exactly 16 digits.\n");
        return NULL;
    }
	Address result = (Address)malloc(sizeof(*result));
	if (NULL == result) return NULL;

    result->addr = (char*)malloc(strlen(ip) + 1); // TODO: change to our strlen.
    if (NULL == result->addr) {
        free(result);
        return NULL;
    }

    strcpy(result->addr, ip);
    result->port = port;

    return result;
}

void AddressDestroy(Address address) {
    free(address->addr);
    free(address);
}


static int socket_count = 0;

SocketID SocketCreate() {
    SocketID sock_id = (SocketID) malloc(sizeof(*sock_id));
    if (sock_id == NULL) {
        return NULL;
    }

    init_empty_socket_id(sock_id);
    return sock_id; 
}

void SocketDestroy(SocketID sockid) {
	free(sockid);
}

Status SocketClose(SocketID sockid) {
    char* end_fifo_write_name = get_end_fifo_write_end_name(sockid);
    if (NULL == end_fifo_write_name) return MEMORY_ERROR;

    char* end_fifo_read_name = get_end_fifo_read_end_name(sockid);
    if (NULL == end_fifo_read_name) {
        free(end_fifo_write_name);
        return MEMORY_ERROR;
    }

    // this might get stuck
    int end_fifo_read_fd = open(end_fifo_read_name, O_RDONLY);
    close(end_fifo_read_fd);

    int end_fifo_write_fd = open(end_fifo_write_name, O_WRONLY);
    if (end_fifo_write_fd == -1) {
        free(end_fifo_write_name);
        free(end_fifo_read_name);
        return MEMORY_ERROR;
    }

    if (write(end_fifo_write_fd, "E", 1) < 1) {
        free(end_fifo_write_name);
        free(end_fifo_read_name);
        close(end_fifo_write_fd);
        return MEMORY_ERROR;
    }

    close(end_fifo_write_fd);
    free(end_fifo_write_name);

    unlink(end_fifo_read_name);
    free(end_fifo_read_name);
    
    return SUCCESS;
}


Status SocketBind(SocketID sockid, Address addrport) {
    if (NULL == sockid || NULL == addrport) return INVALID_ARGUMENT;
    if (NULL == addrport->addr) return INVALID_ARGUMENT;
    if (get_socket_state(sockid) != EMPTY_SOCKET) return INVALID_ARGUMENT;

    char* bind_request_fifo_name = get_bind_requests_fifo_name(addrport->addr);
    if (NULL == bind_request_fifo_name) return MEMORY_ERROR;

    char* my_fifo_name = get_client_fifo_name(getpid(), ++socket_count);
    if (NULL == my_fifo_name) {
        free(bind_request_fifo_name);
        return MEMORY_ERROR;
    }

    int bind_request_fifo_fd = open(bind_request_fifo_name, O_WRONLY);
    if (-1 == bind_request_fifo_fd) {
        printf("Failed opening fifo %s.\n\terrno: %d.\n", bind_request_fifo_name, errno);
        free(bind_request_fifo_name); free(my_fifo_name);
        return MEMORY_ERROR;
    }

    if (mkfifo(my_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            printf("Failed creating fifos (probably already exist - will try to delete them automatically).\n");
            free(bind_request_fifo_name); free(my_fifo_name);
            return -1;
    }

    int my_fifo_fd = open(my_fifo_name, O_RDONLY | O_NONBLOCK);
    if (-1 == my_fifo_fd) {
        printf("Failed opening fifo %s.\n\terrno: %d.\n", my_fifo_name, errno);
        unlink(my_fifo_name);
        close(bind_request_fifo_fd);
        free(bind_request_fifo_name); free(my_fifo_name);
        return MEMORY_ERROR;
    }

    // all fifos are open - send request and await answer.
    char message[MAX_SOCKET_STRING_REPR_SIZE];
    if (sprintf(message, "%d_%d_%d%c", addrport->port, getpid(), socket_count, '\0') < 0) {
        close(my_fifo_fd); close(bind_request_fifo_fd);
        unlink(my_fifo_name);
        free(bind_request_fifo_name); free(my_fifo_name);
        return MEMORY_ERROR;
    }

    if (-1 == write(bind_request_fifo_fd, message, strlen(message) + 1)) { // TODO: change to our strlen. +1 is for null byte
        close(my_fifo_fd); close(bind_request_fifo_fd);
        unlink(my_fifo_name);
        free(bind_request_fifo_name); free(my_fifo_name);
        return MEMORY_ERROR;
    } 

    close(bind_request_fifo_fd);
    free(bind_request_fifo_name);

    // await answer

    char answer;
    while (1) {
        int read_size = read_entire_message(my_fifo_fd, &answer, sizeof(answer));
        if (-1 == read_size) {
            close(my_fifo_fd);
            unlink(my_fifo_name);
            free(my_fifo_name);
            return MEMORY_ERROR;
        }

        if (1 == read_size) {
            close(my_fifo_fd);
            unlink(my_fifo_name);
            free(my_fifo_name);
            break;
        }

        if (1 < read_size) {
            printf("read size > 1 - should not happen (debug.\n");
            close(my_fifo_fd);
            unlink(my_fifo_name);
            free(my_fifo_name);
        }
    }

    if (answer == REQUEST_GRANTED_FIFO) {
        strcpy(sockid->src_ip, addrport->addr); // TODO: change to our strcpy
        sockid->src_port = addrport->port;
        return SUCCESS;
    } else {
        return REQUEST_DENIED;
    }
}

Status SocketListen(SocketID sockid, int queueLimit) {
    if (queueLimit < 0 || queueLimit > 100) {
        printf("Queue limit for listening sockets should be between 0 and 100.\n");
        return INVALID_ARGUMENT;
    }
     if (NULL == sockid) return INVALID_ARGUMENT;
     if (get_socket_state(sockid) != BOUND_ONLY_SOCKET) {
        printf("Socket has to be bound for listening.\n");
        return INVALID_ARGUMENT;
     }

    char* listen_fifo_write_end_name = get_listen_fifo_write_end_name(sockid);
    char* listen_fifo_read_end_name = get_listen_fifo_read_end_name(sockid);

    if (NULL == listen_fifo_read_end_name || NULL == listen_fifo_write_end_name) {
        free(listen_fifo_write_end_name);
        free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    if (0 != mkfifo(listen_fifo_write_end_name, DEFAULT_FIFO_MODE)
        || 0 != mkfifo(listen_fifo_read_end_name, DEFAULT_FIFO_MODE)) {
            free(listen_fifo_write_end_name);
            free(listen_fifo_read_end_name);
            return MEMORY_ERROR;
        }


    int listen_fifo_read_fd = open(listen_fifo_read_end_name, O_RDONLY);
    if (-1 == listen_fifo_read_fd) {
        unlink(listen_fifo_read_end_name);
        unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name);
        free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    int listen_fifo_write_fd;

    listen_fifo_write_fd = open(listen_fifo_write_end_name, O_WRONLY);
    if (-1 == listen_fifo_write_fd) {
        close(listen_fifo_read_fd);
        unlink(listen_fifo_read_end_name);
        unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name);
        free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    // // all fifos are open - send request and await answer.
    char message = (char)queueLimit;
    if (1 != write(listen_fifo_write_fd, &message, 1)) {
        close(listen_fifo_write_fd); close(listen_fifo_read_fd);
        unlink(listen_fifo_read_end_name); unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name); free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }
    close(listen_fifo_write_fd);
    // await answer

    char answer;


    clock_t start = clock();

    while (1) {
        int read_size = read_entire_message(listen_fifo_read_fd, &answer, sizeof(answer));
        if (-1 == read_size) {
            close(listen_fifo_read_fd);
            unlink(listen_fifo_read_end_name);
            free(listen_fifo_read_end_name);

            unlink(listen_fifo_write_end_name);
            free(listen_fifo_write_end_name);
            return MEMORY_ERROR;
        }

        if (1 == read_size) {
            close(listen_fifo_read_fd);
            unlink(listen_fifo_read_end_name);
            free(listen_fifo_read_end_name);

            unlink(listen_fifo_write_end_name);
            free(listen_fifo_write_end_name);
            break;
        }

        clock_t current = clock();
        double time_passed = DIFF2SEC(current - start);
        // printf("clock: %f.\n", time_passed);
        if (time_passed > SOCKET_TIMEOUT) {
            close(listen_fifo_read_fd);
            unlink(listen_fifo_read_end_name);
            free(listen_fifo_read_end_name);

            unlink(listen_fifo_write_end_name);
            free(listen_fifo_write_end_name);
            return TIMED_OUT;
        }
    
    }


    Status return_value;
    if (answer == REQUEST_GRANTED_FIFO) {
        return_value = SUCCESS;
    } else {
        return_value = REQUEST_DENIED;
    }

    return return_value;
}

Status SocketConnect(SocketID sockid, Address foreignAddr) {
    if (NULL == sockid || foreignAddr == NULL) return INVALID_ARGUMENT;
    if (foreignAddr->addr == NULL) return INVALID_ARGUMENT;

    if (get_socket_state(sockid) != BOUND_ONLY_SOCKET) {
        printf("Socket has to be bound for connecting.\n");
        return INVALID_ARGUMENT;
     }

    char* connect_fifo_write_name = get_connect_fifo_write_end_name(sockid);
    char* connect_fifo_read_name = get_connect_fifo_read_end_name(sockid);

    if (NULL == connect_fifo_read_name || NULL == connect_fifo_write_name) {
        free(connect_fifo_write_name);
        free(connect_fifo_read_name);
        return MEMORY_ERROR;
    }

    if (0 != mkfifo(connect_fifo_write_name, DEFAULT_FIFO_MODE)
        || 0 != mkfifo(connect_fifo_read_name, DEFAULT_FIFO_MODE)) {
            free(connect_fifo_write_name);
            free(connect_fifo_read_name);
            return MEMORY_ERROR;
        }

    int connect_fifo_read_fd = open(connect_fifo_read_name, O_RDONLY);
    if (-1 == connect_fifo_read_fd) {
        unlink(connect_fifo_read_name);
        unlink(connect_fifo_write_name);
        free(connect_fifo_write_name);
        free(connect_fifo_read_name);
        return MEMORY_ERROR;
    }

    int connect_fifo_write_fd = open(connect_fifo_write_name, O_WRONLY);
    if (-1 == connect_fifo_write_fd) {
        close(connect_fifo_read_fd);
        unlink(connect_fifo_read_name);
        unlink(connect_fifo_write_name);
        free(connect_fifo_write_name);
        free(connect_fifo_read_name);
        return MEMORY_ERROR;
    }


    // // all fifos are open - send request and await answer.
    char message[MAX_SOCKET_STRING_REPR_SIZE];
    if (sprintf(message, "%s_%d%c", foreignAddr->addr, foreignAddr->port, '\0') < 3) {
        close(connect_fifo_read_fd);
        close(connect_fifo_write_fd);
        unlink(connect_fifo_read_name);
        unlink(connect_fifo_write_name);
        free(connect_fifo_write_name);
        free(connect_fifo_read_name);
        return MEMORY_ERROR;
    }

    if (-1 == write(connect_fifo_write_fd, message, strlen(message) + 1)) { // TODO: change to our strlen
        close(connect_fifo_read_fd);
        close(connect_fifo_write_fd);
        unlink(connect_fifo_read_name);
        unlink(connect_fifo_write_name);
        free(connect_fifo_write_name);
        free(connect_fifo_read_name);
        return MEMORY_ERROR;
    }

    // // await answer

    char answer;

    clock_t start = clock();

        while (1) {
            int read_size = read_entire_message(connect_fifo_read_fd, &answer, sizeof(answer));

            if (-1 == read_size) {
                close(connect_fifo_read_fd);
                unlink(connect_fifo_read_name);
                free(connect_fifo_read_name);

                close(connect_fifo_write_fd);
                unlink(connect_fifo_write_name);
                free(connect_fifo_write_name);
                return MEMORY_ERROR;
            }

            if (1 == read_size) {
                close(connect_fifo_read_fd);
                unlink(connect_fifo_read_name);
                free(connect_fifo_read_name);

                close(connect_fifo_write_fd);
                unlink(connect_fifo_write_name);
                free(connect_fifo_write_name);
                break;
            }

            clock_t current = clock();
            double time_passed = DIFF2SEC(current - start);
            // printf("clock: %f.\n", time_passed);
            if (time_passed > SOCKET_TIMEOUT) {
                close(connect_fifo_read_fd);
                unlink(connect_fifo_read_name);
                free(connect_fifo_read_name);

                close(connect_fifo_write_fd);
                unlink(connect_fifo_write_name);
                free(connect_fifo_write_name);
                return TIMED_OUT;
            }
    
        }

        Status return_value;
        if (answer == REQUEST_GRANTED_FIFO) {

            char* end_fifo_read_name = get_end_fifo_read_end_name(sockid);
            char* end_fifo_write_name = get_end_fifo_write_end_name(sockid);

            if (NULL ==end_fifo_read_name
                || 0 != unlink(end_fifo_read_name)) {}
                // printf("Failed to unlink end fifo read name (delete manually).\n");

            if (NULL ==end_fifo_write_name
                || 0 != unlink(end_fifo_write_name)) {}
                // printf("Failed to unlink end fifo write name (delete manually).\n");

            free(end_fifo_read_name);
            free(end_fifo_write_name);

            return_value = SUCCESS;
            strcpy(sockid->dst_ip, foreignAddr->addr);
            sockid->dst_port = foreignAddr->port;

            char* send_fifo_name = get_socket_send_fifo_name(sockid);
            if (NULL == send_fifo_name) return MEMORY_ERROR;

            char* recv_fifo_name = get_socket_recv_fifo_name(sockid);
            if (NULL == recv_fifo_name) {
                free(send_fifo_name);
                return MEMORY_ERROR;
            }

            if (0 != mkfifo(send_fifo_name, DEFAULT_FIFO_MODE)
                || 0 != mkfifo(recv_fifo_name, DEFAULT_FIFO_MODE)) {
                unlink(send_fifo_name);
                unlink(recv_fifo_name);

                free(recv_fifo_name);
                free(send_fifo_name);
                ip_set_empty(sockid->dst_ip);
                sockid->dst_port = EMPTY_PORT;
                create_socket_end_fifos(sockid);
                return MEMORY_ERROR;
            }
            free(send_fifo_name);
            free(recv_fifo_name);

        } else {
            return_value = REQUEST_DENIED;
        }

        return return_value;
}


SocketID SocketAccept(SocketID sockid) {
    if (NULL == sockid) return ILLEGAL_SOCKET_ID;

    if (get_socket_state(sockid) != BOUND_ONLY_SOCKET) {
        printf("Socket has to be bound and listening for accepting connections.\n");
        return ILLEGAL_SOCKET_ID;
     }

    char* accept_fifo_name = get_accept_fifo_write_end_name(sockid);
    if (NULL == accept_fifo_name) return ILLEGAL_SOCKET_ID;


    char message[MAX_SOCKET_STRING_REPR_SIZE];

    clock_t start = clock();

    int accept_fifo_fd = -1;

    while (1) {

        clock_t current = clock();
        double time_passed = DIFF2SEC(current - start);
        // printf("clock: %f.\n", time_passed);
        if (time_passed > SOCKET_TIMEOUT) {
            if (accept_fifo_fd != -1) close(accept_fifo_fd);
            free(accept_fifo_name);
            printf("Accept request timed out.\n");
            return NULL;
        }

        if (-1 == accept_fifo_fd) {
            accept_fifo_fd = open(accept_fifo_name, O_RDONLY | O_NONBLOCK);
        } else {

            int read_length = read_message_until_char(accept_fifo_fd, message, '\0');

            if (-1 == read_length) {
                free(accept_fifo_name);
                close(accept_fifo_fd);

                return ILLEGAL_SOCKET_ID;
            } else if (read_length > 0) {
                free(accept_fifo_name);
                close(accept_fifo_fd);
                break;
            }
        }
    }


    char ip[MAX_IP_LENGTH + 1];
    int port;

    int sscanf_result_1, sscanf_result_2;
    bool underscore_found = false;
    for (int i = 0; i < strlen(message); ++i) {
        if (message[i] == '_') {
            underscore_found = true;
            message[i] = '\0';

            sscanf_result_1 = sscanf(message + i + 1, "%d", &port);
            sscanf_result_2 = sscanf(message, "%s", ip);
            break;
        }
    }

    if (!underscore_found || sscanf_result_1 != 1 || sscanf_result_2 != 1) {
        return ILLEGAL_SOCKET_ID;
    }

    SocketID new_connection = copy_socket_id(sockid);
    if (NULL == new_connection) return ILLEGAL_SOCKET_ID;

    strcpy(new_connection->dst_ip, ip);
    new_connection->dst_port = port;


    char* send_fifo_name = get_socket_send_fifo_name(new_connection);
    if (NULL == send_fifo_name) return ILLEGAL_SOCKET_ID;
    char* recv_fifo_name = get_socket_recv_fifo_name(new_connection);
    if (NULL == recv_fifo_name) {
        free(send_fifo_name);
        return ILLEGAL_SOCKET_ID;
    }

    if (0 != mkfifo(send_fifo_name, DEFAULT_FIFO_MODE)
        || 0 != mkfifo(recv_fifo_name, DEFAULT_FIFO_MODE)) {
        unlink(send_fifo_name);
        unlink(recv_fifo_name);

        free(recv_fifo_name);
        free(send_fifo_name);
        ip_set_empty(new_connection->dst_ip);
        new_connection->dst_port = EMPTY_PORT;
        return ILLEGAL_SOCKET_ID;
    }
    free(send_fifo_name);
    free(recv_fifo_name);

    return new_connection;
}

int SocketSend(SocketID sockid, char* message, int len) {

    if (NULL == sockid || NULL == message) return -1;

    if (get_socket_state(sockid) != CONNECTED_SOCKET) {
        printf("Socket must be connected before sending data.\n");
        return -1;
    }

    char* send_fifo_name = get_socket_send_fifo_name(sockid);
    if (NULL == send_fifo_name) return -1;

    int send_fifo_fd = open(send_fifo_name, O_RDWR);
    free(send_fifo_name);
    if (-1 == send_fifo_fd) {
        return -1;
    }

    return write(send_fifo_fd, message, len);
}


int SocketRecv(SocketID sockid, char* recvBuffer, int bufferLen) {
    if (NULL == sockid || NULL == recvBuffer || bufferLen <= 0) return -1;

    if (get_socket_state(sockid) != CONNECTED_SOCKET) {
        printf("Socket must be connected before receiving data.\n");
        return -1;
    }


    int recv_fifo_fd = -1;
    int read_length = 0;
    while (0 == read_length) {
        char* recv_fifo_name = get_socket_recv_fifo_name(sockid);
        if (NULL == recv_fifo_name) return -1;

        recv_fifo_fd = open(recv_fifo_name, O_RDONLY);
        if (-1 == recv_fifo_fd) {
            free(recv_fifo_name);
            return -1;
        }

        free(recv_fifo_name);

        read_length = read(recv_fifo_fd, recvBuffer, bufferLen);

        close(recv_fifo_fd);
        if (-1 == read_length) {
            if (EAGAIN == errno) {
                read_length = 0;
                continue;
            }
            return -1;
        }

        // printf("--------\n");
        // for (int i = 0; i < read_length; ++i) {
        //     printf("%d ", recvBuffer[i]);
        // }
        // printf("\n");

        if (recvBuffer[read_length - 1] == CONN_CLOSED) {
            printf("Connection was forcibly closed on the other side.\n");
            return -1;
        }
    }
    return read_length;
}