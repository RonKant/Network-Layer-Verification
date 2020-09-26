#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

/**
 * Utility functions for handling sockets
 */

#include "util_types.h"

#define EMPTY_IP NULL
#define EMPTY_PORT -1

typedef enum {
    CONNECTED_SOCKET, 
    BOUND_ONLY_SOCKET, 
    EMPTY_SOCKET, 
    INVALID_SOCKET
} SocketState;

void init_empty_socket_id(SocketID sock_id);

bool is_socket_connected(SocketID sock_id);

bool is_socket_bound_only(SocketID sock_id);

bool is_socket_empty(SocketID sock_id);

SocketState get_socket_state(SocketID sock_id);

#endif