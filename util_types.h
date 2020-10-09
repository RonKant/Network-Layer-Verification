#ifndef __UTIL_TYPES_H__
#define __UTIL_TYPES_H__

#include <stdbool.h>
#include <time.h>

#include "network.h"
#include "array_queue.h"

#define MAX_WINDOW_SIZE 1024

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
    // Queue connections; // contains pending connections for listening sockets.

	clock_t last_send_clock;
    clock_t creation_time;
    clock_t time_since_fin_sent;

} * Socket;


#endif