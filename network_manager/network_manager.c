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

#define BREAK_ITERATION 2

void unlink_socket_fifos_server_side(Socket sock, NetworkManager manager) {
    char* end_fifo_write_name = get_end_fifo_write_end_name(sock->id);

    if (NULL == end_fifo_write_name
        || 0 != unlink(end_fifo_write_name)) {
        // printf("\tFailed unlinking end fifo write end of socket (delete manually).\n");
    }

    free(end_fifo_write_name);

    char* accept_fifo_name = get_accept_fifo_write_end_name(sock->id);
    if (NULL == accept_fifo_name
        || 0 != unlink(accept_fifo_name)) {
        // printf("\tFailed unlinking accept fifo of socket (delete manually).\n");
    }

    free(accept_fifo_name);

    char* send_fifo_name = get_socket_send_fifo_name(sock->id);
    if (NULL == send_fifo_name
        || 0 != unlink(send_fifo_name)) {
        // printf("\tFailed unlinking send fifo of socket (delete manually).\n");
    }

    free(send_fifo_name);

    char* recv_fifo_name = get_socket_recv_fifo_name(sock->id);

    if (NULL == recv_fifo_name
        || 0 != unlink(recv_fifo_name)) {
        // printf("\tFailed unlinking recv fifo of socket (delete manually).\n");
    }

    free(recv_fifo_name);

    // disconnect partner - may be stuck in recv
    if (sock->seq_of_first_send_window != 0) {
        SocketID partner_id = copy_socket_id(sock->id);
        if (NULL != partner_id) {
            strcpy(partner_id->src_ip, (sock->id)->dst_ip);
            partner_id->src_port = (sock->id)->dst_port;
            strcpy(partner_id->dst_ip, (sock->id)->src_ip);
            partner_id->dst_port = (sock->id)->src_port;


            char* partner_recv_fifo_name = get_socket_recv_fifo_name(partner_id);
            if (NULL != partner_recv_fifo_name) {
                char conn_closed = CONN_CLOSED;
                int recv_fifo_fd = open(partner_recv_fifo_name, O_RDWR);
                if (-1 != recv_fifo_fd) {
                    write(recv_fifo_fd, &conn_closed, 1);
                    close(recv_fifo_fd);
                }
                free(partner_recv_fifo_name);
            }
            destroy_socket_id(partner_id);
        }
    }


}

IPPacket read_ip_packet_from_file(int fd) {
    char total_length_str[11];

    for (int i = 0; i < 11; ++i) total_length_str[i] = '\0';

    int read_length = read_entire_message(fd, total_length_str, 10);
    if (10 != read_length) return NULL;

    total_length_str[11] = '\0';

    int total_length;

    if (sscanf(total_length_str, "%d", &total_length) < 1) return NULL;

    char* packet_str = (char*)malloc(total_length + 1);
    if (NULL == packet_str) return NULL;

    for (int i = 0; i < 10; ++i) packet_str[i] = total_length_str[i];

    read_length = read_nonzero_entire_message(fd, packet_str + 10, total_length - 10);
    if (total_length - 10 != read_length) {
        free(packet_str);
        return NULL;
    }

    packet_str[total_length] = '\0';

    IPPacket result = str_to_ip(packet_str);

    free(packet_str);
    return result;
}

/**
 * Converts tcp packet to ip packet and sends it to it's given destination.
 * Returns 0 on success, returns -2 if recepient not found, returns -1 on any other error. 
 */
