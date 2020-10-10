/* sea pf --inline */

#include<seahorn/seahorn.h>
#include<stdint.h>
#include<stddef.h>

extern int nd(void);
extern int8_t* nd_ptr(void);

#include "Functions.h"
#include "util_types.h"
#include "socket_utils.h"
#include "tcp.h"

SocketID create_socket_id(const char* src_ip, int src_port, const char* dst_ip,  int dst_port) {
    SocketID result = (SocketID)xmalloc(sizeof(*result));
    result->src_port = src_port;
    result->dst_port = dst_port;
    strcpy_t(result->src_ip, src_ip);
    strcpy_t(result->dst_ip, dst_ip);

    return result;
}

int main() {

    Socket sock = create_new_socket();

    // SocketID id1 = create_socket_id("ip1", 8080, "ip2", 8080);
    // sassert(id1 != NULL);

    // sock->id = id1;

    TCPPacket packet1 = construct_packet(sock, "", ACK, 8080);

    assume(packet1 != NULL);
    sassert(packet1->flags == ACK);

    return 0;
}