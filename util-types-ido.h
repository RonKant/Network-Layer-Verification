//
// Created by Ido Yam on 03/10/2020.
//

#ifndef PROJECT_UTIL_TYPES_IDO_H
#define PROJECT_UTIL_TYPES_IDO_H

#include <stdbool.h>

#include "byte_queue.h"
#include "network.h"
#include "time.h"

// from connection_queue.h

typedef struct conn_queue_t* ConnQueue;


// from tcp.h

#define PHANTOM_BYTE "b"
#define PACKET_SIZE_LIMIT 512
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
    // after some defined time -> CLOSED.
} TCPState;

typedef struct {
    SocketID id;
    TCPState state;
    time_t creation_time;


    /**
     * The send window stores bytes that have been sent but not yet acknowledged.
     * bytes leave it's "left" end when acknowledged.
     */
    char* send_window; // contains bytes sent but now acknowledged.
    int send_window_size;
    int max_send_window_size;
    int seq_of_first_send_window; // seq number of first byte in send_window;
    bool* send_window_acks;

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

    ByteQueue future_send_bytes; // contains bytes that will be sent in the future.
    ByteQueue received_bytes_queue; // the user can get their data from here - bytes already acknowledged.

    int max_connections;
    ConnQueue connections; // contains pending connections for listening sockets.
} * Socket;
#endif //PROJECT_UTIL_TYPES_IDO_H