int send_TCP_packet(TCPPacket packet, NetworkManager manager, char* dst_ip) {
    if (NULL == packet || NULL == manager || NULL == dst_ip) return -1;

    char* tcp_string = tcp_to_str(packet);
    if (NULL == tcp_string) return -1;

    IPPacket ip_packet = create_ip_packet(manager->ip, dst_ip, tcp_string);
    if (NULL == ip_packet) {
        free(tcp_string);
        return -1;
    }

    char* ip_str = ip_to_str(ip_packet);

    if (NULL == ip_str) {
        destroy_ip_packet(ip_packet);
        return -1;
    }

    char* dst_fifo_name = get_in_packet_fifo_name(dst_ip);
    if (NULL == dst_fifo_name) {
        destroy_ip_packet(ip_packet);
        free(ip_str);
        return -1;
    }

    int dst_fifo_fd = open(dst_fifo_name, O_RDWR);
    if (-1 == dst_fifo_fd) {
        // printf("Error: destination ip: %s does not exist.\n", dst_ip);
        free(dst_fifo_name);
        destroy_ip_packet(ip_packet);
        free(ip_str);
        return -2;
    }

    if (write(dst_fifo_fd, ip_str, strlen(ip_str)) == -1) {
        close(dst_fifo_fd);
        free(dst_fifo_name);
        destroy_ip_packet(ip_packet);
        free(ip_str);
        return -1;
    }

    close(dst_fifo_fd);
    free(dst_fifo_name);
    destroy_ip_packet(ip_packet);
    free(ip_str);
    return 0;
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

int handle_incoming_ip_packet(IPPacket packet, NetworkManager manager) {   
    // if socket destination exists - pass the message to socket anc handle with already existing functions.
        // here, should also push received data into user socket fifo and send acks.
    // otherwise - ? (discard? send something back?)
    if (NULL == packet) return -1;

    if (calc_ip_checksum(packet) != packet->header_checksum) {
        printf("Received a packet with bad checksum. Real value: %d. Should be: %d.\n", packet->header_checksum, calc_ip_checksum(packet));
        return 0;
    }

    if (strcmp(packet->dst_ip, manager->ip) != 0) {
        // for (int i = 0; i < strlen(packet->dst_ip); ++i) {
        //     printf("%d, %d, %d\n", (packet->dst_ip)[i], (manager->ip)[i], (packet->dst_ip)[i] == (manager->ip)[i]);
        // }
        printf("Received a packet meant for %s, not self (%s)\n", packet->dst_ip, manager->ip);   
        return 0;
    }

    TCPPacket tcp_packet = str_to_tcp(packet->data);
    if (NULL == tcp_packet) return -1;

    if (calc_checksum(tcp_packet) != tcp_packet->checksum) {
        printf("Received a TCP packet with bad checksum. Real value: %d. Should be: %d.\n", tcp_packet->checksum, calc_checksum(tcp_packet));
        return 0;
    }

    SocketID id = (SocketID)malloc(sizeof(*id));
    init_empty_socket_id(id);
    strcpy(id->src_ip, packet->dst_ip);
    strcpy(id->dst_ip, packet->src_ip);
    id->src_port = tcp_packet->dst_port;
    id->dst_port = tcp_packet->src_port;

    Socket sock = getSocket(manager->sockets, id);
    TCPPacket reply = NULL;

    if (NULL != sock) {
        reply = handle_packet(sock, tcp_packet, id->src_ip, manager);
    } else {
        ip_set_empty(id->dst_ip);
        id->dst_port = EMPTY_PORT;
        sock = getSocket(manager->sockets, id);

        if (NULL != sock) {
            reply = handle_packet(sock, tcp_packet, id->src_ip, manager);
        } else {
            // printf("TCP recepient not found.\n");
        }
    }
    if (NULL != reply) {
        if (send_TCP_packet(reply, manager, id->dst_ip) == 0)
            sock->last_send_clock = clock();
    }

    free(id);
    destroy_tcp_packet(reply);
    destroy_tcp_packet(tcp_packet);

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
            // printf("Failed to extract fifo names for manager (delete manually at %s).\n", FIFO_FOLDER_PATH_PREFIX);
        }

    free(terminate_fifo_name); free(in_packet_fifo_name); free(bind_request_fifo_name); free(connect_request_fifo_name);

    if (NULL != manager->sockets) {
        HASH_MAP_FOREACH(sock_id, manager->sockets) {
            Socket sock = getSocket(manager->sockets, sock_id);
            if (NULL != sock) {
                destroy_socket_fifos(sock);
            }
        }

        hashDestroy(manager->sockets);
        manager->sockets = NULL;
    }
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

    HASH_MAP_FOREACH(sock_id, manager->sockets) {

        char* end_fifo_read_name = get_end_fifo_read_end_name(sock_id);
        if (NULL != end_fifo_read_name) {
            
            int end_fifo_read_fd = open(end_fifo_read_name, O_RDWR);

            if (-1 != end_fifo_read_fd) {

                if (write(end_fifo_read_fd, "E", 1) != 1) {
                    printf("Failed to notify active user about socket closing.\n");
                } else {
                    printf("Successfuly notified user %d about closing.\n", sock_id->src_port);
                }

                close(end_fifo_read_fd);
            }

            free(end_fifo_read_name);
        }

        printf("Removing socket on port %d.\n", sock_id->src_port);

        Socket sock = getSocket(manager->sockets, sock_id);
        if (NULL != sock) {
            unlink_socket_fifos_server_side(sock, manager);
        }

    }

    //unlink_and_clean_manager(manager);

    return 0;
}

