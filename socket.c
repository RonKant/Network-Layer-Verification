#include <stdio.h>

#include "socket.h"

TCPPacket pack_data(Socket socket, char* data, int seq_num) {
	TCPPacket result = (TCPPacket) malloc(sizeof(*result));

	if (result == NULL) {
		return NULL;
	}

	result->src_port = (socket->id)->src_port;
	result->dst_port = (socket->id)->dst_port;
	result->seq_num = seq_num;
	result->ack_num = socket->seq_of_first_recv_window;

	result->flags = 0; // to be edited externally.
	result->window_size = socket->recv_window_size;
	result->checksum = 0; // calculate it later;
	result->data_offset = 0;
	result->data = ""; // we first calculate raw header length for correct data offset

	char* str_repr = tcp_to_str(result);
	if (str_repr == NULL) {
		free(result);
		return NULL;
	}
	int padded_length = strlen(str_repr) + ((32 - (strlen(str_repr) % 32)) % 32);
	free(str_repr);
	str_repr = NULL;

	result->data_offset = padded_length; // we now have the correct one
	result->data = data;

	result->checksum = calc_checksum(socket, result); // calculate the correct checksum

	return result;
}

/**
 * checks whether it is possible to remove acknowledged bytes from send window
 * checks whether it is possible to put more bytes into send window (from queue)
 */
void update_send_window(Socket socket) {
	// count the amount of continuoes acknowledged sends from the start
	int acknowledged_sends = 0;
	for (; acknowledged_sends < socket->send_window_size; ++acknowledged_sends) {
		if ((socket->send_window_acks)[acknowledged_sends] == false) {
			break;
		}
	}

	// move the send window backwards along with it's acks.
	for (int i = 0; i < socket->send_window_size - acknowledged_sends; ++i) {
		(socket->send_window)[i] = (socket->send_window)[acknowledged_sends + i];
		(socket->send_window_acks)[i] = (socket->send_window_acks)[acknowledged_sends+i];
	}

	for (int i = socket->send_window_size - acknowledged_sends; i < socket->send_window_size; ++i) { // invalidate some acks
		(socket->send_window_acks)[i] = false;
	}

	socket->seq_of_first_send_window += acknowledged_sends;

	// now try to fit as many additional bits as possible to end of send_window from queue
	char* dequeued = deQueueString(socket->future_send_bytes, socket->max_send_window_size - socket->send_window_size);
	if (dequeued == NULL) {
		return;
	}

	for (int i = 0; i < strlen(dequeued); ++i) {
		(socket->send_window)[socket->send_window_size ++] = dequeued[i++];
	}
}

/**
 * Tries to send a packet matching to unacknowledged bits 
 */
bool try_to_send(Socket socket) {
	// first erase acknowledged sequence from beginning of send window
	update_send_window(socket);

	char sendbuff[PACKET_SIZE_LIMIT + 1];
	memcpy(sendbuff, socket->send_window, PACKET_SIZE_LIMIT);
	sendbuff[PACKET_SIZE_LIMIT] = '\0';

	TCPPacket to_send = pack_data(socket, sendbuff, socket->seq_of_first_send_window);
	if (to_send == NULL) {
		return false; // failed to send
	}

	//send_packet(socket, to_send);

	return true; // success
}

void update_recv_window(Socket socket) {
	int valid_recvs = 0;
	for (; valid_recvs < socket->recv_window_size; ++valid_recvs) {
		if ((socket->recv_window_isvalid)[valid_recvs] == false) {
			break;
		}
	}

	// transfer bytes into user accessible buffer
	for (int i = 0; i < valid_recvs; ++i) {
		if (! enQueue(socket->received_bytes_queue, (socket->recv_window)[i])) {
			valid_recvs = i;
			break;
		}
	}

	// try to acknowledge the valid sequence of receives.
	// Even if sending fails - it's fine since ack will be sent next time some data arrives.
	socket->seq_of_first_recv_window += valid_recvs; // update the ack number
	send_ack(socket); // continute only if ack sent successfuly 
	for (int i = 0; i < socket->recv_window_size - valid_recvs; ++i) {
		(socket->recv_window[i]) = (socket->recv_window)[valid_recvs+i];
		(socket->recv_window_isvalid)[i] = (socket->recv_window_isvalid)[valid_recvs+i];
	}

	for (int i = socket->recv_window_size - valid_recvs; i < socket->recv_window_size; ++i) { // invalidate some cells
		(socket->recv_window_isvalid)[i] = false;
	}
}


int calc_next_send_seq(Socket socket) {
	return socket->seq_of_first_send_window + socket->send_window_size + queueSize(socket->future_send_bytes);
}

bool send_ack(Socket socket) {
	TCPPacket to_send = construct_ack_packet(socket);
	if (to_send == NULL) {
		return false; // failed
	}

	return true;//send_packet(socket, to_send);
}

bool send_packet1(Socket socket, TCPPacket packet){
	return true;
}

void close_socket(Socket socket){}

Socket create_new_socket(){
	Socket s = (Socket)malloc(sizeof(*s));
	return s;
}

typedef struct {
	SocketID id;
	TCPState state;

	int listen_fifo_read_end;
	int listen_fifo_write_end;
	int accept_fifo_write_end; // need these always open?

	int out_fifo_read_end;
	int in_fifo_write_end;
	int end_fifo_read_end;
	int end_fifo_write_end;

	/**
	 * The send window stores bytes that have been sent but not yet acknowledged.
	 * bytes leave it's "left" end when acknowledged.
	 */
	ByteQueue send_window; // TODO: change to Tomer's queue.
	int seq_of_first_send_window;

	/**
	 * The recv window stores bytes that have been received but not yet acknowledged.
	 * It may contain "holes" indicated by recv_window_isvalid.
	 * a continuous sequence of bytes may leave this array from it's "left end" after acknowledge was sent for them.
	 */
	char* recv_window; // contains non-continuous byte sequence received.
	int recv_window_size;
	int max_recv_window_size;
	int seq_of_first_recv_window; // seq number of first byte in recv_window - seq number of next byte to receive;
	bool* recv_window_isvalid; // each index indicates whether the corresponding byte has been received;

	int max_connections;
	ConnQueue connections; // contains pending connections for listening sockets.
							// TODO Change to Tomer's Queue.
} * Socket; // change to Socket after integration TODO