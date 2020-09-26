#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#include "fifo_utils.h"
#include "socket_utils.h"
#include "socket.h"


void init_empty_socket_id(SocketID sock_id) {
    sock_id->dst_ip = EMPTY_IP;
    sock_id->src_ip = EMPTY_IP;
    sock_id->dst_port = EMPTY_PORT;
    sock_id->src_port = EMPTY_PORT;
}

bool is_socket_connected(SocketID sock_id) {
    return (sock_id->dst_ip != EMPTY_IP
            && sock_id->dst_port != EMPTY_PORT
            && sock_id->src_ip != EMPTY_IP
            && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_bound_only(SocketID sock_id) {
    return (sock_id->dst_ip == EMPTY_IP
        && sock_id->dst_port == EMPTY_PORT
        && sock_id->src_ip != EMPTY_IP
        && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_empty(SocketID sock_id) {
    return (sock_id->dst_ip == EMPTY_IP
        && sock_id->dst_port == EMPTY_PORT
        && sock_id->src_ip == EMPTY_IP
        && sock_id->src_port == EMPTY_PORT);
}

SocketState get_socket_state(SocketID sock_id) {
    if (is_socket_connected(sock_id)) return CONNECTED_SOCKET;
    if (is_socket_bound_only(sock_id)) return BOUND_ONLY_SOCKET;
    if (is_socket_empty(sock_id)) return EMPTY_SOCKET;
    return INVALID_SOCKET;
}

/**
 * Note: Shallow copy.
 */
SocketID copy_socket_id(SocketID sock_id) {
    SocketID result = (SocketID)malloc(sizeof(*result));
    if (NULL == result) return NULL;

    result->src_ip = sock_id->src_ip;
    result->src_port = sock_id->src_port;
    result->dst_ip = sock_id->dst_ip;
    result->dst_port = sock_id->dst_port;

    return result;
}

Socket create_bound_socket(SocketID sock_id) {

    Socket result = create_new_socket();

    if (NULL == result) return NULL;

    result->state = CLOSED;

    result->id = copy_socket_id(sock_id);
    if (result->id == NULL) {
        destroy_socket(result);
        return NULL;
    }


    // this might not belong here (since client opens listen fifos only later - it is maybe better to try and open them every time in nmanager loop)

    // init listen fifo
    // char* listen_fifo_read_end_name = get_listen_fifo_read_end_name(sock_id);
    // char* listen_fifo_write_end_name = get_listen_fifo_write_end_name(sock_id);

    // if (listen_fifo_read_end_name == NULL
    //     || listen_fifo_write_end_name == NULL) {
    //         free(listen_fifo_write_end_name); free(listen_fifo_read_end_name);
    //         destroy_socket(result);
    //         return NULL;
    //     }
    // printf("1.\n");
    // result->listen_fifo_read_end = open(listen_fifo_read_end_name, O_RDONLY | O_NONBLOCK);
    // printf("errno: %d.\n", errno);
    // printf("2.\n");
    // result->listen_fifo_write_end = open(listen_fifo_write_end_name, O_WRONLY);
    // printf("errno: %d.\n", errno);
    // printf("3 - (%d, %d).\n", result->listen_fifo_read_end, result->listen_fifo_write_end);
    // if (result->listen_fifo_read_end == -1 || result->listen_fifo_write_end == -1) {
    //     free(listen_fifo_write_end_name); free(listen_fifo_read_end_name);
    //     destroy_socket(result);
    //     return NULL;
    // }

    // free(listen_fifo_write_end_name); free(listen_fifo_read_end_name);

    return result;
}

/**
 * Frees all memory of socket, and unlinks all fifos.
 */
void destroy_socket(Socket socket) {
    close_socket_fifos(socket);
    if (socket->id != NULL) {
        unlink_socket_fifos(socket);
    }
    if (socket->send_window != NULL) destroyQueue(socket->send_window, NULL);
    if (socket->recv_window != NULL) free(socket->recv_window);
    if (socket->recv_window_isvalid != NULL) free(socket->recv_window_isvalid);
    if (socket->connections != NULL)  destroyQueue(socket->connections, NULL);
    if (socket->id != NULL) free(socket->id);
    free(socket);
}

Socket create_new_socket(){
	Socket s = (Socket)malloc(sizeof(*s));

	if (s == NULL) return NULL;

	s->id = NULL;
	s->listen_fifo_read_end = -1;
	s->listen_fifo_write_end = -1;
	s->accept_fifo_write_end = -1;
	s->out_fifo_read_end = -1;
	s->in_fifo_write_end = -1;
	s->end_fifo_read_end = -1;
	s->end_fifo_write_end = -1;

	s->send_window = NULL;
	s->recv_window = NULL;
	s->recv_window_isvalid = NULL;

	s->connections = NULL;

	s->send_window = createQueue_g(sizeof(char));
	s->recv_window = (char*)malloc(sizeof(*s->recv_window) * MAX_WINDOW_SIZE);
	s->recv_window_isvalid = (bool*)malloc(sizeof(*s->recv_window_isvalid) * MAX_WINDOW_SIZE);

	// s->connections = createQueue_g(sizeof(char) * MAX_SOCKET_STRING_REPR_SIZE);

	if (NULL == s->send_window
		|| NULL == s->recv_window
		|| NULL == s->recv_window_isvalid) {
			destroy_socket(s);
			return NULL;
		}

	return s;
}

void destroy_socket_id(SocketID sock_id) {
    if (sock_id == NULL) return;

    free(sock_id->src_ip);
    free(sock_id->dst_ip);

    free(sock_id);
}