void remove_and_destroy_socket(NetworkManager manager, SocketID sock_id) {
    Socket to_remove = getSocket(manager->sockets, sock_id);
    if (NULL == to_remove) {
        return; // nothing to destroy.
    }

    hashmapRemove(manager->sockets, sock_id);
    unlink_socket_fifos_server_side(to_remove, manager);
    destroy_socket(to_remove);
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

        IPPacket packet = read_ip_packet_from_file(in_packet_fd);

        if (NULL == packet) break; // no packets

        if (handle_incoming_ip_packet(packet, manager) != 0) {
            printf("Error Receiving packet.\n");
            free(packet);
            return 0;
        }
        destroy_ip_packet(packet);
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
    Socket socket_on_target_port = getSocket(manager->sockets, sock_id);
    return (socket_on_target_port == NULL);
}

/**
 * Creates a new binding between a socket and a port.
 * Entering here is assuming port is already free.
 */
int bind_new_socket(NetworkManager manager, SocketID sock_id) {
    Socket new_socket = create_bound_socket(sock_id);
    int result = 0;

    if (NULL == new_socket) {
        printf("Error: failed allocating a new listening socket.\n");
        result = -1;
    } else {

        HashMapErrors err = insertSocket(manager->sockets, new_socket);
        if (err != true) {
            printf("Error: failed inserting socket in to hash map.\n");
            result = -1;
        } else {
            result = 0;
        }
    }
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
            destroy_socket_id(sock_id);
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
        if (reply == REQUEST_GRANTED_FIFO) {
            remove_and_destroy_socket(manager, sock_id);
            result = BREAK_ITERATION;
        } else result = -1;
    }

    free(client_fifo_name);
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

    Socket sock = getSocket(manager->sockets, sock_id);
    if (sock == NULL || sock->state == LISTEN) return 0;

    char* listen_fifo_write_name = get_listen_fifo_write_end_name(sock_id);
    if (NULL == listen_fifo_write_name) return 0;

    char* accept_fifo_name = get_accept_fifo_write_end_name(sock_id);
    if (NULL == accept_fifo_name) {
        free(listen_fifo_write_name);
        return 0;
    }


    char* listen_fifo_read_name = get_listen_fifo_read_end_name(sock_id);
    if (NULL == listen_fifo_read_name) {
        free(listen_fifo_write_name); 
        free(accept_fifo_name);
        return 0;
    }

    int listen_fifo_read_fd = open(listen_fifo_read_name, O_WRONLY | O_NONBLOCK);
    if (listen_fifo_read_fd == -1) {
        free(listen_fifo_write_name);
        free(accept_fifo_name);
        free(listen_fifo_read_name);
        return 0;
    }

    int listen_fifo_write_fd = open(listen_fifo_write_name, O_RDONLY);
    if (-1 == listen_fifo_write_fd) {
        free(listen_fifo_write_name); free(accept_fifo_name);
        return 0;
    }

    char request[1];
    int read_length = read_entire_message(listen_fifo_write_fd, request, 1);
    if (read_length == -1 || read_length == 0) {
        close(listen_fifo_write_fd);
        free(listen_fifo_write_name); free(accept_fifo_name);
        close(listen_fifo_read_fd);
        free(listen_fifo_read_name);
        return 0;
    }

    printf("Received listen(%c) request from port: %d.\n", *request, sock_id->src_port);
    char reply;


    sock->state = LISTEN;
    sock->max_connections = (int)(*request);
    reply = REQUEST_GRANTED_FIFO;


    if (REQUEST_GRANTED_FIFO == reply) {
        if (mkfifo(accept_fifo_name, DEFAULT_FIFO_MODE) != 0) {
            close(listen_fifo_write_fd);
            free(listen_fifo_write_name); free(accept_fifo_name);
            close(listen_fifo_read_fd);
            free(listen_fifo_read_name);
            reply = REQUEST_DENIED_FIFO;
            sock->state = CLOSED;
        }
    }

    free(accept_fifo_name);

    // send reply to client fifo.
    if (write(listen_fifo_read_fd, &reply, 1) != 1) {
        printf("Error: writing response %c to client on port %d failed.\n", reply, sock_id->src_port);
        if (reply == REQUEST_GRANTED_FIFO) {
            sock->state = CLOSED;
        }
    }
    close(listen_fifo_write_fd);
    free(listen_fifo_write_name); 
    close(listen_fifo_read_fd);
    free(listen_fifo_read_name);

    return 0;
}

