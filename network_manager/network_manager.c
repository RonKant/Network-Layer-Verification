#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../fifo_utils.h"
#include "../socket_utils.h"
#include "../ip.h"
#include "network_manager.h"

#define SOCKET_MAP_SIZE 100

#define MAX_HANDLES_PER_EPOCH 10
#define MAX_BIND_REQUEST_SIZE 100

const char* ip_str_repr = "%01X%01X%02X%04X%04X%04X%02X%02X%04X%08s%08s%";

// Use tomer's version in ip.c later
IPPacket str_to_ip(char* s) {
    IPPacket result = (IPPacket)malloc(sizeof(*result));
    int scanned = sscanf(s, ip_str_repr,
                         &(result->version),
                         &(result->ihl),
                         &(result->dscp_and_ecn),
                         &(result->total_length),
                         &(result->id),
                         &(result->flags_and_offset),
                         &(result->ttl),
                         &(result->protocol),
                         &(result->header_checksum),
                         &(result->src_ip),
                         &(result->dst_ip));

    if (scanned != 11) { // if scan failed -- error
        free(result);
        return NULL;
    }
    char* s_data = s + (result->ihl * 32);      //s_data is the pointer of the data section in s. ihl is the length of the header
    result->data = (char*)malloc(strlen(s_data) + 1);

    if (result -> data == NULL) {
        free(result);
        return NULL;
    }

    strcpy(result->data, s_data);

    return result;
}

/******************************************
 * Utility Functions For fifo creation
 * ****************************************/


