#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "fifo_utils.h"
#include "packet_handlers.h"
#include "network.h"
#include "network_manager/network_manager.h"

TCPPacket handle_packet(Socket socket, TCPPacket packet, char* src_ip, NetworkManager manager) {
    // printf("packet to port %d (recevied by port %d), with flags %d, in state %d.\n",
    //     packet->dst_port, (socket->id)->src_port, packet->flags, socket->state);
    if ((packet->flags & RST) && (!(socket->state == LISTEN))) {
        socket->state = CLOSED;
        return NULL;
    }

    if ((!(socket->state == LISTEN)) && (packet->src_port != (socket->id)->dst_port)) {
        return NULL;
    }

    TCPPacket response_packet;
    switch(socket->state) {
        case CLOSED:
            response_packet = handle_packet_closed(socket, packet, src_ip);
        break;
        case LISTEN:
            response_packet = handle_packet_listen(socket, packet, src_ip, manager);
        break;
        case SYN_SENT:
             response_packet = handle_packet_syn_sent(socket, packet, src_ip, manager);
        break;
        case SYN_RECEIVED:
             response_packet = handle_packet_syn_received(socket, packet, src_ip, manager);
        break;
        case ESTABLISED:
            response_packet = handle_packet_established(socket, packet, src_ip);
        break;
        case CLOSE_WAIT:
            response_packet = handle_packet_close_wait(socket, packet, src_ip);
        break;
        case LAST_ACK:
            response_packet = handle_packet_last_ack(socket, packet, src_ip);
        break;
        case FIN_WAIT_1:
            response_packet = handle_packet_fin_wait_1(socket,  packet, src_ip);
        break;
        case FIN_WAIT_2:
            response_packet = handle_packet_fin_wait_2(socket, packet, src_ip);
        break;
        case CLOSING:
            response_packet = handle_packet_closing(socket, packet, src_ip);
        break;
        case TIME_WAIT:
            response_packet = handle_packet_time_wait(socket, packet, src_ip);
        break;
    }

    // printf("\tmoved to state %d.\n", socket->state);

    return response_packet;
}

TCPPacket generate_rst_packet(Socket socket, int dst_port) {
    TCPPacket result = construct_packet(socket, "", RST, dst_port);

    if (result == NULL) {
        return NULL;
    }

    return result;
}

TCPPacket handle_packet_closed(Socket socket, TCPPacket packet, char* src_ip) {return NULL;} // does nothing since socket does not even have a port
TCPPacket handle_packet_listen(Socket socket, TCPPacket packet, char* src_ip, NetworkManager manager) {
    if (!(packet->flags & SYN)) {return NULL;}
    // trying to create a new connection

    if (socket->max_connections == 0) {
        return generate_rst_packet(socket, packet->src_port);
    } // full

    SocketID new_conn_id = copy_socket_id(socket->id);
    if (NULL == new_conn_id) return NULL;

    strcpy(new_conn_id->dst_ip, src_ip);
    new_conn_id->dst_port = packet->src_port;

    Socket new_conn = create_new_socket();
    if (NULL == new_conn) {
        destroy_socket_id(new_conn_id);
        return NULL;
    }

    if (0 != create_socket_end_fifos(new_conn_id)) {
        destroy_socket(new_conn);
        destroy_socket_id(new_conn_id);
        return NULL;
    }

    new_conn->id = new_conn_id;

    srand(time(NULL));
    int my_seq = rand();

    new_conn->seq_of_first_send_window = my_seq;

    new_conn->state = SYN_RECEIVED;
    new_conn->seq_of_first_recv_window = packet->seq_num+1;

    if (insertSocket(manager->sockets, new_conn_id, new_conn) != HASH_MAP_SUCCESS) {
        unlink_socket_fifos(new_conn);
        destroy_socket(new_conn);
        return NULL;
    } // should not break iteration since not in queue for each loop.
    if (enqueue(socket->connections, new_conn) == false) {
        hashmapRemove(manager->sockets, new_conn_id, NULL);
        unlink_socket_fifos(new_conn);
        destroy_socket(new_conn);
        return NULL;
    }

    socket->max_connections--;

    return NULL;

}
TCPPacket handle_packet_syn_sent(Socket socket, TCPPacket packet, char* src_ip, NetworkManager manager) {
    if (!(packet->flags & SYN)) { return NULL;}
    //if ((!(packet->flags & SYN)) || (!(packet->flags & ACK))) { return generate_rst_packet(socket);}

    if (!(packet->flags & ACK) || packet->ack_num != socket->seq_of_first_send_window + 1)
        return NULL;


    int result= notify_connect_client(manager, socket);

    if (result == 0) {
        printf("\tConnect successful.\n");
        socket->seq_of_first_recv_window = packet->seq_num + 1;
        socket->state = ESTABLISED;
    }


    return NULL;
}

