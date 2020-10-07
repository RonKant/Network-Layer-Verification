#include <errno.h>
#include "fifo_utils.h"

/**
 * This module contains functions for obtaining fifo names out of network manager IPs and SocketID obejcts.
 */

#include <stdlib.h>
#include <string.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 

#include "socket_utils.h"
#include "util_types.h"

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
 * Allocates and returns a string representation of a socket.
 * Returns null on failure.
 */
char* construct_full_socket_fifo_name(char* prefix, SocketID sock_id) {
    if (sock_id == NULL) return NULL;
    char socket_repr[MAX_SOCKET_STRING_REPR_SIZE];
    char* dst_ip = sock_id->dst_ip;
    if (NULL == dst_ip) dst_ip = "NOIP";

    if (sprintf(socket_repr, "(%s_%d_%s_%d)", sock_id->src_ip, sock_id->src_port, dst_ip, sock_id->dst_port) < 0) {
        return NULL;
    }

    return construct_full_fifo_name(prefix, socket_repr);
}

char* get_listen_fifo_read_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(LISTEN_SOCKET_READ_FIFO_PREFIX, sock_id);
}

char* get_listen_fifo_write_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(LISTEN_SOCKET_WRITE_FIFO_PREFIX, sock_id);
}

char* get_accept_fifo_write_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(ACCEPT_SOCKET_READ_FIFO_PREFIX, sock_id);
}

char* get_connect_fifo_read_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(CONNECT_SOCKET_READ_FIFO_PREFIX, sock_id);

}

char* get_connect_fifo_write_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(CONNECT_SOCKET_WRITE_FIFO_PREFIX, sock_id);

}

char* get_socket_recv_fifo_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(IN_DATA_SOCKET_FIFO_PREFIX, sock_id);
}

char* get_end_fifo_read_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(END_SOCKET_READ_FIFO_PREFIX, sock_id);
}

char* get_end_fifo_write_end_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(END_SOCKET_WRITE_FIFO_PREFIX, sock_id);
}