int check_and_handle_connect_request(SocketID sock_id, NetworkManager manager) {
    // Check socket connect fifo. If there is something and socket is bound AND LISTENING, create new connection 
    // in send SYN mode, and insert it to connection queue and hashmap.

    Socket sock = getSocket(manager->sockets, sock_id);
    if (sock == NULL || sock->state != CLOSED) return 0;

    char* connect_fifo_read_end_name = get_connect_fifo_read_end_name(sock_id);
    if (NULL == connect_fifo_read_end_name) return 0;

    int connect_fifo_read_fd = open(connect_fifo_read_end_name, O_WRONLY | O_NONBLOCK);
    if (-1 == connect_fifo_read_fd) {
        free(connect_fifo_read_end_name);
        return 0;
    }

    close(connect_fifo_read_fd);
    free(connect_fifo_read_end_name);

    char* connect_fifo_write_end_name = get_connect_fifo_write_end_name(sock_id);
    if (NULL == connect_fifo_write_end_name) return 0;

    int connect_fifo_write_fd = open(connect_fifo_write_end_name, O_RDONLY);
    if (-1 == connect_fifo_write_fd) {  
        free(connect_fifo_write_end_name);
        return 0;
    }

    char request[MAX_SOCKET_STRING_REPR_SIZE];
    int read_length = read_nonzero_message_until_char(connect_fifo_write_fd, request, '\0');

    if (read_length == -1 || read_length == 0) {
        close(connect_fifo_write_fd);
        free(connect_fifo_write_end_name);
        return 0;
    }

    char dst_ip[MAX_IP_LENGTH + 1];
    int dst_port;

    if (NULL == dst_ip) {
        close(connect_fifo_write_fd);
        free(connect_fifo_write_end_name);
        return 0;
    }

    int sscanf_result_1, sscanf_result_2;
    bool underscore_found = false;
    for (int i = 0; i < strlen(request); ++i) {
        if (request[i] == '_') {
            underscore_found = true;
            request[i] = '\0';

            sscanf_result_1 = sscanf(request + i + 1, "%d", &dst_port);
            sscanf_result_2 = sscanf(request, "%s", dst_ip);
            break;
        }
    }

    if (!underscore_found || sscanf_result_1 != 1 || sscanf_result_2 != 1) {
        close(connect_fifo_write_fd);
        free(connect_fifo_write_end_name);
        return 0;
    }
    close(connect_fifo_write_fd);
    free(connect_fifo_write_end_name);
    printf("Connect Request:\n\tDestination: (%s, %d)\n\tSource: (%s, %d)\n",
        sock_id->src_ip, sock_id->src_port,
        dst_ip, dst_port);

    SocketID connected_client_id = copy_socket_id(sock_id);
    if (NULL == connected_client_id) {
        return 0;
    }
    strcpy(connected_client_id->dst_ip, dst_ip);
    connected_client_id->dst_port = dst_port;

    Socket connected_sock = getSocket(manager->sockets, connected_client_id);
    if (connected_sock != NULL) {
        // client already exists
        destroy_socket_id(connected_client_id);
        return 0;
    }

    destroy_socket_id(connected_client_id);

    sock = getSocket(manager->sockets, sock_id);
    if (sock == NULL || sock->state == LISTEN) {
        return 0;
    } else {
        bool result = hashmapRemove(manager->sockets, sock_id);
        if (result != true) {
            printf("Error: failed removing listener socket from hashmap.\n");
            return -1;
        }
        sock->state = SYN_SENT;
        srand(time(NULL));
        sock->seq_of_first_send_window = rand() % 2000 + 1;
        sock->seq_of_first_recv_window = 0;
        strcpy((sock->id)->dst_ip, dst_ip);
        (sock->id)->dst_port = dst_port;
        strcpy(sock_id->dst_ip, dst_ip);
        sock_id->dst_port = dst_port;
        if (true != insertSocket(manager->sockets, sock)) {
            printf("Error: hash map insertion error in socket connect.\n");
            destroy_socket(sock);
            return 0;
        }
    }

    return BREAK_ITERATION;
}