TCPPacket handle_packet_syn_received(Socket socket, TCPPacket packet, char* src_ip, NetworkManager manager) {
    if (!(packet->flags & ACK)) { return NULL;}

    if (!(packet->ack_num == socket->seq_of_first_send_window + 1))
        return NULL;

    if (0 == notify_accept_client(manager, socket))
        socket->state = ESTABLISED;

    return NULL;
}

/**
 * inserts data from accepted data packet into an ESTABLISHED socket recv_window
 */
// void receiveNewData(Socket socket, TCPPacket packet) {
//     for (int i = 0; i < strlen(packet->data); ++i) { // take only missing bytes into receive window
//                 int current_byte_seq = packet->seq_num + i;
//                 int place_in_window = current_byte_seq - socket->seq_of_first_recv_window;
//                 if (place_in_window >= 0 && place_in_window <= socket->recv_window_size) {
//                     if ((socket->recv_window_isvalid)[place_in_window] == false) {
//                         (socket->recv_window)[place_in_window] = (packet->data)[i];
//                         (socket->recv_window_isvalid)[place_in_window] = true;
//                     }
//                 }
//             }

//     update_recv_window(socket);
// }

TCPPacket handle_packet_established(Socket socket, TCPPacket packet, char* src_ip) {
    // if (packet->flags & FIN) {
    //     TCPPacket ack_for_fin = construct_ack_packet(socket);
    //     if (ack_for_fin == NULL) { // error
    //         close_socket(socket);
    //         return generate_rst_packet(socket);
    //     } else {
    //         ack_for_fin->ack_num = packet->seq_num + 1;
    //         ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
    //         socket->state = CLOSE_WAIT;
    //         return ack_for_fin;
    //     }
    // } else {
    //     receiveNewData(socket, packet);
    //     send_ack(socket);
    //     return NULL;
    //     // state stays ESTABLISHED
    // }
    return NULL;
}

TCPPacket handle_packet_close_wait(Socket socket, TCPPacket packet, char* src_ip) {return NULL;} // ignore all messages


TCPPacket handle_packet_last_ack(Socket socket, TCPPacket packet, char* src_ip) {
    // if (!(packet->flags & ACK)) {return generate_rst_packet(socket);}

    // if (packet->ack_num <= socket->seq_of_first_send_window) { return generate_rst_packet(socket);} // ack should match the fin seq sent

    // socket->state = CLOSED;
    return NULL;
}

TCPPacket handle_packet_fin_wait_1(Socket socket, TCPPacket packet, char* src_ip) {
    // bool received_ack_for_own_fin = false;
    // if ((packet->flags & ACK) && (packet->ack_num > socket->seq_of_first_send_window)) {
    //     socket->state = FIN_WAIT_2; // wait for receiving FIN, next - check if current packet also has a fin.
    //     received_ack_for_own_fin = true;
    // }

    // if (packet->flags & FIN) { // acknowledge the fin
    //     TCPPacket ack_for_fin = construct_ack_packet(socket);
    //     if (ack_for_fin == NULL) { // error
    //         close_socket(socket);
    //         return generate_rst_packet(socket);
    //     } else {
    //         ack_for_fin->ack_num = packet->seq_num + 1;
    //         ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
    //     }

    //     if (received_ack_for_own_fin) {
    //         socket->state = TIME_WAIT; // received both ack for own fin and second fin -> finish
    //     } else {
    //         socket->state = CLOSING; // have yet to receive it's own fin ack
    //     }
    //     return ack_for_fin;
    // }
    
    return NULL;
}

TCPPacket handle_packet_fin_wait_2(Socket socket, TCPPacket packet, char* src_ip) {
    // if (packet->flags & FIN) { // acknowledge the fin
    //     TCPPacket ack_for_fin = construct_ack_packet(socket);
    //     if (ack_for_fin == NULL) { // error
    //         close_socket(socket);
    //         return generate_rst_packet(socket);
    //     } else {
    //         ack_for_fin->ack_num = packet->seq_num + 1;
    //         ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
    //     }

    //     socket->state = TIME_WAIT; // received both ack for own fin and second fin -> finish
    //     return ack_for_fin;
    // } else {
    //     return NULL;
    // }

    return NULL;
}
TCPPacket handle_packet_closing(Socket socket, TCPPacket packet, char* src_ip) {
    // if ((packet->flags & ACK) && (packet->ack_num > socket->seq_of_first_send_window)) {
    //     socket->state = TIME_WAIT;
    //     return NULL;
    // } else {
    //     return generate_rst_packet(socket);
    // }
    return NULL;
}
TCPPacket handle_packet_time_wait(Socket socket, TCPPacket packet, char* src_ip) { return NULL; } // ignore all messages