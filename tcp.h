#ifndef TCP_H__
#define TCP_H__

/*
 * This file is for everything related to parsing TCP requests
*/

#include "socket.h"
#include "util_types.h"

typedef enum {
    CWR=1, ECE=2, URG=4, ACK=8, PSH=16, RST=32, SYN=64, FIN=128
} TCP_FLAGS;

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
int calc_checksum(Socket socket, TCPPacket packet);

/**
 * Constructs a packet for acknowledging byets up to it's last ack_num (inclusive)
 */
TCPPacket construct_ack_packet(Socket socket);

/**
 * frees all memory associated with packet
 */
void destroyPacket(TCPPacket packet);

#endif // TCP_H__