#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

/**
 * Utility functions for handling sockets
 */

#include "util_types.h"
#include "network.h"
#include "tcp.h"


#define EMPTY_PORT 0
#define MAX_PORT_STRING_LENGTH
#define SOCKET_TIMEOUT 10
#define SOCKET_SEND_AGAIN_TIME 3
#define SOCKET_FIN_RST_TIME 8
#define SOCKET_TIME_WAIT_TIMEOUT 1
#define MAX_DATA_PER_PACKET 128

#define DIFF2SEC(DIFF) ((double)(DIFF)) / CLOCKS_PER_SEC

bool is_empty_ip(char* ip);

void ip_set_empty(char* ip);

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

SocketID copy_socket_id(SocketID sock_id);

/* in these functions void* is socket (this is done this way for passing them to queue). */
void* copy_socket(void* to_copy);

bool compare_socket(void* s1, void* s2);

/**
 * Creates an empty - most generic state, socket.
 */
Socket create_new_socket();

/**
 * Frees all memory of socket
 */
void destroy_socket(void* socket);

void destroy_socket_fifos(Socket socket);

void destroy_socket_id(SocketID sock_id); // also frees ip strings

/**
 * Moves the receive window back by amount of received sequence of bytes.
 * Transfers said bytes to user fifo.
 */
void update_recv_window(Socket socket);

#endif