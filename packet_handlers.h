#ifndef __PACKET_HANDLERS_H
#define __PACKET_HANDLERS_H

#include "util_types.h"

/**
 * This module contains a packet handling function for every possible TCP state
 * to be called by a general function that switches on current socket state
 */

/**
 * General function that makes all necessary changes to socket according to packet received.
 * If return value is not NULL - this is a response packet that should be sent immediately (without queue) to 
 * the address that sent packet (not necessarily the current socket partner).
 * 
 * packet is freed in the process.
 */
TCPPacket handle_packet(Socket socket, TCPPacket packet, char* src_ip);

/**
 * Specific message handling functions for all TCP states
 */
TCPPacket handle_packet_closed(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_listen(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_syn_sent(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_syn_received(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_established(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_close_wait(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_last_ack(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_fin_wait_1(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_fin_wait_2(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_closing(Socket socket, TCPPacket packet, char* src_ip);
TCPPacket handle_packet_time_wait(Socket socket, TCPPacket packet, char* src_ip);

#endif