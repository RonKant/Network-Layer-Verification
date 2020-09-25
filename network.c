#include <stdbool.h>
#include <stdlib.h>

#include "network.h"

#define EMPTY_IP NULL
#define EMPTY_PORT -1

/******************************
 * Utility functions
 ******************************/

typedef enum {
    CONNECTED_SOCKET, 
    BOUND_ONLY_SOCKET, 
    EMPTY_SOCKET, 
    INVALID_SOCKET
} SocketState;

void init_empty_socket(SocketID sock_id) {
    sock_id->dst_ip = EMPTY_IP;
    sock_id->src_ip = EMPTY_IP;
    sock_id->dst_port = EMPTY_PORT;
    sock_id->src_port = EMPTY_PORT;
}

bool is_socket_connected(SocketID sock_id) {
    return (sock_id->dst_ip != EMPTY_IP
            && sock_id->dst_port != EMPTY_PORT
            && sock_id->src_ip != EMPTY_IP
            && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_bound_only(SocketID sock_id) {
    return (sock_id->dst_ip == EMPTY_IP
        && sock_id->dst_port == EMPTY_PORT
        && sock_id->src_ip != EMPTY_IP
        && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_empty(SocketID sock_id) {
    return (sock_id->dst_ip == EMPTY_IP
        && sock_id->dst_port == EMPTY_PORT
        && sock_id->src_ip == EMPTY_IP
        && sock_id->src_port == EMPTY_PORT);
}

SocketState get_socket_state(SocketID sock_id) {
    if (is_socket_connected(sock_id)) return CONNECTED_SOCKET;
    if (is_socket_bound_only(sock_id)) return BOUND_ONLY_SOCKET;
    if (is_socket_empty(sock_id)) return EMPTY_SOCKET;
    return INVALID_SOCKET;
}


/******************************
 * Interface implementation
 ******************************/

SocketID SocketCreate() {
    SocketID sock_id = (SocketID) malloc(sizeof(*sock_id));
    if (sock_id == NULL) {
        return NULL;
    }

    init_empty_socket(sock_id);
    return sock_id; 
}


Status SocketClose(SocketID sockid);


Status SocketBind(SocketID sockid, Address addrport);

Status SocketListen(SocketID sockid, int queueLimit);

Status SocketConnect(SocketID sockid, Address foreignAddr); // TODO: in our implementation one has to bind before connecting (so we don't randomize client port)
															// distinct (by enum) a socket bound+LISTEN and a socket bound+nothing.


SocketID SocketAccept(SocketID sockid, Address clientAddr);

int SocketSend(SocketID sockid, char* message);


int SocketRecv(SocketID sockid, char* recvBuffer, int bufferLen);