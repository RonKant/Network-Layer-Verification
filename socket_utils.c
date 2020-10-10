#include <errno.h>
#include <stdio.h>
#include <unistd.h>


#include<seahorn/seahorn.h>
#include<stdint.h>
#include<stddef.h>

#include "Functions.h"
#include "fifo_utils.h"
#include "socket_utils.h"
#include "array_queue.h"
//#include<seahorn/seahorn.h>



bool is_empty_ip(char* ip) {
    return ip[0] == 'e';
}

void ip_set_empty(char* ip) {
    for (int i = 0; i < MAX_IP_LENGTH; ++i) ip[i] = 'e';
    ip[MAX_IP_LENGTH] = '\0';
}

void init_empty_socket_id(SocketID sock_id) {
    ip_set_empty(sock_id->dst_ip);
    ip_set_empty(sock_id->src_ip);
    sock_id->dst_port = EMPTY_PORT;
    sock_id->src_port = EMPTY_PORT;
}

bool is_socket_connected(SocketID sock_id) {
    return (! is_empty_ip(sock_id->dst_ip)
            && sock_id->dst_port != EMPTY_PORT
            && ! is_empty_ip(sock_id->src_ip)
            && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_bound_only(SocketID sock_id) {
    return (is_empty_ip(sock_id->dst_ip)
        && sock_id->dst_port == EMPTY_PORT
        && ! is_empty_ip(sock_id->src_ip)
        && sock_id->src_port != EMPTY_PORT);
}

bool is_socket_empty(SocketID sock_id) {
    return (is_empty_ip(sock_id->dst_ip)
        && sock_id->dst_port == EMPTY_PORT
        && is_empty_ip(sock_id->src_ip)
        && sock_id->src_port == EMPTY_PORT);
}

SocketState get_socket_state(SocketID sock_id) {
    if (NULL == sock_id) return INVALID_SOCKET;
    if (is_socket_connected(sock_id)) return CONNECTED_SOCKET;
    if (is_socket_bound_only(sock_id)) return BOUND_ONLY_SOCKET;
    if (is_socket_empty(sock_id)) return EMPTY_SOCKET;
    return INVALID_SOCKET;
}

SocketID copy_socket_id(SocketID sock_id) {
    SocketID result = (SocketID)xmalloc(sizeof(*result));
    if (NULL == result) return NULL;

    ip_set_empty(result->src_ip);
    ip_set_empty(result->dst_ip);

    strcpy_t(result->src_ip, sock_id->src_ip);
    strcpy_t(result->dst_ip, sock_id->dst_ip);


    result->src_port = sock_id->src_port;
    result->dst_port = sock_id->dst_port;

    return result;
}

Socket create_bound_socket(SocketID sock_id) {

    Socket result = create_new_socket();

    if (NULL == result) return NULL;

    result->state = CLOSED;

    if (0 != create_socket_end_fifos(sock_id)) {
        destroy_socket(result);
        return NULL;
    }

    result->id = sock_id;

    return result;
}

bool compare_socket_id(SocketID id1, SocketID id2) {
    if(id1 == id2)
    return true;

    return id1->dst_port == id2->dst_port && !strcmp_t(id1->src_ip,id2->src_ip) &&
            id1->src_port == id2->src_port && !strcmp_t(id1->dst_ip,id2->dst_ip);
}

bool compare_socket(void* s1, void* s2) {
    if (s1 == NULL) return s2 == NULL;
    return compare_socket_id((SocketID)(((Socket)s1)->id), (SocketID)(((Socket)s2)->id));
}

/**
 * Frees all memory of socket, without touching it's fifos.
 */
void destroy_socket(void* socket) {
    Socket socket_to_destroy = (Socket) socket;
    if (socket_to_destroy->send_window != NULL) QueueDestroy(socket_to_destroy->send_window);
    if (socket_to_destroy->recv_window != NULL) free(socket_to_destroy->recv_window);
    if (socket_to_destroy->recv_window_isvalid != NULL) free(socket_to_destroy->recv_window_isvalid);
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
	Socket s = (Socket)xmalloc(sizeof(*s));
    sassert(s != NULL);
	// if (s == NULL) return NULL;

	s->id = NULL;
	s->listen_fifo_read_end = -1;
	s->listen_fifo_write_end = -1;
	s->accept_fifo_write_end = -1;
	s->out_fifo_read_end = -1;
	s->in_fifo_write_end = -1;
	s->end_fifo_read_end = -1;
	s->end_fifo_write_end = -1;
    sassert(s != NULL);

	s->send_window = NULL;
	s->recv_window = NULL;
	s->recv_window_isvalid = NULL;
    sassert(s != NULL);

    s->max_recv_window_size = MAX_WINDOW_SIZE;
    sassert(s != NULL);

	s->send_window = QueueCreate();
	s->recv_window = (char*)xmalloc(sizeof(*(s->recv_window)) * MAX_WINDOW_SIZE);
	s->recv_window_isvalid = (bool*)xmalloc(sizeof(*(s->recv_window_isvalid)) * MAX_WINDOW_SIZE);
    sassert(s != NULL);

    for (int i = 0; i < s->max_recv_window_size; ++i) {
        (s->recv_window_isvalid)[i] = false;
    }
    sassert(s != NULL);

    s->seq_of_first_recv_window = 0;
    s->seq_of_first_send_window = 0;
    sassert(s != NULL);

    // s->last_send_clock = clock();
    // s->creation_time = clock();

    s->last_send_clock = 0;
    s->creation_time = 0;
    sassert(s != NULL);

	// if (NULL == s->send_window
	// 	|| NULL == s->recv_window
	// 	|| NULL == s->recv_window_isvalid) {
	// 		destroy_socket(s);
	// 		return NULL;
	// 	}

    for (int i = 0; i < s->max_recv_window_size; ++i) {
        (s->recv_window)[i] = false;
    }
    sassert(s != NULL);

	return s;
}

void destroy_socket_id(SocketID sock_id) {
    if (sock_id == NULL) return;

    free(sock_id);
}


void update_recv_window(Socket socket) {
    int valid_recvs = 0;
    for (; valid_recvs < socket->max_recv_window_size; ++valid_recvs) {
        if ((socket->recv_window_isvalid)[valid_recvs] == false) break;
    }

    char* socket_recv_fifo_name = get_socket_recv_fifo_name(socket->id);
    if (NULL == socket_recv_fifo_name) return;

    int bytes_written_to_user = write_string_to_fifo_name(
        socket_recv_fifo_name, socket->recv_window, valid_recvs
    );

    free(socket_recv_fifo_name);

    if (bytes_written_to_user == -1) return;

    socket->seq_of_first_recv_window += bytes_written_to_user;
    for (int i = 0; i < socket->max_recv_window_size - bytes_written_to_user; ++i) {
        (socket->recv_window)[i] = (socket->recv_window)[i + bytes_written_to_user];
        (socket->recv_window_isvalid)[i] = (socket->recv_window_isvalid)[i + bytes_written_to_user];
    }

    for (int i = socket->max_recv_window_size - bytes_written_to_user; i < socket->max_recv_window_size; ++i) {
        (socket->recv_window_isvalid)[i] = false;
    }
}