int check_and_handle_out_fifo(SocketID sock_id, NetworkManager manager) {
    // Check socket out fifo for RAW USER STRINGS. If there is something: push into socket outgoing queue.

    return 0;
}

/**
 * If socket state is closed, inform user. Also free all memory related to socket.
 */
int check_and_handle_closed_socket(SocketID id_for_fifo, NetworkManager manager,
    Socket socket_to_close) {
    if (CLOSED == socket_to_close->state 
        && (DIFF2SEC(clock() - socket_to_close->creation_time)) > SOCKET_TIMEOUT) {

        char* end_fifo_read_name = get_end_fifo_read_end_name(id_for_fifo);
        if (NULL == end_fifo_read_name) return 0;
        // printf("3 %s.\n", end_fifo_read_name);
        int end_fifo_read_fd = open(end_fifo_read_name, O_RDWR);
        // printf("4 %d.\n", errno);
        if (-1 != end_fifo_read_fd) {
            if (write(end_fifo_read_fd, "E", 1) == 1) {
                printf("Failed to notify active user about socket closing.\n");
            }

            close(end_fifo_read_fd);
        }
        // printf("Removing id: (%s, %d), (%s, %d).\n",
        //     id_for_fifo->src_ip, id_for_fifo->src_port,
        //     id_for_fifo->dst_ip, id_for_fifo->dst_port);

        printf("Removing socket on port %d.\n", (socket_to_close->id)->src_port);
        remove_and_destroy_socket(manager, socket_to_close->id);
        free(end_fifo_read_name);

        return BREAK_ITERATION;
    }

    return 0;
}

/**
 * If received close command - change socket state to closed.
 */
int check_and_handle_close_command(SocketID id_for_fifo, NetworkManager manager,
    Socket socket_to_close) {


    char* end_fifo_write_name = get_end_fifo_write_end_name(id_for_fifo);
    if (NULL == end_fifo_write_name) return 0;

    // for synching.
    char* end_fifo_read_name = get_end_fifo_read_end_name(id_for_fifo); 
    if (NULL == end_fifo_read_name) {
        free(end_fifo_write_name);
        return 0;
    }

    int end_fifo_read_fd = open(end_fifo_read_name, O_WRONLY | O_NONBLOCK);
    if (-1 == end_fifo_read_fd) {
        free(end_fifo_read_name);
        free(end_fifo_write_name);
        return 0;
    }

    close(end_fifo_read_fd);
    free(end_fifo_read_name);

    int end_fifo_write_fd = open(end_fifo_write_name, O_RDONLY);
    if (-1 == end_fifo_write_fd) {
        free(end_fifo_write_name);
        return 0;
    }

    char message;
    if (read(end_fifo_write_fd, &message, 1) == 1) {
        printf("Closing socket on port %d (By user command).\n", (socket_to_close->id)->src_port);
        
        if (socket_to_close->state == SYN_SENT) {
            ip_set_empty((socket_to_close->id)->dst_ip);
            (socket_to_close->id)->dst_port = EMPTY_PORT;
        }
        
        socket_to_close->seq_of_first_send_window += QueueSize(socket_to_close->send_window);
        socket_to_close->state = FIN_WAIT_1;
        socket_to_close->time_since_fin_sent = clock();
    }

    close(end_fifo_write_fd);
    free(end_fifo_write_name);

    return 0;
}

