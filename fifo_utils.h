#ifndef FIFO_UTILS_H
#define FIFO_UTILS_H

/**
 * This module contains functions for obtaining fifo names out of network manager IPs and SocketID obejcts.
 */

#include <stdlib.h>
#include <string.h>

#include "socket_utils.h"

#define MAX_SOCKET_STRING_REPR_SIZE 80

#define REQUEST_GRANTED 'K'
#define REQUEST_DENIED 'N'

#define FIFO_FOLDER_PATH_PREFIX "/tmp/vnetwork_fifos/"

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

#define CLIENT_FIFO_PREFIX "vnetwork_"

/**
 * create the fifo directory if it does not exist yet.
 */
int init_fifo_directory() {
    struct stat st = {0};
    if (stat(FIFO_FOLDER_PATH_PREFIX, &st) == -1) {
        return mkdir(FIFO_FOLDER_PATH_PREFIX, 0700);
    }
    return 0;
}

char* add_folder_prefix(const char* s) {
    char* result = (char*)malloc(strlen(s) + strlen(FIFO_FOLDER_PATH_PREFIX) + 1);
    if (result == NULL) return NULL;
    result[0] = '\0';

    result = strcat(result, FIFO_FOLDER_PATH_PREFIX);
    result = strcat(result, s);

    return result;
}

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

// constructs a name with folder prefix
char* construct_full_fifo_name(const char* prefix, const char* s) {
    char* result = add_as_prefix(prefix, s);
    if (NULL == result) return NULL;
    char* result_with_folder = add_folder_prefix(result);
    free(result);
    return result_with_folder;
}


/******************************************
 * Network manager fifos
 *****************************************/

char* get_terminate_fifo_name(const char* ip) {
    return construct_full_fifo_name(TERMINATE_FIFO_PREFIX, ip);
}

char* get_in_packet_fifo_name(const char* ip) {
    return construct_full_fifo_name(IN_PACKET_FIFO_PREFIX, ip);
}

char* get_bind_requests_fifo_name(const char* ip) {
    return construct_full_fifo_name(BIND_REQUEST_FIFO_PREFIX, ip);
}

char* get_connect_requests_fifo_name(const char* ip) {
    return construct_full_fifo_name(CONNECT_REQUEST_FIFO_PREFIX, ip);
}

/******************************************
 * Socket fifos
 *****************************************/

/**
 * Allocates and returns a string representation of a CONNECTED socket.
 * Returns null on failure.
 */
char* get_connected_socket_repr_string(SocketID sock_id) {
    if (sock_id == NULL || get_socket_state(sock_id) != CONNECTED_SOCKET) return NULL;

    char* result = (char*)malloc(sizeof(*result) * MAX_SOCKET_STRING_REPR_SIZE);
    if (NULL == result) return NULL;
    if (sprintf(result, "%s_%d_%s_%d", sock_id->src_ip, sock_id->src_port, sock_id->dst_ip, sock_id->dst_port) < 0) {
        free(result);
        return NULL;
    }

    return result;
}

/**
 * Allocates and returns a string representation of a BOUND ONLY socket.
 * Returns null on failure.
 */
char* get_bound_socket_repr_string(SocketID sock_id) {
    if (sock_id == NULL || get_socket_state(sock_id) != BOUND_ONLY_SOCKET) return NULL;

    char* result = (char*)malloc(sizeof(*result) * MAX_SOCKET_STRING_REPR_SIZE);
    if (NULL == result) return NULL;
    if (sprintf(result, "%s_%d_BOUND", sock_id->src_ip, sock_id->src_port) < 0) {
        free(result);
        return NULL;
    }

    return result;
}

/******************************************
 * Client fifo
 *****************************************/

char* get_client_fifo_name(int pid, int socket_counter) {
    char* result = (char*)malloc(sizeof(*result) * MAX_SOCKET_STRING_REPR_SIZE);
    if (NULL == result) return NULL;

    if (sprintf(result, "%d_%d", pid, socket_counter) < 0) {
        free(result);
        return NULL;
    }

    char* fifo_name = construct_full_fifo_name(CLIENT_FIFO_PREFIX, result);
    free(result);

    return fifo_name;
}

/******************************************
 * I/O
 *****************************************/

int write_char_to_fifo_name(char* fifo_name, char to_write) {
    if (NULL == fifo_name) {
        return -1;
    } else {
        int fifo_fd = open(fifo_name, O_WRONLY);
        if (fifo_fd == -1) {
            return -1;
        } else if (write(fifo_fd, &to_write, sizeof(to_write)) == -1) {
            close(fifo_fd);
            return -1;
        }
        close(fifo_fd);
        return 0;
    }
}

#endif