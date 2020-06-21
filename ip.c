#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ip.h"
#include "util_types.h"


const char* ip_str_repr = "%01X%01X%02X%04X%04X%04X%02X%02X%04X%08s%08s%";

IPPacket str_to_ip(char* s) {
    IPPacket result = (IPPacket)malloc(sizeof(*result));
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
    int data_offset = result->flags_and_offset%265;
    char* s_data = s + (data_offset * 32);
    result->data = (char*)malloc(strlen(s_data) + 1);

    if (result -> data == NULL) {
        free(result);
        return NULL;
    }

    strcpy(result->data, s_data);

    return result;
}

char* ip_to_str(IPPacket packet) {

    int data_offset = packet->flags_and_offset%265;
    int header_size = data_offset * 32;

    char* result = (char*) calloc(header_size + strlen(packet->data) + 1, sizeof(char));

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

    strcat(result + header_size, packet->data);

    return result;
}
/*
void create_key(char* s, HashMap hash_map){
    IPPacket ip = str_to_ip(s);
    TCPPacket tcp = str_to_tcp(ip-> data);
    SocketID s_id = (SocketID)malloc(sizeof(*s_id));
    s_id->src_ip = ip->src_ip;
    s_id->dst_ip = ip->dst_ip;
    s_id->dst_port = tcp->dst_port;
    s_id->src_port = tcp->src_port;
    //SocketID s_id = SocketID(tcp->src_port,tcp->dst_port,ip->src_ip,src->dst_ip );
}
*/


bool is_contain_full(Key key, HashMap hash_map,char* data){
    Socket s = getSocket(hash_map,key);
    if (s==NULL || s->state == CLOSED){
        return false;
    }
    pack_data(s,data,1);
    return true;
}

bool is_contain_half(Key key, HashMap hash_map,char* data){
    Socket s = getSocket(hash_map,key);
    if (s==NULL || s->state != LISTEN){
        return false;
    }
    pack_data(s,data,0);
    return true;
}

bool send_packet(Socket socket, char* tcp_as_str, char* ip_dst){
    IPPacket packet = (IPPacket)malloc(sizeof(*packet));
    packet->data = tcp_as_str;
    packet->version=0, packet->ihl=0, packet->dscp_and_ecn=0;
    packet->total_length=0;
    packet->id=0, packet->flags_and_offset=0,packet->ttl=0,packet->protocol=0,packet->header_checksum=0;

    packet->src_ip = socket->id->src_ip;
    packet->dst_ip = ip_dst;

    FILE *fp;
    fp = fopen(ip_dst, "w+");
    fprintf(fp, ip_to_str(packet));
    fclose(fp);
    return true;


}

Key getKeyFromIPpacket(IPPacket packet){
    TCPPacket tcp_packet = str_to_tcp(packet->data);
    Key key = (Key)malloc(sizeof(*key));

    key->localPort = tcp_packet->src_port;
    key->remotePort = tcp_packet->dst_port;
    free(tcp_packet);

    key->localIp = packet->src_ip;
    key->remoteIp = packet->dst_ip;

    return key;
}

bool handle_ip_message(char* ip_header, HashMap hashMap){
    IPPacket ip_packet = str_to_ip(ip_header);
    Key key = getKeyFromIPpacket(ip_packet);
    Socket socket = getSocket(hashMap, key);
    free(key);
    if (socket==NULL)
        return false;

    TCPPacket after_handle_packet = handle_packet(socket,str_to_tcp(ip_packet->data),ip_packet->src_ip);
    if (after_handle_packet!=NULL){
        send_packet(socket,ip_packet->data,ip_packet->dst_ip);
    }

    return true;
}
