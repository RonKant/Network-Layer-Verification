#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "network.h"
#include "socket_utils.h"

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

            sleep(5);
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