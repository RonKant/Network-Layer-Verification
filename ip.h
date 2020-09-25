//
// Created by tomer on 07-May-20.
//

#ifndef CODE_IP_H
#define CODE_IP_H

#include "Hashmap.h"
#include "util_types.h"
#include "tcp.h"
#include "packet_handlers.h"
#include "socket.h"

/*
 * This file is for everything related to parsing IP requests
*/
typedef struct {
    int version;
    int ihl;
    int dscp_and_ecn;
    int total_length;
    int id;
    int flags_and_offset;
    int ttl;
    int protocol;
    int header_checksum;
    char src_ip[15];
    char dst_ip[15];
    char* data; // the message itself, after the header
} * IPPacket;

/**
 * Converts a string representation into a ip packet struct (allocated).
 * returns NULL on error.
 */
IPPacket str_to_ip(char* s);

/**
 * Converts a ip packet objet to an allocated string representation
 * returns NULL on error.
 */
char* ip_to_str(IPPacket packet);

/**
 * Sends a packet immediately (skips/after window mechanism)
 */
bool send_packet(Socket socket, char* tcp_as_str, char* ip_dst);

/**
 *
 * @param ip_header -
 * @param hashMap
 * @return true if send the packet, false if fail
 */
bool handle_ip_message(char* ip_header, HashMap hashMap);
void* xcalloc(size_t nelem,size_t elem_size);
char* strcat_t(char* dest, char* source);
unsigned int strlen_t(char* str);

#endif //CODE_IP_H
