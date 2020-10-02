#ifndef FIFO_UTILS_H
#define FIFO_UTILS_H

/**
 * This module contains functions for obtaining fifo names out of network manager IPs and SocketID obejcts.
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

#include "util_types.h"

#define MAX_SOCKET_STRING_REPR_SIZE 80

#define DEFAULT_FIFO_MODE 0666

#define REQUEST_GRANTED_FIFO 'K'
#define REQUEST_DENIED_FIFO 'N'

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

#define CLIENT_FIFO_PREFIX "vnetwork_nonbound_client_"

/**
 * create the fifo directory if it does not exist yet.
 */
int init_fifo_directory();

char* add_folder_prefix(const char* s);

/**
 * Returns a new string, containing the concatenation of the two strings.
 * Used in translating ip/ports to fifo names.
 */
char* add_as_prefix(const char* prefix, const char* s);

// constructs a name with folder prefix
char* construct_full_fifo_name(const char* prefix, const char* s);


/******************************************
 * Network manager fifos
 *****************************************/

char* get_terminate_fifo_name(const char* ip);

char* get_in_packet_fifo_name(const char* ip);

char* get_bind_requests_fifo_name(const char* ip);

char* get_connect_requests_fifo_name(const char* ip);

/******************************************
 * Socket fifos
 *****************************************/

/**
 * Allocates and returns a string representation of a CONNECTED socket.
 * Returns null on failure.
 */
char* get_connected_socket_repr_string(SocketID sock_id);

char* construct_full_socket_fifo_name(char* prefix, SocketID sock_id);

char* get_listen_fifo_read_end_name(SocketID sock_id);

char* get_listen_fifo_write_end_name(SocketID sock_id);

char* get_accept_fifo_write_end_name(SocketID sock_id);

char* get_out_fifo_read_end_name(SocketID sock_id);

char* get_in_fifo_write_end_name(SocketID sock_id);

char* get_end_fifo_read_end_name(SocketID sock_id);

char* get_end_fifo_write_end_name(SocketID sock_id);

/**
 * Allocates and returns a string representation of a BOUND ONLY socket.
 * Returns null on failure.
 */
char* get_bound_socket_repr_string(SocketID sock_id);

/******************************************
 * Client fifo
 *****************************************/

char* get_client_fifo_name(int pid, int socket_counter);

/******************************************
 * I/O
 *****************************************/

int write_char_to_fifo_name(char* fifo_name, char to_write);




void close_socket_fifos(Socket socket);

void unlink_socket_fifos(Socket socket);

/**
 * Reads from fd into buf.
 * If fd is empty - returns 0.
 * If fd contains anything - will block and read into buf until entire len has been reached.
 * On success - returns 0 or length. On failre returns -1.
 */
int read_entire_message(int fd, char* buf, int len);

/**
 * Like read_entire_message, but will not return 0 (will keep blocking).
 */
int read_nonzero_entire_message(int fd, char* buf, int len);

/**
 * Reads a string from fd to buf, until stop is encountered (including it).
 * Returns length of message read, or 0 if buf is empty, or -1 on error.
 */
int read_message_until_char(int fd, char* buf, char stop);

/**
 * Reads a string from fd to buf until stop is encountered (including it).
 * Blocks until read something.
 * Returns length of message read, or -1 on error.
 */
int read_nonzero_message_until_char(int fd, char* buf, char stop);

#endif