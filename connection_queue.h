
#ifndef __CONN_QUEUE_H
#define __CONN_QUEUE_H

#include "util_types.h"

/**
 * represents a queue data structure for incomming connections for listening
 */
ConnQueue connCreateQueue(int max_size);

/**
 * return value is true if and only if operation successful
 */
bool connEnQueue(ConnQueue q, Socket conn);

bool connIsEmpty(ConnQueue q);

/**
 * pops an established connection from the queue and returns it;
 * returns NULL if no connections are established
 */
Socket connGetEstablished(ConnQueue q);

int connQueueSize(ConnQueue q);

// check if connectino already in queue
bool connInQueue(ConnQueue q, char* dst_ip, int dst_port);

// return the socket if exists, otherwise NULL - does not remove from queue
Socket getConnByAddr(ConnQueue q, char* dst_ip, int dst_port);

// same as above but removes from queue
Socket connRemove(ConnQueue q, char* dst_ip, int dst_port);

bool connIsFull(ConnQueue q);

#endif