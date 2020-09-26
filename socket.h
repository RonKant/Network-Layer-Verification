#ifndef __SOCKET_H__
#define __SOCKET_H__

/*
 * This file is for everything related to the socket structure
*/

#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "util_types.h"
#include "tcp.h"
#include "ip.h"

/**
 * Creates a new TCP packet for given raw data according to information stored in socket.
 * seq_num is the sequence number of the packet created.
 * (does not modify socket - this should be done in an external calling function)
 * flags are initiated to 0.
 * Returns NULL on error.
 */
TCPPacket pack_data(Socket socket, char* data, int seq_num);

/**
 * Tries to send an ack packet for current ack num of socket
 * return value indicates success.
 */
bool send_ack(Socket socket);

/**
 * Calculates the appropriate sequential number for a new packet
 */
int calc_next_send_seq(Socket socket);



/**
 * Closes the socket while clearing all message windows. (e.g. received RST from partner).
 * Does not send FIN.
 */
void close_socket(Socket socket);

/**
 * clears the longest sequence of received data from the start of receive window and transfers it to user buffer.
 * updates all relevant socket fields.
 */
void update_recv_window(Socket socket);

void update_socket_id(SocketID to_update, char* src_ip, int port);

#endif