#include <stdio.h>
#include <stdlib.h>
#include <string.h> // TODO: implement by ourselves
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "network_manager.h"

#define OUT_PACKET_FIFO_PREFIX "vnetwork_out_packets_"
#define IN_PACKET_FIFO_PREFIX "vnetwork_in_packets_"
#define BIND_REQUEST_FIFO_PREFIX "vnetwork_bind_requests_"
#define CONNECT_REQUEST_FIFO_PREFIX "vnetwork_connect_requests_"

#define MAX_SOCKETS 32768
#define DEFAULT_FIFO_MODE 0666

/**
 * Returns a new string, containing the concatenation of the two strings.
 * Used in translating ip/ports to fifo names.
 */
char* add_as_prefix(const char* prefix, const char* s) {
    char* result = (char*)malloc(strlen(s) + strlen(prefix) + 1);
    if (result == NULL) return NULL;
    result[0] = '\0';

    result = strcat(result, prefix);
    result = strcat(result, s);

    return result;
}

char* get_out_packet_fifo_name(const char* ip) {
    return add_as_prefix(OUT_PACKET_FIFO_PREFIX, ip);
}

char* get_in_packet_fifo_name(const char* ip) {
    return add_as_prefix(IN_PACKET_FIFO_PREFIX, ip);
}

char* get_bind_requests_fifo_name(const char* ip) {
    return add_as_prefix(BIND_REQUEST_FIFO_PREFIX, ip);
}

char* get_connect_requests_fifo_name(const char* ip) {
    return add_as_prefix(CONNECT_REQUEST_FIFO_PREFIX, ip);
}

int initialize_network_manager_fifos(NetworkManager manager) {
    manager->out_packet_fifo_name = get_out_packet_fifo_name(manager->ip);
    manager->in_packet_fifo_name = get_in_packet_fifo_name(manager->ip);
    manager->bind_request_fifo_name = get_bind_requests_fifo_name(manager->ip);
    manager->connect_request_fifo_name = get_connect_requests_fifo_name(manager->ip);

    if (manager->out_packet_fifo_name == NULL
        || manager->in_packet_fifo_name == NULL
        || manager->in_packet_fifo_name == NULL
        || manager->connect_request_fifo_name == NULL) {
            return -1;
        }

    if (mkfifo(manager->out_packet_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->in_packet_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->bind_request_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(manager->connect_request_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            return -1;
        }

    return 0;
}

NetworkManager createNetworkManager(const char* ip) {
    NetworkManager manager = (NetworkManager)malloc(sizeof(*manager));
    if (manager == NULL) return NULL;

    // so we can destroy them later if something fails.
    manager->out_packet_fifo_name = NULL;
    manager->in_packet_fifo_name = NULL;
    manager->bind_request_fifo_name = NULL;
    manager->connect_request_fifo_name = NULL;
    manager->sockets = NULL;

    manager->ip = ip;

    manager->out_packet_fifo_name = get_out_packet_fifo_name(manager->ip);
    manager->in_packet_fifo_name = get_in_packet_fifo_name(manager->ip);
    manager->bind_request_fifo_name = get_bind_requests_fifo_name(manager->ip);
    manager->connect_request_fifo_name = get_connect_requests_fifo_name(manager->ip);

    if (initialize_network_manager_fifos(manager) != 0) {
            destroyNetworkManager(manager);
            return NULL;
        }

    manager->sockets = createHashMap(MAX_SOCKETS);
    if (manager->sockets == NULL) {
        destroyNetworkManager(manager);
        return NULL;
    }
    return manager;
}


void destroyNetworkManager(NetworkManager manager) {
    // should: free all memory in struct, unlink all fifos (?)
    // if clients exist - notify them about shutdown?

    hashDestroy(manager->sockets);
    unlink(manager->out_packet_fifo_name);
    unlink(manager->in_packet_fifo_name);
    unlink(manager->bind_request_fifo_name);
    unlink(manager->connect_request_fifo_name);

    free(manager->out_packet_fifo_name);
    free(manager->in_packet_fifo_name);
    free(manager->bind_request_fifo_name);
    free(manager->connect_request_fifo_name);

    free(manager);
}

int managerLoop(NetworkManager manager) { 
    return 0; 
} // mock

int stopManager(char* ip);