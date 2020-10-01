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
    if (NULL == sock_id) return INVALID_SOCKET;
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

    result->src_ip = NULL;
    result->dst_ip = NULL;

    if (sock_id->src_ip != NULL) {
        result->src_ip = (char*)malloc(strlen(sock_id->src_ip) + 1); // TODO: change to our strlen
        if (NULL == result->src_ip) {
            free(result);
            return NULL;
        }
        strcpy(result->src_ip, sock_id->src_ip);
    }

    if (sock_id->dst_ip != NULL) {
        result->dst_ip = (char*)malloc(strlen(sock_id->dst_ip) + 1); // TODO: change to our strlen
        if (NULL == result->dst_ip) {
            free(result->src_ip);
            free(result);
            return NULL;
        }
        strcpy(result->dst_ip, sock_id->dst_ip);
    }

    result->src_port = sock_id->src_port;
    result->dst_port = sock_id->dst_port;

    return result;
}

Socket create_bound_socket(SocketID sock_id) {

    Socket result = create_new_socket();

    if (NULL == result) return NULL;

    result->state = CLOSED;

    result->id = copy_socket_id(sock_id);
        printf("5. %p\n", result->id->dst_ip);

    if (result->id == NULL) {
        destroy_socket(result);
        return NULL;
    }

    return result;
}

bool compare_socket(void* s1, void* s2) {
    if (s1 == NULL) return s2 == NULL;
    return compareKeys((SocketID)(((Socket)s1)->id), (SocketID)(((Socket)s2)->id));
}

/**
 * Frees all memory of socket, without touching it's fifos.
 */
void destroy_socket(void* socket) {
    Socket socket_to_destroy = (Socket) socket;
    if (socket_to_destroy->send_window != NULL) destroyQueue(socket_to_destroy->send_window, NULL);
    if (socket_to_destroy->recv_window != NULL) free(socket_to_destroy->recv_window);
    if (socket_to_destroy->recv_window_isvalid != NULL) free(socket_to_destroy->recv_window_isvalid);
    if (socket_to_destroy->connections != NULL)  destroyQueue(socket_to_destroy->connections, NULL);
    if (socket_to_destroy->id != NULL) destroy_socket_id(socket_to_destroy->id);
    free(socket_to_destroy);
}

void destroy_socket_fifos(Socket socket) {
    close_socket_fifos(socket);
    if (socket->id != NULL) {
        unlink_socket_fifos(socket);
    }
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

    s->max_recv_window_size = MAX_WINDOW_SIZE;
    s->recv_window_size = s->max_recv_window_size; // redundant

	s->connections = NULL;

	s->send_window = createQueue_g(NULL, NULL, NULL); // TODO: change to actual functions
	s->recv_window = (char*)malloc(sizeof(*(s->recv_window)) * MAX_WINDOW_SIZE);
	s->recv_window_isvalid = (bool*)malloc(sizeof(*(s->recv_window_isvalid)) * MAX_WINDOW_SIZE);

	// s->connections = createQueue_g(sizeof(char) * MAX_SOCKET_STRING_REPR_SIZE);

	if (NULL == s->send_window
		|| NULL == s->recv_window
		|| NULL == s->recv_window_isvalid) {
			destroy_socket(s);
			return NULL;
		}

	return s;
}

void* copy_socket(void* to_copy) {
    if (to_copy == NULL) return NULL;

    Socket to_copy_socket = (Socket)to_copy;

    Socket s = (Socket)malloc(sizeof(*s));

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

    // copy now:

    s->id = copy_socket_id(to_copy_socket->id);
    if (NULL == s->id) {
        destroy_socket(s);
        return NULL;
    }

    s->send_window = copyQueue(to_copy_socket->send_window);
    if (NULL == s->send_window) {
        destroy_socket(s);
        return NULL;
    }

    s->recv_window = (char*)malloc(sizeof(char) * to_copy_socket->max_recv_window_size);
    if (s->recv_window == NULL) {
        destroy_socket(s);
        return NULL;
    }

    for (int i = 0; i < to_copy_socket->max_recv_window_size; ++i) {
        (s->recv_window)[i] = (to_copy_socket->recv_window)[i];
    }

    s->recv_window_isvalid = (bool*)malloc(sizeof(bool) * to_copy_socket->max_recv_window_size);
    if (NULL == s->recv_window_isvalid) {
        destroy_socket(s);
        return NULL;
    }

    for (int i = 0; i < to_copy_socket->max_recv_window_size; ++i) {
        (s->recv_window_isvalid)[i] = (to_copy_socket->recv_window_isvalid)[i];
    }

    if (NULL != to_copy_socket->connections) {
        s->connections = copyQueue(to_copy_socket->connections);
        if (NULL == s->connections) {
            destroy_socket(s);
            return NULL;
        }
    }

    s->state = to_copy_socket->state;
    s->listen_fifo_read_end = to_copy_socket->listen_fifo_read_end;
    s->listen_fifo_write_end = to_copy_socket->listen_fifo_write_end;
    s->accept_fifo_write_end = to_copy_socket->accept_fifo_write_end;

    s->out_fifo_read_end = to_copy_socket->out_fifo_read_end;
    s->in_fifo_write_end = to_copy_socket->in_fifo_write_end;
    s->end_fifo_read_end = to_copy_socket->end_fifo_read_end;
    s->end_fifo_write_end = to_copy_socket->end_fifo_write_end;

    s->seq_of_first_send_window = to_copy_socket->seq_of_first_send_window;
    s->recv_window_size = to_copy_socket->recv_window_size;
    s->max_recv_window_size = to_copy_socket->max_recv_window_size;
    s->seq_of_first_recv_window = to_copy_socket->seq_of_first_recv_window;

    s->max_connections = to_copy_socket->max_connections;

    return s;
}

void destroy_socket_id(SocketID sock_id) {
    if (sock_id == NULL) return;

    free(sock_id->src_ip);
    free(sock_id->dst_ip);

    free(sock_id);
}