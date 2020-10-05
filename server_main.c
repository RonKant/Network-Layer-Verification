#include <stdio.h>
#include <stdlib.h>

#include "network.h"
#include "socket_utils.h"

int main() {
    SocketID sock = SocketCreate();
    if (NULL == sock) {
        printf("Failed creating a socket.\n");
        return -1;
    }

    Address addr = AddressCreate("1234123412341234", 8080);
    if (NULL == addr) {
        printf("Failed allocating address object.\n");
        free(sock);
        return -1;
    }

    Status status = SocketBind(sock, addr);
    if (status != SUCCESS) {
        printf("Failed binding socket.\n");
    } else {
        printf("Bind successful.\n");

        Status status = SocketListen(sock, 5);
        if (status != SUCCESS) {
            printf("Failed listening socket.\n");
        } else {
            printf("Listen successful.\n");
            SocketID new_connection = SocketAccept(sock);
            if (NULL == new_connection) {
                printf("Failed accepting a new connection.\n");
            } else {
                printf("Received a new connection from (%s, %d)\n", new_connection->dst_ip, new_connection->dst_port);
                destroy_socket_id(new_connection);
            }
        }
    }


    AddressDestroy(addr);
    SocketDestroy(sock);

    return 0;
}