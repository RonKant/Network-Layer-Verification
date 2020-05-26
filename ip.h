//
// Created by tomer on 07-May-20.
//

#ifndef CODE_IP_H
#define CODE_IP_H

#include "Hashmap.h"
#include "util_types.h"
#include "tcp.h"
#include "socket.h"

/*
 * This file is for everything related to parsing IP requests
*/
typedef struct {

    int vesion;
    int ihl;
    int dscp_and_ecn;
    int total_length;
    int id;
    int flags_and_offset;
    int ttl;
    int protocol;
    int header_checksum;
    int src_ip;
    int dst_ip;
    char* data; // the message itself, after the header
} * IPPacket;


#endif //CODE_IP_H
