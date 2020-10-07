#ifndef __UTIL_TYPES_H__
#define __UTIL_TYPES_H__

#include <stdbool.h>
#include <time.h>

#include "byte_queue.h"
#include "network.h"
#include "array_queue.h"
// from connection_queue.h

typedef struct conn_queue_t* ConnQueue;

#define MAX_WINDOW_SIZE 1024


// from tcp.h

// #define PHANTOM_BYTE "b"
// #define PACKET_SIZE_LIMIT 512
// typedef struct {
// 	int src_port;
// 	int dst_port;
// 	int seq_num;
// 	int ack_num;
// 	int data_offset; // header size in multiples of 32
// 	char flags; // without NS
// 	int window_size; // bytes you can receive starting from ack_num
// 	int checksum;
// 	char* data; // the message itself, after the header
// } * TCPPacket;

// from socket.h

typedef enum {
    CONNECTED_SOCKET,
    BOUND_ONLY_SOCKET,
    EMPTY_SOCKET,
    INVALID_SOCKET
} SocketState;

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
    Queue send_window; // TODO: change to Tomer's queue.
    int seq_of_first_send_window;

    /**
     * The recv window stores bytes that have been received but not yet acknowledged.
     * It may contain "holes" indicated by recv_window_isvalid.
     * a continuous sequence of bytes may leave this array from it's "left end" after acknowledge was sent for them.
     */
    char* recv_window; // contains non-continuous byte sequence received.
    int max_recv_window_size;
    int seq_of_first_recv_window; // seq number of first byte in recv_window - seq number of next byte to receive;
    bool* recv_window_isvalid; // each index indicates whether the corresponding byte has been received;

    int max_connections;
    Queue connections; // contains pending connections for listening sockets.

	clock_t last_send_clock;
    clock_t creation_time;

} * Socket;


// From fifo_utils.h

#endif