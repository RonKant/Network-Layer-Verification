#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ip.h"
#include "util_types.h"
#include "network.h"


const char* ip_str_repr = "%01X%01X%02X%04X%04X%04X%02X%02X%04X%015s%015s%";


void *xxmalloc(size_t sz){
    void *p;
    p=malloc(sz);
    //   assume(p>0);
    return p;
}

IPPacket str_to_ip(char* s) {
    IPPacket result = (IPPacket)xxmalloc(sizeof(*result));
    int scanned = sscanf(s, ip_str_repr,
                         &(result->version),
                         &(result->ihl),
                         &(result->dscp_and_ecn),
                         &(result->total_length),
                         &(result->id),
                         &(result->flags_and_offset),
                         &(result->ttl),
                         &(result->protocol),
                         &(result->header_checksum),
                         &(result->src_ip),
                         &(result->dst_ip));

    if (scanned != 11) { // if scan failed -- error
        free(result);
        return NULL;
    }
    char* s_data = s + (result->ihl * 32);      //s_data is the pointer of the data section in s. ihl is the length of the header
    result->data = (char*)malloc(strlen(s_data) + 1);

    if (result -> data == NULL) {
        free(result);
        return NULL;
    }

    strcpy(result->data, s_data);

    return result;
}
/*
char* ip_to_str(IPPacket packet) {

    int length_of_msg = packet->total_length*4;
    char* result = (char*) calloc(length_of_msg, sizeof(char));

    if (result == NULL) {
        return NULL;
    }

    int retval = sprintf(result,ip_str_repr,
                         (packet->version),
                         (packet->ihl),
                         (packet->dscp_and_ecn),
                         (packet->total_length),
                         (packet->id),
                         (packet->flags_and_offset),
                         (packet->ttl),
                         (packet->protocol),
                         (packet->header_checksum),
                         (packet->src_ip),
                         (packet->dst_ip)); // we don't use the urgent field

    if (retval < 0) {
        free(result);
        return NULL;
    }
    int header_size = packet->ihl * 32;
    strcat(result + header_size, packet->data);//result+header_size = the begining of the data section

    return result;
}

bool send_packet(Socket socket, char* tcp_as_str, char* ip_dst){
    IPPacket packet = (IPPacket)malloc(sizeof(*packet));
    if (packet == NULL)
        return false;
    //create the packet
    packet->data = tcp_as_str;
    packet->version=0, packet->ihl=0, packet->dscp_and_ecn=0;
    packet->total_length=0;
    packet->id=0, packet->flags_and_offset=0,packet->ttl=0,packet->protocol=0,packet->header_checksum=0;

    // packet->src_ip = socket->id->src_ip;
    //packet->dst_ip = ip_dst;
    strcpy(packet->src_ip,socket->id->src_ip);
    strcpy(packet->dst_ip,ip_dst);

    //write to file that supposes to simulate
    FILE *fp;
    fp = fopen(ip_dst, "w+"); //"sending" the packet to ip_dst
    if (fp == NULL){
        free(packet);
        return false;
    }
    fprintf(fp, ip_to_str(packet));
    fclose(fp);
    free(packet);
    return true;


}

Key getKeyFromIPpacket(IPPacket packet){
    TCPPacket tcp_packet = str_to_tcp(packet->data);
    Key key = (Key)malloc(sizeof(*key));
    if (key == NULL)
        return NULL;

    key->localPort = tcp_packet->src_port;
    key->remotePort = tcp_packet->dst_port;
    free(tcp_packet);

    key->localIp = packet->src_ip;
    key->remoteIp = packet->dst_ip;

    return key;
}

bool handle_ip_message(char* ip_header, HashMap hashMap){
    IPPacket ip_packet = str_to_ip(ip_header);  //translate the string to a packet
    Key key = getKeyFromIPpacket(ip_packet);//finding the right key in the hash map that suit to the packet
    if (key == NULL)
        return false;
    Socket socket = getSocket(hashMap, key);
    free(key);
    if (socket==NULL)
        return false;

    TCPPacket after_handle_packet = handle_packet(socket,str_to_tcp(ip_packet->data),ip_packet->src_ip);
    if (after_handle_packet!=NULL){
        return send_packet(socket,ip_packet->data,ip_packet->dst_ip);
    }

    return true;
}*/
