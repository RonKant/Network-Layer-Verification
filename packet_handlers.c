#include <stdlib.h>
#include <time.h>

#include "packet_handlers.h"
#include "tcp.h"
#include "socket.h"
#include "network.h"
#include "util_types.h"
#include "connection_queue.h"

TCPPacket handle_packet(Socket socket, TCPPacket packet, char* src_ip) {

    if ((packet->flags & RST) && (!(socket->state == LISTEN))) {
        close_socket(socket);
        destroyPacket(packet);
        return NULL;
    }

    TCPPacket response_packet;
    switch(socket->state) {
        case CLOSED:
            response_packet = handle_packet_closed(socket, packet, src_ip);
        break;
        case LISTEN:
            response_packet = handle_packet_listen(socket, packet, src_ip);
        break;
        case SYN_SENT:
             response_packet = handle_packet_syn_sent(socket, packet, src_ip);
        break;
        case SYN_RECEIVED:
             response_packet = handle_packet_syn_received(socket, packet, src_ip);
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
    free(packet);
    return response_packet;
}

TCPPacket generate_rst_packet(Socket socket) {
    TCPPacket result = pack_data(socket, "", calc_next_send_seq(socket)); // take an empty data packet and change it's fields.
                                                                                // seq_num does not matter for RST only packet.
    if (result == NULL) {
        return NULL;
    }

    result->flags |= RST;
    result->checksum = calc_checksum(socket, result); // calculate checksum again since we changed the flags

    return result;
}

TCPPacket handle_packet_closed(Socket socket, TCPPacket packet, char* src_ip) {return NULL;} // does nothing since socket does not even have a port
TCPPacket handle_packet_listen(Socket socket, TCPPacket packet, char* src_ip) {


    // This case should not happen since when a connection is created, it's id is changed to match the partner
    // Code left here for backup

    // if (packet->flags & ACK) { // ACK packet for some non established sub connection
    //     Socket conn = getConnByAddr(socket->connections, src_ip, packet->src_port);
    //     if (conn == NULL) {
    //         return generate_rst_packet(socket);
    //     } // ACK arrived for non existing sub connection
    //     else {
    //         handle_packet(conn, packet, src_ip);
    //         return NULL;
    //     } // let existing subconnection handle the packet
    // }

    if (!(packet->flags & SYN)) {return generate_rst_packet(socket);}
    // trying to create a new connection

    if (connQueueSize(socket->connections) == socket->max_connections) {return generate_rst_packet(socket);} // full

    srand(time(NULL));
    int my_seq = rand();
    socket->seq_of_first_send_window = my_seq;

    Socket new_conn = create_new_socket();

    if (new_conn == NULL) {
        return generate_rst_packet(socket);
    }

    update_socket_id(new_conn->id, socket->id->src_ip, socket->id->src_port, src_ip, packet->src_port); // TODO: this should update Tomer's dictionary key
                                                                                                        // TODO: also check return value (this pair might already exist)
    new_conn->state = SYN_RECEIVED;
    new_conn->seq_of_first_recv_window = packet->seq_num+1;

    TCPPacket result = pack_data(socket, "", calc_next_send_seq(socket)); // take an empty data packet and change it's fields.
                                                                            // will be a SYN+ACK packet
    if (result == NULL) {
        free(new_conn);
        return generate_rst_packet(socket);
    }

    connEnQueue(socket->connections, new_conn); // should not fail since we checked it is not full

    result->flags |= SYN | ACK;
    result->ack_num = new_conn->seq_of_first_recv_window;
    result->checksum = calc_checksum(socket, result); // calculate checksum again since we changed the flags

    return result;

}
TCPPacket handle_packet_syn_sent(Socket socket, TCPPacket packet, char* src_ip) {
    if (!(packet->flags & SYN)) { return generate_rst_packet(socket);}
    //if ((!(packet->flags & SYN)) || (!(packet->flags & ACK))) { return generate_rst_packet(socket);}

    socket->seq_of_first_recv_window = packet->seq_num + 1;

    TCPPacket result = construct_ack_packet(socket);
    if (result == NULL) {
        return generate_rst_packet(socket);
    }

    if (packet->flags & ACK) {
        if (packet->ack_num <= socket->seq_of_first_send_window) {return generate_rst_packet(socket);} // ack should match the syn seq sent
        socket->state = ESTABLISED;
    } else {
        socket->state = SYN_RECEIVED;
    }

    return result;
}

TCPPacket handle_packet_syn_received(Socket socket, TCPPacket packet, char* src_ip) {
    if (!(packet->flags & ACK)) { return generate_rst_packet(socket);}

    if (packet->ack_num <= socket->seq_of_first_send_window) { return generate_rst_packet(socket);} // ack should match the syn seq sent

    socket->state = ESTABLISED;
    return NULL;
}

/**
 * inserts data from accepted data packet into an ESTABLISHED socket recv_window
 */
void receiveNewData(Socket socket, TCPPacket packet) {
    for (int i = 0; i < strlen(packet->data); ++i) { // take only missing bytes into receive window
                int current_byte_seq = packet->seq_num + i;
                int place_in_window = current_byte_seq - socket->seq_of_first_recv_window;
                if (place_in_window >= 0 && place_in_window <= socket->recv_window_size) {
                    if ((socket->recv_window_isvalid)[place_in_window] == false) {
                        (socket->recv_window)[place_in_window] = (packet->data)[i];
                        (socket->recv_window_isvalid)[place_in_window] = true;
                    }
                }
            }

    update_recv_window(socket);
}

TCPPacket handle_packet_established(Socket socket, TCPPacket packet, char* src_ip) {
    if (packet->flags & FIN) {
        TCPPacket ack_for_fin = construct_ack_packet(socket);
        if (ack_for_fin == NULL) { // error
            close_socket(socket);
            return generate_rst_packet(socket);
        } else {
            ack_for_fin->ack_num = packet->seq_num + 1;
            ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
            socket->state = CLOSE_WAIT;
            return ack_for_fin;
        }
    } else {
        receiveNewData(socket, packet);
        send_ack(socket);
        return NULL;
        // state stays ESTABLISHED
    }
}

TCPPacket handle_packet_close_wait(Socket socket, TCPPacket packet, char* src_ip) {return NULL;} // ignore all messages


TCPPacket handle_packet_last_ack(Socket socket, TCPPacket packet, char* src_ip) {
    if (!(packet->flags & ACK)) {return generate_rst_packet(socket);}

    if (packet->ack_num <= socket->seq_of_first_send_window) { return generate_rst_packet(socket);} // ack should match the fin seq sent

    socket->state = CLOSED;
    return NULL;
}

TCPPacket handle_packet_fin_wait_1(Socket socket, TCPPacket packet, char* src_ip) {
    bool received_ack_for_own_fin = false;
    if ((packet->flags & ACK) && (packet->ack_num > socket->seq_of_first_send_window)) {
        socket->state = FIN_WAIT_2; // wait for receiving FIN, next - check if current packet also has a fin.
        received_ack_for_own_fin = true;
    }

    if (packet->flags & FIN) { // acknowledge the fin
        TCPPacket ack_for_fin = construct_ack_packet(socket);
        if (ack_for_fin == NULL) { // error
            close_socket(socket);
            return generate_rst_packet(socket);
        } else {
            ack_for_fin->ack_num = packet->seq_num + 1;
            ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
        }

        if (received_ack_for_own_fin) {
            socket->state = TIME_WAIT; // received both ack for own fin and second fin -> finish
        } else {
            socket->state = CLOSING; // have yet to receive it's own fin ack
        }
        return ack_for_fin;
    }
    
    return NULL;
}

TCPPacket handle_packet_fin_wait_2(Socket socket, TCPPacket packet, char* src_ip) {
    if (packet->flags & FIN) { // acknowledge the fin
        TCPPacket ack_for_fin = construct_ack_packet(socket);
        if (ack_for_fin == NULL) { // error
            close_socket(socket);
            return generate_rst_packet(socket);
        } else {
            ack_for_fin->ack_num = packet->seq_num + 1;
            ack_for_fin->checksum = calc_checksum(socket, ack_for_fin);
        }

        socket->state = TIME_WAIT; // received both ack for own fin and second fin -> finish
        return ack_for_fin;
    } else {
        return NULL;
    }
}
TCPPacket handle_packet_closing(Socket socket, TCPPacket packet, char* src_ip) {
    if ((packet->flags & ACK) && (packet->ack_num > socket->seq_of_first_send_window)) {
        socket->state = TIME_WAIT;
        return NULL;
    } else {
        return generate_rst_packet(socket);
    }
}
TCPPacket handle_packet_time_wait(Socket socket, TCPPacket packet, char* src_ip) { return NULL; } // ignore all messages