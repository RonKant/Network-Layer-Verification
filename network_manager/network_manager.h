#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

// #include <sys/types.h>
// #include <sys/stat.h>

#include "../Hashmap.h"

/**
 * This module represents a component which manages the full tcp communication
 * on a single "IP", and is capable of communicating with other network managers and clients via fifo.
 */

typedef struct {
    char* ip; // the IP this component is in charge of
    HashMap sockets; // holds all active "clients" which are using this IP

    // // Used for communication with clients / other managers:
    // char* in_packet_fifo_name;  // always named vnetwork_in_packets_<my_ip>.
    //                             // packets received here will be returned to listening clients.

    // char* bind_request_fifo_name;   // always named vnetwork_bind_requests_<my_ip>.
    // char* connect_request_fifo_name; // always named vnetwork_connect_requests_<my_ip>.
    // char* terminate_fifo_name; // when something goes here - terminate loop.

    int in_packet_fifo_fd;
    int bind_request_fifo_fd;
    int connect_request_fifo_fd;
    int terminate_fifo_fd;
}* NetworkManager;

/**
 * Allocates a new manager of given ip with empty dictionary of clients.
 * Returns NULL on failure.
 */
NetworkManager createNetworkManager(char* ip);

/**
 * De-allocates all memory+resources related to given manager.
 */
void destroyNetworkManager(NetworkManager manager);

/**
 * Enters an "infinte" loop of network management.
 * Until receiving a special stop command, will receive and manage new tcp connections.
 * 
 * When stops, returns 0 if stopped by a command or -1 if stopped by an unexpected failure.
 */
int managerLoop(NetworkManager manager);

/**
 * When activated on a different process - sends the special stop message to a given manager.
 * Returns 0 on success or -1 on failure (manager does not exist or memory failure).
 */
int stopManager(char* ip);

/**
 * Closes all socket resources, and notifies client by fifo.
 */
void close_socket(NetworkManager manager, SocketID sock_id);


#endif // NETWORK_MANAGER_H