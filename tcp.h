#ifndef TCP_H__
#define TCP_H__

/*
 * This file is for everything related to parsing TCP requests
*/

#include "socket_utils.h"

typedef enum {
    CONNECTED_SOCKET,
    BOUND_ONLY_SOCKET,
    EMPTY_SOCKET,
    INVALID_SOCKET
} SocketState;

typedef enum {
    ACK=1, FIN=2, RST=4, SYN=8
} TCP_FLAGS;

typedef struct {
    int src_port;
    int dst_port;
    int seq_num;
    int ack_num;
    char flags;
    int checksum;
    char* data;
} * TCPPacket;

/**
 * This is an FSM representation of an active connection
 */
typedef enum {
	CLOSED, 		// initial state - does not respond to anything. 
					// passive open (server listen) -> LISTEN
					// active open (client connect) + Send SYN -> SYN_SENT

	LISTEN,			// waiting for a connection.
					// receive client SYN + send SYN+ACK -> SYN_RECEIVED

	SYN_SENT, 		// normally a client state -- sent it's first SYN and waiting for a response
					// receive SYN + send ACK -> SYN_RECEIVED.
					// receive SYN+ACK + send ACK -> ESTABLISHED

	SYN_RECEIVED,	//waiting for final ack to establish connection
					// receive ACK -> ESTABLISHED

	ESTABLISED,		// main "steady" state -- can now exchange data.
					// Close + send FIN -> FIN_WAIT_1
					// receive FIN -> CLOSE_WAIT

	CLOSE_WAIT,		// device has received a close request FIN -- it must wait for the app to generate a matchine request
					// Close + send FIN -> LAST_ACK.

	LAST_ACK,		// a device that's already received a close request and acknowledged, has sent it's own FIN and waits for final ACK.
					// receive ACK for FIN -> CLOSED.

	FIN_WAIT_1,		// waiting for an ACK for it's FIN or a close request from other side.
					// receive ACK for FIN -> FIN_WAIT_2.
					// receive FIN + send ACK -> CLOSING.

	FIN_WAIT_2,		// device has received ACK for it's FIN and is waiting for other side's FIN.
					// receive FIN + send ACK -> TIME_WAIT.

	CLOSING,		// device has received FIN and sent an ACK, but has not received ACK for it's own FIN.
					// receive ACK for FIN -> TIME_WAIT

	TIME_WAIT		// connection is done. Must wait to ensure no connection overlap.
					// after some defined time -> CLOSED (and delete from hashmap).
} TCPState;


/**
 * Converts a string representation into a tcp packet struct (allocated).
 * returns NULL on error.
 */
TCPPacket str_to_tcp(char* s);

/**
 * Converts a tcp packet objet to an allocated string representation (which should be sent over the network).
 * returns NULL on error.
 */
char* tcp_to_str(TCPPacket packet);

/**
 * Prints a log-style representation of a packet to stdout (For debugging).
 */ 
void print_tcp_packet(TCPPacket packet);

/**
 * (for now, a simplified) checksum calculation
 */ 
int calc_checksum(TCPPacket packet);

/**
 * Constructs a packet from given fields. ack num and seq num are updated according to data in socket.
 */
TCPPacket construct_packet(Socket socket, const char* data, char flags, int dst_port);

/**
 * frees all memory associated with packet
 */
void destroy_tcp_packet(TCPPacket packet);

#endif // TCP_H__
