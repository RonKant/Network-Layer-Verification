#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "connection_queue.h"
#include "util_types.h"

#define TIMEOUT_THRESHOLD 100

struct conn_queue_t {
    Socket* sockets;
    int current_size;
    int max_size;
};

// remove timed out connections
void kick_idle_connections(ConnQueue q) {
    time_t current_time = time(NULL);
    for (int i = 0; i < q->max_size; ++i) {
        if ((q->sockets)[i] != NULL) {
            Socket s = (q->sockets)[i];
            if (s->state != ESTABLISED && (((long)current_time) - ((long) s->creation_time) > TIMEOUT_THRESHOLD)) {
                connRemove(q, s->id->dst_ip, s->id->dst_port);
            }
        }
    }
}

ConnQueue connCreateQueue(int max_size) {
    ConnQueue q = (ConnQueue)malloc(sizeof(*q));
    if (q == NULL) {
        return NULL;
    }
    q->sockets = (Socket*)malloc(sizeof(*(q->sockets)) * max_size);
    if (q->sockets == NULL) {
        free(q);
        return NULL;
    }
    for (int i = 0; i < max_size; ++i) {
        (q->sockets)[i] = NULL;
    }
    q->max_size = max_size;
    q->current_size = 0;
    return q;
}

int connQueueSize(ConnQueue q) {
    kick_idle_connections(q);
    return q->current_size;
}

bool connEnQueue(ConnQueue q, Socket conn) {
    kick_idle_connections(q);

    bool success = false;
    for (int i = 0; i < q->max_size; ++i) {
        if ((q->sockets)[i] == NULL) {
            (q->sockets)[i] = conn;
            success = true;
            break;
        }
    }

    if (!success) {return false;}

    q->current_size++;
    return true;
}

bool connIsEmpty(ConnQueue q) {
    kick_idle_connections(q);
    return (q->current_size == 0);
}

Socket connGetEstablished(ConnQueue q) {
    Socket result;
    for (int i = 0; i < q->max_size; ++i) {
        if ((q->sockets)[i] != NULL) {
            result = (q->sockets)[i];
            if (result->state == ESTABLISED) {
                (q->sockets)[i] = NULL;
                q->current_size--;
                return result;
            }
        }
    }
    return NULL;
}

bool connInQueue(ConnQueue q, char* dst_ip, int dst_port) {
    kick_idle_connections(q);
    return (getConnByAddr(q, dst_ip, dst_port) != NULL);
}

Socket connRemove(ConnQueue q, char* dst_ip, int dst_port) {
    for (int i = 0; i < q->max_size; ++i) {
        if ((q->sockets)[i] != NULL) {
            Socket s = (q->sockets)[i];
            if (strcmp(s->id->dst_ip, dst_ip) == 0 && s->id->dst_port == dst_port) {
                (q->sockets)[i] = NULL;
                q->current_size--; // important
                return s;
            }
        }
    }
    return NULL;
}

Socket getConnByAddr(ConnQueue q, char* dst_ip, int dst_port) {
    Socket result = connRemove(q, dst_ip, dst_port);
    if (result == NULL) {
        return NULL;
    } else {
        connEnQueue(q, result);
        return result;
    }
}

bool connIsFull(ConnQueue q) {
    kick_idle_connections(q);
    return q->current_size == q->max_size;
}