int initialize_network_manager_fifos(NetworkManager manager) {
    char* terminate_fifo_name = get_terminate_fifo_name(manager->ip);
    char* in_packet_fifo_name = get_in_packet_fifo_name(manager->ip);
    char* bind_request_fifo_name = get_bind_requests_fifo_name(manager->ip);
    char* connect_request_fifo_name = get_connect_requests_fifo_name(manager->ip);

    if (terminate_fifo_name == NULL
        || in_packet_fifo_name == NULL
        || in_packet_fifo_name == NULL
        || connect_request_fifo_name == NULL) {
            printf("Failed to extract fifo names for manager.\n");
            free(terminate_fifo_name); free(in_packet_fifo_name); free(bind_request_fifo_name); free(connect_request_fifo_name);
            return -1;
        }



    if (mkfifo(terminate_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(in_packet_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(bind_request_fifo_name, DEFAULT_FIFO_MODE) != 0
        || mkfifo(connect_request_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            printf("Failed creating manager fifos (probably already exist - will try to delete them automatically).\n");
            free(terminate_fifo_name); free(in_packet_fifo_name); free(bind_request_fifo_name); free(connect_request_fifo_name);
            return -1;
        }

    manager->terminate_fifo_fd = open(terminate_fifo_name, O_RDONLY | O_NONBLOCK);
    manager->in_packet_fifo_fd = open(in_packet_fifo_name, O_RDONLY | O_NONBLOCK);
    manager->bind_request_fifo_fd = open(bind_request_fifo_name, O_RDONLY | O_NONBLOCK);
    manager->connect_request_fifo_fd = open(connect_request_fifo_name, O_RDONLY | O_NONBLOCK);

    free(terminate_fifo_name); free(in_packet_fifo_name); free(bind_request_fifo_name); free(connect_request_fifo_name);

    if (manager->terminate_fifo_fd == -1
        || manager->in_packet_fifo_fd == -1
        || manager->bind_request_fifo_fd == -1
        || manager->connect_request_fifo_fd == -1) {
            printf("Failed to open fifos.\n");
            return -1;
        }

    return 0;
}

/******************************************
 * Utility Functions For manager loop
 * ****************************************/

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


/**
 * With having already read version and header size, completes the reading of a given ip packet.
 * Returns NULL on error.
 */
IPPacket read_ip_packet_from_stream(int out_packet_fd, char version, char header_size) {
    int header_size_bytes = 4 * ((int)header_size);
    char* header = (char*)malloc(header_size_bytes);
    if (header == NULL) {
        printf("Failed allocating ip header of size %d.\n", header_size_bytes);
        return NULL;
    }

    // try to read header
    header[0] = (version << 4) | header_size;
    int read_result = read_nonzero_entire_message(out_packet_fd, header + 1, header_size - 1);
    if (read_result == -1) {
        printf("Failed reading header from fifo.\n");
        free(header);
        return NULL;
    }

    int total_packet_length = (((int)header[2]) << 8) + ((int)header[3]);

    char* ip_packet_raw = (char*)malloc(total_packet_length);
    if (ip_packet_raw == NULL) {
        printf("Failed allocating ip packet of size %d.\n", total_packet_length);
        free(header);
        return NULL;
    }

    if (memcpy(ip_packet_raw, header, header_size_bytes) == NULL) {
        printf("Failed copying memory header into ip packet raw.\n");
        free(header);
        free(ip_packet_raw);
        return NULL;
    }

    // try to read entire packet
    read_result = read_nonzero_entire_message(
        out_packet_fd, ip_packet_raw + header_size_bytes,
        total_packet_length - header_size_bytes);

    if (read_result == -1) {
        printf("Failed reading ip packet of size %d.\n", total_packet_length);
        free(header);
        free(ip_packet_raw);
        return NULL;
    }

    IPPacket packet = str_to_ip(ip_packet_raw);
    free(header);
    free(ip_packet_raw);
    if (packet == NULL) {
        printf("Failed to convert raw pakcet.\n");
        return NULL;
    }

    return packet;
}

int handle_incoming_ip_packet(IPPacket packet) {
    // if not meant for self - decrease TTL, maybe discard, maybe send onward.

    // otherwise, check other IP fields, then strip IP header, check tcp header.

    // if socket destination exists - pass the message to socket anc handle with already existing functions.
        // here, should also push received data into user socket fifo and send acks.
    // otherwise - ? (discard? send something back?)
    return 0; // mock
}

int send_string(char* data_to_send) {
    return 0; //mock
}

/******************************************
 * Utility Functions For termination
 * ****************************************/

/**
 * Returns true if the manager has a pending temination order or if an error occurs.
 * Returns false otherwise.
 */
bool should_terminate_manager(NetworkManager manager) {
    int termination_fd = manager->terminate_fifo_fd;

    bool should_terminate = false;
    char buf[1];
    int read_length = read_entire_message(termination_fd, buf, 1);

    if (read_length == 1) {
        should_terminate = true;
    } else {
        should_terminate = false;
    }

    return should_terminate;
}


/**
 * destroys socket map and deletes fifos.
 */
void unlink_and_clean_manager(NetworkManager manager) {
    if (manager->terminate_fifo_fd != -1) close(manager->terminate_fifo_fd);
    if (manager->in_packet_fifo_fd != -1) close(manager->terminate_fifo_fd);
    if (manager->bind_request_fifo_fd != -1) close(manager->terminate_fifo_fd);
    if (manager->connect_request_fifo_fd != -1) close(manager->terminate_fifo_fd);

    char* terminate_fifo_name = get_terminate_fifo_name(manager->ip);
    char* in_packet_fifo_name = get_in_packet_fifo_name(manager->ip);
    char* bind_request_fifo_name = get_bind_requests_fifo_name(manager->ip);
    char* connect_request_fifo_name = get_connect_requests_fifo_name(manager->ip);

    if (terminate_fifo_name != NULL)
        unlink(terminate_fifo_name);
    if (in_packet_fifo_name != NULL)
        unlink(in_packet_fifo_name);
    if (bind_request_fifo_name != NULL)
        unlink(bind_request_fifo_name);
    if (connect_request_fifo_name != NULL)
        unlink(connect_request_fifo_name);

    if (terminate_fifo_name == NULL
        || in_packet_fifo_name == NULL
        || in_packet_fifo_name == NULL
        || connect_request_fifo_name == NULL) {
            printf("Failed to extract fifo names for manager (delete manually at %s).\n", FIFO_FOLDER_PATH_PREFIX);
        }

    free(terminate_fifo_name); free(in_packet_fifo_name); free(bind_request_fifo_name); free(connect_request_fifo_name);

    hashDestroy(manager->sockets, NULL);
    manager->sockets = NULL;
}

/**
 * Sends termination messages to all connected sockets.
 * Returns -1 if there were any problems, 0 on success.
 */
int terminate_manager(NetworkManager manager) {
    // should also unlink fifos here so next time the manager comes up it's clean.
    // also cleans hashMap.
    if (NULL == manager || NULL == manager->sockets) {
        return -1;
    }

    // HASH_MAP_FOREACH(sock_id, manager->sockets) {
    //     // send termination signal to socket.
    //     // unlink socket fifos. TODO
    // }

    unlink_and_clean_manager(manager);

    return 0;
}

void remove_and_destroy_socket(NetworkManager manager, SocketID sock_id) {
    Socket to_remove = getSocket(manager->sockets, sock_id, NULL);
    if (NULL == to_remove) {
        return; // nothing to destroy.
    }

    hashmapRemove(manager->sockets, sock_id, NULL);
    //destroy_socket(to_remove); //this is done in hashmap remove (shouldn't TODO)
}

/******************************************
 * Stages of manager loop
 * ****************************************/

/**
 * Goes over the incoming packets, handling them with the corresponding sockets.
 * 0 on success, otherwise - failure.
 */
int handle_in_packets_fifo(NetworkManager manager) {
    int in_packet_fd = manager->in_packet_fifo_fd;

    for (int i = 0; i < MAX_HANDLES_PER_EPOCH; ++i) {
        char version_and_header_size[1];
        int read_result = read_entire_message(in_packet_fd, version_and_header_size, 1);

        if (read_result == -1) {
            printf("Error while trying to read packet from outgoing fifo.\n");
            return -1;
        }
        if (read_result == 0) {
            break; // no more messages to read in this epoch.
        }

        char version = (version_and_header_size[1] >> 4);
        char header_size = (version_and_header_size[1] << 4) >> 4;

        IPPacket packet = read_ip_packet_from_stream(in_packet_fd, version, header_size);
        if (packet == NULL) {
            return -1;
        }

        if (handle_incoming_ip_packet(packet) != 0) {
            printf("Error sending packet.\n");
            free(packet);
            return -1;
        }
        
        free(packet);
    }

    return 0;
}

/**
 * Go over outgoing messages (not packets) fifo and send each to it's target.
 */
int handle_out_requests_fifo(NetworkManager manager) {
    return 0; // mock
}

/**
 * Checks whether a given port is free for binding.
 */
bool can_bind_new_socket(NetworkManager manager, SocketID sock_id) {
    Socket socket_on_target_port = getSocket(manager->sockets, sock_id, NULL);
    return (socket_on_target_port == NULL);
}

/**
 * Creates a new binding between a socket and a port.
 * Entering here is assuming port is already free.
 */
int bind_new_socket(NetworkManager manager, SocketID sock_id) {
    SocketID id_copy = copy_socket_id(sock_id);
    if (id_copy == NULL) {
        return -1;
    }

    Socket new_socket = create_bound_socket(id_copy);
    int result = 0;

    destroy_socket_id(id_copy);

    if (NULL == new_socket) {
        printf("Error: failed allocating a new listening socket.\n");
        result = -1;
    } else {

        HashMapErrors err = insertSocket(manager->sockets, sock_id, new_socket);
        if (err != HASH_MAP_SUCCESS) {
            printf("Error: failed inserting socket in to hash map.\n");
            result = -1;
        } else {
            result = 0;
        }
    }
    destroy_socket(new_socket);
    return result; // mock
}

int handle_bind_request(NetworkManager manager, char* bind_request) {

    int port, pid, socket_counter;
    if (sscanf(bind_request, "%d_%d_%d", &port, &pid, &socket_counter) != 3) {
        printf("Invalid bind request (ignored).\n");
        return -1;
    }

    SocketID sock_id = (SocketID)malloc(sizeof(*sock_id));
    if (sock_id == NULL) return -1;

    init_empty_socket_id(sock_id);
    sock_id->src_ip = (char*)malloc(sizeof(char) * strlen(manager->ip) + 1); // TODO: change to our strlen.
    if (sock_id->src_ip == NULL) {
        free(sock_id);
        return -1;
    }

    strcpy(sock_id->src_ip, manager->ip); // TODO: change to our strcpy.
    sock_id->src_port = port;

    printf("\nBind Request:\n\tPort: %d.\n\tFrom: %d-%d.\n", port, pid, socket_counter);

    int result; // what to return to main.
    char reply; // what to reply to client.
    if (can_bind_new_socket(manager, sock_id)) {
        printf("\tPort free - accepting request.\n");
        int bind_result = bind_new_socket(manager, sock_id);

        if (bind_result != 0) {
            result = -1; reply = REQUEST_DENIED_FIFO;
        } else {
            result = 0; reply = REQUEST_GRANTED_FIFO;
            printf("\tBind successful.\n");
        }
    } else {
        printf("\tPort Taken - denying request.\n");
        result = 0; reply = REQUEST_DENIED_FIFO;
    }
    
    // send reply to client fifo.
    char* client_fifo_name = get_client_fifo_name(pid, socket_counter);
    if (write_char_to_fifo_name(client_fifo_name, reply) != 0) {
        if (reply == REQUEST_GRANTED_FIFO) remove_and_destroy_socket(manager, sock_id);
        result = -1;
    }

    free(client_fifo_name);
    destroy_socket_id(sock_id);
    return result;

}

/**
 * Checks bind fifo for new bind requests and handles them.
 * Returns 0 on success (even if could not bind since port taken), -1 on failure.
 */
int handle_bind_fifo(NetworkManager manager) {

    char bind_request[MAX_BIND_REQUEST_SIZE];
    int read_len = read_message_until_char(manager->bind_request_fifo_fd, bind_request, '\0');

    if (read_len < 0) {
        printf("Error reading bind request.\n");
        return -1;
    }

    if (read_len == 0) return 0; // no bind requests.

    // bind_request now contains a full bind request of format:
    // <port>_<pid>_<socket-counter>\0

    return handle_bind_request(manager, bind_request);
}

int check_and_handle_listen_request(SocketID sock_id, NetworkManager manager) {
    // Check listen fifo. If found something: If socket CAN listen, do stuff. otherwise send N.
    // return 0 on non critical errors so the entire system does not crash.

    char* listen_fifo_write_name = get_listen_fifo_write_end_name(sock_id);
    if (NULL == listen_fifo_write_name) return 0;

    char* accept_fifo_name = get_accept_fifo_write_end_name(sock_id);
    if (NULL == accept_fifo_name) {
        free(listen_fifo_write_name);
        return 0;
    }

    int listen_fifo_write_fd = open(listen_fifo_write_name, O_RDONLY | O_NONBLOCK);
    if (-1 == listen_fifo_write_fd) {
        free(listen_fifo_write_name); free(accept_fifo_name);
        return 0;
    }

    char request[1];
    int read_length = read_entire_message(listen_fifo_write_fd, request, 1);

    if (read_length == -1 || read_length == 0) {
        close(listen_fifo_write_fd);
        free(listen_fifo_write_name); free(accept_fifo_name);
        return 0;
    }

    printf("Received listen(%c) request from port: %d.\n", *request, sock_id->src_port);
    char reply;

    Socket sock = getSocket(manager->sockets, sock_id, NULL);
    if (sock == NULL || sock->state == LISTEN) {
        reply = REQUEST_DENIED_FIFO;
    } else {
        sock->connections = createQueue_g(compare_socket, destroy_socket, copy_socket);
        if (sock->connections == NULL) {
            reply = REQUEST_DENIED_FIFO;
        } else {
            sock->state = LISTEN;
            reply = REQUEST_GRANTED_FIFO;
        }
    }

    if (REQUEST_GRANTED_FIFO == reply) {
        if (mkfifo(accept_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            close(listen_fifo_write_fd);
            free(listen_fifo_write_name); free(accept_fifo_name);
            reply = REQUEST_DENIED_FIFO;
            sock->state = CLOSED;
        }
    }

    free(accept_fifo_name);


    // send reply to client fifo.
    char* listen_fifo_read_name = get_listen_fifo_read_end_name(sock_id);
    if (write_char_to_fifo_name(listen_fifo_read_name, reply) == -1) {
        printf("Error: writing response %c to client on port %d failed.\n", reply, sock_id->src_port);
        if (reply == REQUEST_GRANTED_FIFO) {
            sock->state = CLOSED;
            destroyQueue(sock->connections, NULL);
            sock->connections = NULL;
        }
    }

    close(listen_fifo_write_fd);
    free(listen_fifo_write_name); 
    free(listen_fifo_read_name);

    return 0;
}

int handle_socket_in_network(SocketID sock_id, NetworkManager manager) {
    /*
    * If socket is a listener and has a pending connection, and accept fifo is empty - put connection in accept fifo.
    * Check socket out fifo for RAW USER STRINGS. If there is something: push into socket outgoing queue.
    * Check end fifo. If there is something: close socket and free it's memory.
    * Check socket out queue/buffer. If (condition) - resend packet of unacked data.
    */

    // printf("\t(%p, %d) -> (%s, %d)\n", sock_id->src_ip, sock_id->src_port, sock_id->dst_ip, sock_id->dst_port);
    int return_value = 0;

    if (get_socket_state(sock_id) == BOUND_ONLY_SOCKET) {
        return_value = check_and_handle_listen_request(sock_id, manager);
        if (return_value != 0) return return_value;
    }

    return 0; // mock
}

/******************************************
 * Interface
 * ****************************************/

NetworkManager createNetworkManager(char* ip) {
    if (init_fifo_directory() != 0) return NULL;

    NetworkManager manager = (NetworkManager)malloc(sizeof(*manager));
    if (manager == NULL) return NULL;

    // so we can destroy them later if something fails.
    manager->sockets = NULL;

    manager->terminate_fifo_fd = -1;
    manager->bind_request_fifo_fd = -1;
    manager->connect_request_fifo_fd = -1;
    manager->in_packet_fifo_fd = -1;

    manager->ip = (char*)malloc(sizeof(*ip) * strlen(ip) + 1); // TODO: change to our strlen.
    if (NULL == manager->ip) {
        destroyNetworkManager(manager);
        return NULL;
    }
    if (strcpy(manager->ip, ip) == NULL) {
        destroyNetworkManager(manager);
        return NULL;
    }

    // fifo and hash map creation is done in manager loop.
    return manager;
}


void destroyNetworkManager(NetworkManager manager) {
    // should: free all memory in struct, unlink all fifos (?)
    // if clients exist - notify them about shutdown?
    // terminate here?
    unlink_and_clean_manager(manager);

    free(manager->ip);
    free(manager);

    printf("Manager memory freed.\n");
}

int managerLoop(NetworkManager manager) { 
    if (manager->sockets != NULL) {
        printf("Error: trying to call manager loop on an already running manager.\n");
        return -1;
    }
    if (initialize_network_manager_fifos(manager) != 0) {
        return -1;
    }
    manager->sockets = createHashMap(SOCKET_MAP_SIZE);
    if (manager->sockets == NULL) {
        return -1;
    }

    while (1) {
        if (should_terminate_manager(manager)) {
            printf("Terminating manager\n");
            terminate_manager(manager);
            return 0;
        }

        if (handle_bind_fifo(manager) != 0) {
            return -1;
        }
    
        // go over connect requests fifo, handle them
        
        if (handle_in_packets_fifo(manager) != 0) {
            return -1;
        }

        // currently broken because hashmap iterator does not work (because of badly initialized table entries)

        // printf("Connected sockets:\n");
        HASH_MAP_FOREACH(sock_id, manager->sockets) {
            handle_socket_in_network(sock_id, manager);
        }

    }
}

// int stopManager(char* ip);