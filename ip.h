//
// Created by tomer on 07-May-20.
//

#ifndef CODE_IP_H
#define CODE_IP_H

#include "Hashmap.h"
#include "util_types.h"
#include "tcp.h"
#include "packet_handlers.h"

/*
 * This file is for everything related to parsing IP requests
*/
typedef struct {


    long long total_length;
    char src_ip[17];
    char dst_ip[17];
    int header_checksum;
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

IPPacket create_ip_packet(char* src, char* dst, char* data);

void destroy_ip_packet(IPPacket ipPacket);

int calc_ip_checksum(IPPacket packet);

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
void int_to_str(char* str, int size_of_str, long n);

#endif //CODE_IP_H