int check_and_handle_socket_end_fifo(SocketID sock_id, NetworkManager manager) {
    // Check end fifo. If there is something: close socket and free it's memory.
    // If socket is closed, write into end fifo and remove it.

    Socket socket_to_close = getSocket(manager->sockets, sock_id);
    if (NULL == socket_to_close) return 0;
    SocketID id_for_fifo = sock_id;

    if (socket_to_close->state == SYN_SENT) {
        // this is a not yet connected socket - so it's end request is in 
        // bound only fifo
        id_for_fifo = copy_socket_id(sock_id);
        if (NULL == id_for_fifo) return 0;

        ip_set_empty(id_for_fifo->dst_ip);
        id_for_fifo->dst_port = EMPTY_PORT;
    }

    int result = check_and_handle_close_command(id_for_fifo, manager, socket_to_close);
    if (0 != result)  {
        if (id_for_fifo != sock_id) destroy_socket_id(id_for_fifo);
        return result;
    }

    result = check_and_handle_closed_socket(id_for_fifo, manager, socket_to_close);
    if (id_for_fifo != sock_id) destroy_socket_id(id_for_fifo);
    return result;
}

char* get_next_socket_packet_data(Socket sock) {
    if (NULL == sock || NULL == sock->send_window) return NULL;
    char* data = (char*)malloc(MAX_DATA_PER_PACKET * sizeof(char) + 1);
    if (NULL == data) return NULL;

    int bytes_added = 0;

    QUEUE_FOR_EACH(item, sock->send_window) {
        data[bytes_added++] = item;

        if (bytes_added == MAX_DATA_PER_PACKET) break;
    }

    data[bytes_added] = '\0';

    return data;
}

int check_and_handle_send_window(SocketID sock_id, NetworkManager manager) {
    Socket sock = getSocket(manager->sockets, sock_id);
    if (NULL == sock || sock->state != ESTABLISED) return 0;
    // Check socket out queue/buffer. If (condition) - resend packet of unacked data.
    if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_SEND_AGAIN_TIME) {

        char* est_socket_data = get_next_socket_packet_data(sock);
        if (est_socket_data == NULL) est_socket_data = "";

        TCPPacket ack_packet = construct_packet(sock, est_socket_data, ACK, sock_id->dst_port);
        // printf("Trying to send packet with data:\n\t%s\n", est_socket_data);
        free(est_socket_data);
        if (NULL == ack_packet) return 0;

        ack_packet->seq_num = sock->seq_of_first_send_window;
        ack_packet->ack_num = sock->seq_of_first_recv_window;

        if (0 == send_TCP_packet(ack_packet, manager, sock_id->dst_ip)) {
            sock->last_send_clock = clock();
        }
        destroy_tcp_packet(ack_packet);
    }
    return 0;
}

