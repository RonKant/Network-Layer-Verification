#include <stdio.h>
#include <stdlib.h>

#include "tcp.h"
#include "socket.h"

void print_tcp_packet(TCPPacket packet) {
    printf("ports: %d %d\nnums: %d %d\ndata offset: %d\nflags: %d\nwsize: %d\ncksum: %d\ndata: %s\n", packet->src_port, packet->dst_port,
        packet->seq_num, packet->ack_num, packet->data_offset, packet->flags, packet->window_size, packet->checksum, packet->data);
}

int main() {
    TCPPacket p = malloc(sizeof(*p));
    p->src_port = 8080;
    p->dst_port = 8081;
    p->seq_num = 44;
    p->ack_num = 55;
    p->data_offset = 2;
    p->flags = 3;
    p->window_size = 4096;
    p->checksum = 99;
    p->data = "hello world";
    
    printf("%s\n", tcp_to_str(p));
    printf("%s\n", tcp_to_str(p) + 64);

    TCPPacket p2 = str_to_tcp(tcp_to_str(p));
    print_tcp_packet(p2);

    Socket sock = (Socket) malloc(sizeof(*sock));
    sock->id = (SocketID)malloc(sizeof(*(sock->id)));
    (sock->id)->src_port = 11; (sock->id)->dst_port = 22; (sock->id)->src_ip = "127.0.0.1"; (sock->id)->dst_ip = "127.0.0.2";
    sock->next_seq = 55; sock->last_ack = 66; sock->state = ESTABLISED;
    sock->send_window_size = 4096; sock->recv_window_size = 4096;

    TCPPacket p3 = pack_data(sock, "cool verification");
    printf("%s\n", tcp_to_str(p3));
    print_tcp_packet(str_to_tcp(tcp_to_str(p3)));

    return 0;
}