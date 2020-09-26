#include <stdbool.h>
#include <stdlib.h>

#include "network.h"
#include "socket_utils.h"

/******************************
 * Utility functions
 ******************************/


/******************************
 * Interface implementation
 ******************************/

SocketID SocketCreate() {
    SocketID sock_id = (SocketID) malloc(sizeof(*sock_id));
    if (sock_id == NULL) {
        return NULL;
    }

    init_empty_socket_id(sock_id);
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