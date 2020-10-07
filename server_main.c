#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"
#include "socket_utils.h"

#define RECV_BUFFER_LENGTH 100

void chat(SocketID sock_id) {
    char recv_buffer[RECV_BUFFER_LENGTH + 1];
    printf("Start chatting.\n");

    int received_length;
    while(1) {
        received_length = SocketRecv(sock_id, recv_buffer, RECV_BUFFER_LENGTH);
        if (-1 == received_length) {
            printf("Error while receiving data.\n");
            break;
        }
        recv_buffer[received_length] = '\0';

        printf("Message:\n\tLength: %ld.\n\t%s\n", strlen(recv_buffer), recv_buffer);
    }
}

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
                
                chat(new_connection);
                
                if (SUCCESS != SocketClose(new_connection)) {
                    printf("Failed closing new connection socket.\n");
                } else {
                    printf("New connection socket closed manually.\n");
                }
                destroy_socket_id(new_connection);
            }
        }
        if (SUCCESS != SocketClose(sock)) {
            printf("Failed closing listener socket.\n");
        } else {
            printf("Listener socket closed manually.\n");
        } 
    }

    AddressDestroy(addr);
    SocketDestroy(sock);

    return 0;
}