char* get_socket_send_fifo_name(SocketID sock_id) {
    return construct_full_socket_fifo_name(OUT_DATA_SOCKET_FIFO_PREFIX, sock_id);
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
        int fifo_fd = open(fifo_name, O_RDWR);
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

int write_string_to_fifo_name(char* fifo_name, const char* to_write, int len) {
    if (NULL == fifo_name)
        return -1;
    else {
        int fifo_fd = open(fifo_name, O_RDWR);
        if (fifo_fd == -1)
            return -1;
        else {
            int written = write(fifo_fd, to_write, len);
            close(fifo_fd);
            return written;
        } 
    }
}



void close_socket_fifos(Socket socket) {
	if (socket->listen_fifo_read_end != -1) close(socket->listen_fifo_read_end);
	if (socket->listen_fifo_write_end != -1) close(socket->listen_fifo_write_end);
	if (socket->accept_fifo_write_end != -1) close(socket->accept_fifo_write_end);
	if (socket->out_fifo_read_end != -1) close(socket->out_fifo_read_end);
	if (socket->in_fifo_write_end != -1) close(socket->in_fifo_write_end);
	if (socket->end_fifo_read_end != -1) close(socket->end_fifo_read_end);
	if (socket->end_fifo_write_end != -1) close(socket->end_fifo_write_end);
}

void unlink_socket_fifos(Socket socket) {
	char* listen_fifo_read_end_name = get_listen_fifo_read_end_name(socket->id);
	char* listen_fifo_write_end_name = get_listen_fifo_write_end_name(socket->id);
	char* accept_fifo_write_end_name = get_accept_fifo_write_end_name(socket->id);
	char* out_fifo_read_end_name = get_out_fifo_read_end_name(socket->id);
	char* in_fifo_write_end_name = get_in_fifo_write_end_name(socket->id);
	char* end_fifo_read_end_name = get_end_fifo_read_end_name(socket->id);
	char* end_fifo_write_end_name = get_end_fifo_write_end_name(socket->id);

    if (listen_fifo_read_end_name == NULL
		|| listen_fifo_write_end_name == NULL
		|| accept_fifo_write_end_name == NULL
		|| out_fifo_read_end_name == NULL
		|| in_fifo_write_end_name == NULL
		|| end_fifo_read_end_name == NULL
		|| end_fifo_write_end_name == NULL) {
            printf("Failed to extract fifo names for socket (delete manually at %s).\n", FIFO_FOLDER_PATH_PREFIX);
        }

    unlink(listen_fifo_read_end_name);
    unlink(listen_fifo_write_end_name);
    unlink(accept_fifo_write_end_name);
    unlink(out_fifo_read_end_name);
    unlink(in_fifo_write_end_name);
    unlink(end_fifo_read_end_name);
    unlink(end_fifo_write_end_name);	

    free(listen_fifo_read_end_name);
    free(listen_fifo_write_end_name);
    free(accept_fifo_write_end_name);
    free(out_fifo_read_end_name);
    free(in_fifo_write_end_name);
    free(end_fifo_read_end_name);
    free(end_fifo_write_end_name);
}


int create_socket_end_fifos(SocketID sock_id) {
    char* end_fifo_read_name = get_end_fifo_read_end_name(sock_id);
    char* end_fifo_write_name = get_end_fifo_write_end_name(sock_id);

    if (NULL == end_fifo_read_name
        || NULL == end_fifo_write_name)
        return -1;


    int result = 0;

    if (0 != mkfifo(end_fifo_read_name, DEFAULT_FIFO_MODE)
        || 0 != mkfifo(end_fifo_write_name, DEFAULT_FIFO_MODE)) {
        unlink(end_fifo_read_name);
        unlink(end_fifo_write_name);
        result = -1;
    }

    free(end_fifo_read_name);
    free(end_fifo_write_name);
    return result;
}

/**
 * Like read_entire_message, but will not return 0 (will keep blocking).
 */
int read_nonzero_entire_message(int fd, char* buf, int len) {
    while (1) {
        int read_result = read_entire_message(fd, buf, len);
        if (read_result == -1) {
            return -1;
        }
        if (read_result > 0) {
            return read_result;
        }
    }
}

int read_nonzero_message_until_char(int fd, char* buf, char stop) {
    while (1) {
        int read_result = read_message_until_char(fd, buf, stop);
        if (read_result == -1) {
            return -1;
        }
        if (read_result > 0) {
            return read_result;
        }
    }
}

int read_entire_message(int fd, char* buf, int len) {
    if (len < 0) {
        printf("Invalid length provided - must be non-negative.\n");
        return -1;
    }

    if (NULL == buf) {
        printf("Invalid (NULL) buffer provided.\n");
        return -1;
    }

    int bytes_left = len;
    int read_result = read(fd, buf, len);
    if (read_result == 0) {
        return 0; // fd empty.
    }

    if (read_result == -1) {
        if (errno == EAGAIN) return 0;
        printf("An error has occurred while reading from file.\n");
        printf("\terrno: %d\n", errno);
        return -1;
    }

    bytes_left -= read_result;
    while (bytes_left > 0) {
        
        read_result = read(fd, ((char*)buf) + len - bytes_left, bytes_left);
        if (read_result == -1) {
            if (errno == EAGAIN) return 0;
            printf("An error has occurred while reading from file.\n");
            return -1;
        }

        bytes_left -= read_result;
    }

    return len;
}

/**
 * Reads a string from fd to buf, until stop is encountered (including it).
 * Returns length of message read, or 0 if buf is empty, or -1 on error.
 */
int read_message_until_char(int fd, char* buf, char stop) {
    if (NULL == buf) {
        printf("Invalid (NULL) buffer provided.\n");
        return -1;
    }

    char* current_end = buf;
    char current_char;
    int read_value;

    while ((read_value = read(fd, &current_char, 1)) > 0) {
        *current_end = current_char;
        current_end++;

        if (current_char == stop) break;
    }

    if (read_value < 0 && errno != EAGAIN) {
        printf("Error while reading from file.\n");
        printf("\terrno: %d.\n", errno);
        return -1;
    }

    if (current_end - buf == 0) { // read nothing.
        return 0;
    }

    if (current_char != stop) { // loop finished because EOF
        printf("Error: early EOF unexpected while reading from file.\n");
        return -1;
    }

    return current_end - buf;
}