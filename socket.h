#ifndef __SOCKET_H__
#define __SOCKET_H__

/*
 * This file is for everything related to the socket structure
*/

#include <stdlib.h>
#include <string.h>

#include "network.h"
#include "util_types.h"
#include "tcp.h"

/**
 * Creates a new TCP packet for given raw data according to information stored in socket.
 * (does not modify socket - this should be done in an external calling function)
 * flags are initiated to 0.
 * Returns NULL on error.
 */
TCPPacket pack_data(Socket socket, char* data);

#endif