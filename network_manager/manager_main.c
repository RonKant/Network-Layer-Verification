#include <stdio.h>

#include "network_manager.h"
/** 
 * Creates and runs a manager on an ip provided by user arg.
 */

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Only argument should be ip.\n");
        return -1;
    }

    char* ip = argv[1];
    NetworkManager manager = createNetworkManager(ip);
    if (manager == NULL) {
        printf("Failed creating a manager.\n");
        return -1;
    }

    printf("Running manager on ip: %s.\n", manager->ip);
    int loop_result = managerLoop(manager);

    if (loop_result == 0) {
        printf("Manager was stopped manually.\n");
    } else {
        printf("Manager has stopped unexpectedly: an error has occured.\n");
    }

    destroyNetworkManager(manager);
    return loop_result;
}