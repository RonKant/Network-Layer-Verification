#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include<seahorn/seahorn.h>
#include<stdint.h>
#include<stddef.h>

#include "Functions.h"
#include "socket_utils.h"
#include "tcp.h"

extern int nd(void);


// typedef struct {
//     int src_port;
//     int dst_port;
//     int seq_num;
//     int ack_num;
//     char flags;
//     int checksum;
//     char* data;
// } * TCPPacket;

const char* tcp_str_repr = "%05d%05d%010d%010d%c%10d%s";
const char* tcp_str_repr_no_data = "%05d%05d%010d%010d%c%10d";

TCPPacket construct_packet(Socket socket, char data, char flags, int dst_port) {
	TCPPacket result = (TCPPacket)xmalloc(sizeof(*result));
	// if (NULL == result) return NULL;
	// if (NULL == result->data) {
	// 	free(result);
	// 	return NULL;
	// }

	result->src_port = (socket->id)->src_port;
	result->dst_port = dst_port;

	result->seq_num = socket->seq_of_first_send_window;
	result->ack_num = socket->seq_of_first_recv_window;

	result->flags = flags;
	result->checksum = calc_checksum(result);

	result->data = data;

    return result;
}

// TCPPacket str_to_tcp(char* s) {
// 	TCPPacket result = (TCPPacket)malloc(sizeof(*result));
// 	if (NULL == result) return NULL;

// 	(result->data)[0] = '\0';

// 	int scanned = sscanf(s, tcp_str_repr_no_data,
// 		&(result->src_port),
// 		&(result->dst_port),
// 		&(result->seq_num),
// 		&(result->ack_num),
// 		&(result->flags),
// 		&(result->checksum)
// 	);

// 	if (scanned != 6) {
// 		free(result);
// 		return NULL;
// 	}

// 	if (strlen(s) > 41) {
// 		int data_length = strlen(s) - 41;
// 		memcpy(result->data, s + 41, data_length);
// 		(result->data)[data_length] = '\0';
// 	}

// 	return result;
// }

// char* tcp_to_str(TCPPacket packet) {
// 	char* result = (char*)malloc(41 + strlen(packet->data) + 1);
// 	if (NULL == result) return NULL;

// 	int retval = sprintf(result, tcp_str_repr,
// 		packet->src_port,
// 		packet->dst_port,
// 		packet->seq_num,
// 		packet->ack_num,
// 		packet->flags,
// 		packet->checksum,
// 		packet->data
// 	);

//     if (retval < 0) {
//         free(result);
//         return NULL;
//     }

// 	return result;
// }

void print_tcp_packet(TCPPacket packet) {
	printf("TCP PACKET:\n\
			\tsrc_port: %d\n\
			\tdst_port: %d\n\
			\tseq_num: %d\n\
			\tack_num: %d\n\
			\tflags: %c\n\
			\tchecksum: %d\n\
			\tdata: %c\n",
			
			packet->src_port,
			packet->dst_port,
			packet->seq_num,
			packet->ack_num,
			packet->flags,
			packet->checksum,
			packet->data);
}

int calc_checksum(TCPPacket packet) {
	return packet->src_port + packet->dst_port + packet->seq_num + packet->ack_num + packet->flags;
}


void destroy_tcp_packet(TCPPacket packet) {
	if (packet == NULL) return;
	// free(packet->data);
	free(packet);
}


