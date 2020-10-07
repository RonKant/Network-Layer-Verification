#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network.h"
#include "socket_utils.h"

#define MAX_MESSAGE_LENGTH 100

void chat(SocketID sock_id) {
    char message[MAX_MESSAGE_LENGTH + 1];
    printf("Start chatting.\n");

    while (1) {
        if (NULL != fgets(message, MAX_MESSAGE_LENGTH, stdin)) {

            if (message[strlen(message) - 1] == '\n') {
                message[strlen(message) - 1] = '\0';
            }

            if (-1 == SocketSend(sock_id, message, strlen(message))) {
                printf("Error while sending data.\n");
                break;
            }

            if (! strcmp("exit", message)) {
                printf("Done chatting.\n");
                break;
            }
        }
    }
}

int main() {
    SocketID sock = SocketCreate();
    if (NULL == sock) {
        printf("Failed creating a socket.\n");
        return -1;
    }

    Address addr = AddressCreate("1234123412341234", 8081);
    if (NULL == addr) {
        printf("Failed allocating address object.\n");
        free(sock);
        return -1;
    }

    Address server_addr = AddressCreate("1234123412341234", 8080);
    if (NULL == addr) {
        printf("Failed allocating address object.\n");
        AddressDestroy(addr);
        free(sock);
        return -1;
    }    

    Status status = SocketBind(sock, addr);
    if (status != SUCCESS) {
        printf("Failed binding socket.\n");
    } else {
        printf("Bind successful.\n");

        Status connect_status = SocketConnect(sock, server_addr);
        if (connect_status != SUCCESS) {
            printf("Failed connecting to remote address.\n");
        } else {
            printf("Connected succesfully.\n");
            chat(sock);
        }

        if (SUCCESS != SocketClose(sock)) {
            printf("Failed closing new connection socket.\n");
        } else {
            printf("New connection socket closed manually.\n");
        }        
    }


    AddressDestroy(server_addr);
    AddressDestroy(addr);
    SocketDestroy(sock);

    return 0;
}