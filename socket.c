#include <stdio.h>

#include "socket.h"

TCPPacket pack_data(Socket socket, char* data) {
	TCPPacket result = (TCPPacket) malloc(sizeof(*result));

	if (result == NULL) {
		return NULL;
	}

	result->src_port = (socket->id)->src_port;
	result->dst_port = (socket->id)->dst_port;
	result->seq_num = socket->next_seq;
	result->ack_num = socket->last_ack;

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