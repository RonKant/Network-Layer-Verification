/**
 * This module contains functions for obtaining fifo names out of network manager IPs and SocketID obejcts.
 */

#include <stdlib.h>
#include <string.h>

#define TERMINATE_FIFO_PREFIX "vnetwork_terminate_"
#define IN_PACKET_FIFO_PREFIX "vnetwork_in_packets_"
#define BIND_REQUEST_FIFO_PREFIX "vnetwork_bind_requests_"
#define CONNECT_REQUEST_FIFO_PREFIX "vnetwork_connect_requests_"

#define LISTEN_SOCKET_READ_FIFO_PREFIX "vnetwork_listen_socket_read_"
#define LISTEN_SOCKET_WRITE_FIFO_PREFIX "vnetwork_listen_socket_write_"
#define ACCEPT_SOCKET_READ_FIFO_PREFIX "vnetwork_accept_socket_read_"
#define OUT_DATA_SOCKET_FIFO_PREFIX "vnetwork_out_data_"
#define IN_DATA_SOCKET_FIFO_PREFIX "vnetwork_in_data_socket_read_"
#define END_SOCKET_WRITE_FIFO_PREFIX "vnetwork_end_socket_write_"
#define END_SOCKET_READ_FIFO_PREFIX "vnetwork_end_socket_read_"

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

char* get_terminate_fifo_name(const char* ip) {
    return add_as_prefix(TERMINATE_FIFO_PREFIX, ip);
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
