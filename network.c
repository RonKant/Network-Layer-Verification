#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
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
	free(sockid->src_ip);
	free(sockid->dst_ip);
	free(sockid);
}

Status SocketClose(SocketID sockid);


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

    int bind_request_fifo_fd = open(bind_request_fifo_name, O_RDWR);
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
    }

    if (answer == REQUEST_GRANTED_FIFO) {
        sockid->src_ip = (char*)malloc(strlen(addrport->addr) + 1); // TODO: change to our strlen
        if (NULL == sockid->src_ip) return MEMORY_ERROR;
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

    mkfifo(listen_fifo_write_end_name, DEFAULT_FIFO_MODE);
    mkfifo(listen_fifo_read_end_name, DEFAULT_FIFO_MODE);

    int listen_fifo_write_fd = open(listen_fifo_write_end_name, O_RDWR);
    if (-1 == listen_fifo_write_fd) {
        unlink(listen_fifo_read_end_name);
        unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name);
        free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    int listen_fifo_read_fd = open(listen_fifo_read_end_name, O_RDONLY | O_NONBLOCK);
    if (-1 == listen_fifo_read_fd) {
        close(listen_fifo_write_fd);
        unlink(listen_fifo_read_end_name);
        unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name);
        free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    // // all fifos are open - send request and await answer.
    char message = (char)queueLimit;

    if (-1 == write(listen_fifo_write_fd, &message, 1)) {
        close(listen_fifo_write_fd); close(listen_fifo_read_fd);
        unlink(listen_fifo_read_end_name); unlink(listen_fifo_write_end_name);
        free(listen_fifo_write_end_name); free(listen_fifo_read_end_name);
        return MEMORY_ERROR;
    }

    close(listen_fifo_write_fd);
    // await answer

    char answer;
    while (1) {

        int read_size = read_entire_message(listen_fifo_read_fd, &answer, sizeof(answer));
        if (-1 == read_size) {

            close(listen_fifo_read_fd);
            unlink(listen_fifo_read_end_name);
            free(listen_fifo_read_end_name);
            return MEMORY_ERROR;
        }

        if (1 == read_size) {

            close(listen_fifo_read_fd);
            unlink(listen_fifo_read_end_name);
            free(listen_fifo_read_end_name);
            break;
        }
    }

    Status return_value;
    if (answer == REQUEST_GRANTED_FIFO) {
        return_value = SUCCESS;
    } else {
        return_value = REQUEST_DENIED;
    }

    close(listen_fifo_write_fd);
    unlink(listen_fifo_write_end_name);
    free(listen_fifo_write_end_name);

    return return_value;
}

Status SocketConnect(SocketID sockid, Address foreignAddr); // TODO: in our implementation one has to bind before connecting (so we don't randomize client port)
															// distinct (by enum) a socket bound+LISTEN and a socket bound+nothing.


SocketID SocketAccept(SocketID sockid, Address clientAddr);

int SocketSend(SocketID sockid, char* message);


int SocketRecv(SocketID sockid, char* recvBuffer, int bufferLen);