int check_and_handle_outgoing_status_messages(SocketID sock_id, NetworkManager manager) {
    // Handles sending status messages, such as SYN, SYN+ACK, FIN, FIN+ACK, ACK (?)

    Socket sock = getSocket(manager->sockets, sock_id);
    if (NULL == sock) return 0;

    if (sock->state == SYN_SENT) {
        if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_SEND_AGAIN_TIME) {

            sock->seq_of_first_send_window--;
            TCPPacket syn_packet = construct_packet(sock, "", SYN, sock_id->dst_port);
            sock->seq_of_first_send_window++;
            
            if (NULL == syn_packet) return 0;


            if (0 == send_TCP_packet(syn_packet, manager, sock_id->dst_ip)) {
                sock->last_send_clock = clock();
            }
            destroy_tcp_packet(syn_packet);
        }
    } else if (sock->state == SYN_RECEIVED) {
        if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_SEND_AGAIN_TIME) {
            sock->seq_of_first_send_window--;
            TCPPacket syn_ack_packet = construct_packet(sock, "", SYN | ACK, sock_id->dst_port);
            sock->seq_of_first_send_window++;
            if (NULL == syn_ack_packet) return 0;

            syn_ack_packet->seq_num = sock->seq_of_first_send_window - 1;
            syn_ack_packet->ack_num = sock->seq_of_first_recv_window;

            if (0 == send_TCP_packet(syn_ack_packet, manager, sock_id->dst_ip)) {
                sock->last_send_clock = clock();
            }
            destroy_tcp_packet(syn_ack_packet);
        }
    } else if (sock->state == ESTABLISED) {

        if (NULL == sock->send_window) {
            printf("Invalid conected socket on port %d - %d: no send window.",
                (sock->id)->src_port, (sock->id)->dst_port);
            return 0;
        }
        // user can try to send things.
        char* socket_send_fifo_name = get_socket_send_fifo_name(sock->id);
        if (NULL == socket_send_fifo_name) return 0;

        int socket_send_fifo_fd = open(socket_send_fifo_name, O_RDONLY | O_NONBLOCK);
        free(socket_send_fifo_name);

        if (-1 == socket_send_fifo_fd) return 0;

        char read_byte;

        for (int i = 0; i < MAX_DATA_PER_PACKET; ++i) {
            if (QueueSize(sock->send_window) >= QueueCapacity(sock->send_window))
                break;

            int read_length = read(socket_send_fifo_fd, &read_byte, 1);
            if (read_length != 1) break;

            enqueue(sock->send_window, read_byte);
            close(socket_send_fifo_fd);
        }
        close(socket_send_fifo_fd);
    } else if (sock->state == FIN_WAIT_1) {
        if (DIFF2SEC(clock() - sock->time_since_fin_sent) > SOCKET_FIN_RST_TIME) {
            TCPPacket rst_packet = construct_packet(sock, "", RST, sock_id->dst_port);
            if (NULL != rst_packet) {
                if (-1 != send_TCP_packet(rst_packet, manager, sock_id->dst_ip)) {
                    sock->state = TIME_WAIT;
                    destroy_tcp_packet(rst_packet);
                    return 0;
                }
                destroy_tcp_packet(rst_packet);
            }
        }
        if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_SEND_AGAIN_TIME) {
            TCPPacket fin_packet = construct_packet(sock, "", FIN, sock_id->dst_port);
            if (NULL == fin_packet) return 0;

            if (0 == send_TCP_packet(fin_packet, manager, sock_id->dst_ip)) {
                sock->last_send_clock = clock();
            }
            destroy_tcp_packet(fin_packet);
        }
    } else if (sock->state == CLOSE_WAIT) {
        sock->seq_of_first_send_window += QueueSize(sock->send_window);
        sock->state = LAST_ACK;
    } else if (sock->state == CLOSING) {
        if (DIFF2SEC(clock() - sock->time_since_fin_sent) > SOCKET_FIN_RST_TIME) {
            TCPPacket rst_packet = construct_packet(sock, "", RST, sock_id->dst_port);
            if (NULL != rst_packet) {
                if (0 == send_TCP_packet(rst_packet, manager, sock_id->dst_ip)) {
                    sock->state = TIME_WAIT;
                    return 0;
                }
                destroy_tcp_packet(rst_packet);
            }
        }
    } else if (sock->state == LAST_ACK) {
        if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_SEND_AGAIN_TIME) {
            TCPPacket fin_packet = construct_packet(sock, "", FIN | ACK, sock_id->dst_port);
            if (NULL == fin_packet) return 0;

            if (0 == send_TCP_packet(fin_packet, manager, sock_id->dst_ip)) {
                sock->last_send_clock = clock();
            }
            destroy_tcp_packet(fin_packet);
        }
    } else if (sock->state == TIME_WAIT) {
        if (DIFF2SEC(clock() - sock->last_send_clock) > SOCKET_TIME_WAIT_TIMEOUT) {
            sock->state = CLOSED;
        }
    }
    return 0;
}

