/* sea pf --inline */

#include<seahorn/seahorn.h>
#include<stdint.h>
#include<stddef.h>

#include <string.h>

extern int nd(void);
extern int8_t* nd_ptr(void);

#include "Functions.h"
#include "packet_handlers.h"
#include "util_types.h"
#include "socket_utils.h"
#include "tcp.h"


TCPPacket nd_packet() {
    TCPPacket result = (TCPPacket)xmalloc(sizeof(*result));

	result->src_port = nd();
    assume(result->src_port >= 1 && result->src_port <= 5);
	result->dst_port = nd();
    assume(result->dst_port >= 1 && result->dst_port <= 5);

	result->seq_num = nd();
    assume(result->seq_num >= 1 && result->seq_num <= 5);
	result->ack_num = nd();
    assume(result->ack_num >= 1 && result->ack_num <= 5);


	result->flags = nd();
    assume(result->flags >= 0 && result->flags <= (SYN | ACK | FIN | RST));
	result->checksum = calc_checksum(result);

	result->data = NULL;

    return result;
}


SocketID create_socket_id(const char* src_ip, int src_port, const char* dst_ip,  int dst_port) {
    SocketID result = (SocketID)xmalloc(sizeof(*result));
    result->src_port = src_port;
    result->dst_port = dst_port;
    strcpy_t(result->src_ip, src_ip);
    strcpy_t(result->dst_ip, dst_ip);

    return result;
}


void constructPacketVerification() {
    Socket sock = create_new_socket();

    SocketID id1 = create_socket_id("ip1", 8080, "ip2", 8080);
    sassert(id1 != NULL);

    sock->id = id1;

    int flags = nd();
    
    char* data = "acbde";
    
    TCPPacket packet1 = construct_packet(sock, data, flags, (sock->id)->dst_port);


    assume(packet1 != NULL);
    sassert(packet1->flags == flags);
    sassert(packet1->src_port == (sock->id)->src_port);
    sassert(packet1->dst_port == (sock->id)->dst_port);
    sassert(packet1->seq_num == sock->seq_of_first_send_window);

    sassert(packet1->data== data);
}

void checksumVerification() {
    /* No other checksum than the correct one can have the packet accepted. */
    Socket sock = create_new_socket();
    SocketID id1 = create_socket_id("ip1", 8080, "ip2", 8080);
    sock->id = id1;
    int flags = nd();
    char* data = "acbde";
        

    TCPPacket packet1 = nd_packet();

    int old_checksum = packet1->checksum;
    packet1->checksum = nd();
    assume(packet1->checksum != old_checksum);
    TCPPacket reply = handle_packet(sock, packet1, "1234123412341234", NULL);
    sassert(reply == NULL);
}

void relevantAckVerification() {
    /* Receiving an ack for a relevant seq dequeues the bytes. */

    Socket sock = create_new_socket();

    int old_seq = nd();
    assume(old_seq > 50);
    assume(old_seq < 60);

    sock->seq_of_first_send_window = old_seq;

    for (char i = 'a'; i < 'a' + 10; ++i) {
        enqueue(sock->send_window, i);
        sassert(QueueSize(sock->send_window) == i - 'a' + 1);
    }

    sassert(QueueSize(sock->send_window) == 10);

    sassert(sock->seq_of_first_send_window == old_seq);

    int bytes_acked = nd();
    assume(bytes_acked > 0 && bytes_acked <= 10);
    int ack_num = old_seq + bytes_acked;

    update_socket_with_ack_packet(sock, ack_num);

    sassert(sock->seq_of_first_send_window == old_seq + bytes_acked);
    sassert(QueueSize(sock->send_window) == 10 - bytes_acked);
    
}

void irrelevantAckVerification() {
    /* Receiving an ack for an irrelevant seq does not change the queue size */


    Socket sock = create_new_socket();

    int old_seq = nd();
    assume(old_seq > 50);
    assume(old_seq < 60);

    sock->seq_of_first_send_window = old_seq;

    for (char i = 'a'; i < 'a' + 10; ++i) {
        enqueue(sock->send_window, i);
        sassert(QueueSize(sock->send_window) == i - 'a' + 1);
    }

    sassert(QueueSize(sock->send_window) == 10);

    sassert(sock->seq_of_first_send_window == old_seq);

    int ack_num = nd();
    assume(ack_num <= old_seq && ack_num > 0);

    update_socket_with_ack_packet(sock, ack_num);

    sassert(sock->seq_of_first_send_window == old_seq);
    sassert(QueueSize(sock->send_window) == 10);

}

void receiveWindowVerification() {
    // on socket create, recv window should be empty.
    Socket sock = create_new_socket();

    sassert(sock->max_recv_window_size == MAX_WINDOW_SIZE);

    for (int i = 0; i < sock->max_recv_window_size; ++i) {
        sock->recv_window[i] = 'e';
    }

    int idx = nd();
    assume(idx >= 0 && idx < sock->max_recv_window_size);
    sassert(sock->recv_window_isvalid[idx] == false);
    sassert(sock->recv_window[idx] == 'e');

    // simulate a state in which recv window is: FTTTTTT...TTTFFFFFFFF
    // update recv window should do nothing since first byte has to be received too.

    // int already_received_idx = nd();
    // assume(already_received_idx >= 0 && already_received_idx < sock->max_recv_window_size);
    int already_received_idx = 5;

    for (int i = already_received_idx; i > 0; i--) {
        sock->recv_window_isvalid[i] = true;
        sock->recv_window[i] = 'a';
    }

    int equallity_idx = nd();
    assume(equallity_idx >= 0 && equallity_idx < sock->max_recv_window_size);
    // int equallity_idx = 3;

    bool is_valid_old_content = sock->recv_window_isvalid[equallity_idx];
    char recv_window_old_content = sock->recv_window[equallity_idx];
    update_recv_window(sock);


    sassert(sock->recv_window_isvalid[equallity_idx] == is_valid_old_content);
    sassert(sock->recv_window[equallity_idx] == recv_window_old_content);



}

int main() {
    // constructPacketVerification();
    // checksumVerification();
    // relevantAckVerification();
    // irrelevantAckVerification();

    receiveWindowVerification();

    return 0;
}