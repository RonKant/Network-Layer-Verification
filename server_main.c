#include <stdio.h>
#include <stdlib.h>

#include "network.h"

int main() {
    SocketID sock = SocketCreate();
    if (NULL == sock) {
        printf("Failed creating a socket.\n");
        return -1;
    }

    Address addr = AddressCreate("123", 8080);
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
    }


    AddressDestroy(addr);
    SocketDestroy(sock);

    return 0;
}