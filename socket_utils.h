#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

/**
 * Utility functions for handling sockets
 */

#include "util_types.h"

#define EMPTY_IP NULL
#define EMPTY_PORT -1

void init_empty_socket_id(SocketID sock_id);

bool is_socket_connected(SocketID sock_id);

bool is_socket_bound_only(SocketID sock_id);

bool is_socket_empty(SocketID sock_id);

SocketState get_socket_state(SocketID sock_id);

/**
 * Create a new socket with given sock_id that is bound to an address,
 * with all relevant fields initialized.
 * Returns NULL on error.
 */
Socket create_bound_socket(SocketID sock_id);


/**
 * Creates an empty - most generic state, socket.
 */
Socket create_new_socket();

/**
 * Frees all memory of socket, and unlinks all fifos.
 */
void destroy_socket(Socket socket);

#endif