int handle_socket_in_network(SocketID sock_id, NetworkManager manager) {
    // printf("\t(%s, %d) -> (%s, %d)\n", sock_id->src_ip, sock_id->src_port, sock_id->dst_ip, sock_id->dst_port);
    int return_value = 0;
    if (get_socket_state(sock_id) == BOUND_ONLY_SOCKET) {
        return_value = check_and_handle_listen_request(sock_id, manager);
        if (return_value != 0) return return_value;

        return_value = check_and_handle_connect_request(sock_id, manager);
        if (return_value != 0) return return_value;

    } else if (get_socket_state(sock_id) == CONNECTED_SOCKET) {
        return_value = check_and_handle_out_fifo(sock_id, manager);
        if (return_value != 0) return return_value;
    }

    return_value = check_and_handle_socket_end_fifo(sock_id, manager);
    if (return_value != 0) return return_value;

    return_value = check_and_handle_send_window(sock_id, manager);
    if (return_value != 0) return return_value;

    return_value = check_and_handle_outgoing_status_messages(sock_id, manager);
    if (return_value != 0) return return_value;

    return 0; // mock
}

/******************************************
 * Interface
 * ****************************************/

NetworkManager createNetworkManager(char* ip) {
    if (strlen(ip) != MAX_IP_LENGTH) {
        printf("Manager ip has to be 16 digits.\n");
        return NULL;
    }
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
        // printf("Connected sockets:\n");
        HASH_MAP_FOREACH(sock_id, manager->sockets) {
            int result = handle_socket_in_network(sock_id, manager);
            if (result == BREAK_ITERATION) break;
            if (-1 == result) return -1;
        }
    }
}


int notify_connect_client(NetworkManager manager, Socket socket) {
    if (0 != create_socket_end_fifos(socket->id)) {
        return -1;
    }

    SocketID old_id = copy_socket_id(socket->id);
    if (NULL == old_id) return -1;
    old_id->dst_port = EMPTY_PORT;
    ip_set_empty(old_id->dst_ip);

    char* connect_fifo_name = get_connect_fifo_read_end_name(old_id);

    if (NULL == connect_fifo_name) {
        free(old_id);
        return -1;
    }

    int result = write_char_to_fifo_name(connect_fifo_name, REQUEST_GRANTED_FIFO);

    free(connect_fifo_name);
    free(old_id);

    return result;
}

int notify_accept_client(NetworkManager manager, Socket socket) {
    SocketID old_id = copy_socket_id(socket->id);
    if (NULL == old_id) return -1;
    old_id->dst_port = EMPTY_PORT;
    ip_set_empty(old_id->dst_ip);

    char* accept_fifo_name = get_accept_fifo_write_end_name(old_id);

    if (NULL == accept_fifo_name) {
        free(old_id);
        return -1;
    }

    char reply_string[MAX_SOCKET_STRING_REPR_SIZE + 1];
    sprintf(reply_string, "%s_%d", (socket->id)->dst_ip, (socket->id)->dst_port);
    reply_string[strlen(reply_string)] = '\0';


    int accept_fifo_fd = open(accept_fifo_name, O_WRONLY | O_NONBLOCK);
    int result;
    if (-1 == accept_fifo_fd) {
        result = -1;
    } else {
        if (write(accept_fifo_fd, reply_string, strlen(reply_string) + 1) == strlen(reply_string) + 1)
            result = 0;
        else result = -1;
        close(accept_fifo_fd);
    }

    free(accept_fifo_name);
    free(old_id);

    return result;
}

// int stopManager(char* ip);