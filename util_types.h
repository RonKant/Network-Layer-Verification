#ifndef __UTIL_TYPES_H__
#define __UTIL_TYPES_H__

// from tcp.h

typedef struct {
	int src_port;
	int dst_port;
	int seq_num;
	int ack_num;
	int data_offset; // header size in multiples of 32
	char flags; // without NS
	int window_size; // bytes you can receive starting from ack_num
	int checksum;
	char* data; // the message itself, after the header
} * TCPPacket;

// from socket.h

typedef enum {
	LISTEN, // until receiving syn
	SYNC_SENT, // until receiving syn+ack
	ESTABLISED,
	FIN_WAIT_CLIENT1, // when first side initiates closing, it goes here after FIN
	FIN_WAIT_CLIENT2, // first side goes here after ACK from side 2 on it's FIN (until received FIN)
	FIN_WAIT_SERVER1, // second side goes here when receiving first FIN of side 1 (sends fin)
	FIN_WAIT_SERVER2, // server goes here when sending fin to client (until receiving last ack, then close)
} TCPState;

typedef struct {
	SocketID id;
	int next_seq; // next seq to send (first byte of next packet)
	int last_ack; // last acknowledge number I sent
	TCPState state;
	int send_window_size;
	int recv_window_size;
} * Socket;

#endif