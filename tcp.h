#ifndef TCP_H__
#define TCP_H__

/*
 * This file is for everything related to parsing TCP requests
*/

#include "socket_utils.h"

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
TCPPacket construct_packet(Socket socket, const char* data, char flags, char dst_port);

/**
 * frees all memory associated with packet
 */
void destroy_tcp_packet(TCPPacket packet);

#endif // TCP_H__