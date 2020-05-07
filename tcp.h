#ifndef __TCP_H__
#define __TCP_H__

/*
 * This file is for everything related to parsing TCP requests
*/

#include "socket.h"
#include "util_types.h"

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
 * (for now, a simplified) checksum calculation
 */ 
int calc_checksum(Socket socket, TCPPacket packet);

#endif