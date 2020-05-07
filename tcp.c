#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tcp.h"


const char* tcp_str_repr = "%04X%04X%08X%08X%02X%02X%04X%04X";

TCPPacket str_to_tcp(char* s) {
	TCPPacket result = (TCPPacket)malloc(sizeof(*result));
	int scanned = sscanf(s, tcp_str_repr,
	&(result->src_port),
	&(result->dst_port),
	&(result->seq_num),
	&(result->ack_num),
	&(result->data_offset),
	&(result->flags),
	&(result->window_size),
	&(result->checksum));

    if (scanned != 8) { // if scan failed -- error
        free(result);
        return NULL;
    }

	char* s_data = s + (result->data_offset * 32);
	result->data = (char*)malloc(strlen(s_data) + 1);

    if (result -> data == NULL) {
        free(result);
        return NULL;
    }

	strcpy(result->data, s_data);

	return result;
}

char* tcp_to_str(TCPPacket packet) {

	int header_size = packet->data_offset * 32;

	char* result = (char*) calloc(header_size + strlen(packet->data) + 1, sizeof(char));

    if (result == NULL) {
        return NULL;
    }

	int retval = sprintf(result, tcp_str_repr,
		packet->src_port,
		packet->dst_port,
		packet->seq_num,
		packet->ack_num,
		packet->data_offset,
		packet->flags,
		packet->window_size,
		packet->checksum
		); // we don't use the urgent field

    if (retval < 0) {
        free(result);
        return NULL;
    }
	 
	strcat(result + header_size, packet->data);

	return result;
}

int calc_checksum(Socket socket, TCPPacket packet) {
	int result = 0;
	
	for (int i = 0; i < strlen((socket->id)->src_ip); ++i) {
		result += ((socket->id)->src_ip)[i];
	}

	for (int i = 0; i < strlen((socket->id)->dst_ip); ++i) {
		result += ((socket->id)->dst_ip)[i];
	}

	result += packet->src_port + packet->dst_port + packet->seq_num + packet->ack_num;
	result += packet->data_offset + packet->flags + packet->window_size;

	for (int i = 0; i < strlen(packet->data); ++i) {
		result += (packet->data)[i];
	}

	return result;
}