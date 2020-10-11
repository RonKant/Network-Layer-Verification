//
// Created by Ido Yam on 02/05/2020.
//

#ifndef PROJECT_HASHMAP_H
#define PROJECT_HASHMAP_H

#include <stdbool.h>
#include "util_types.h"
#include <stdio.h>
#include <stdlib.h>
#include "array_queue.h"
//typedef struct Key_t *Key;

typedef struct HashMap_t *HashMap;


struct HashMap_t{
    int size;
    int number_of_sockets;
    Socket table[5];
    SocketID socket_id[5];
    Socket ghost_v;
    bool ghost_has_v;
};


typedef enum {
    HASH_MAP_SUCCESS,
    HASH_MAP_ALLOCATION_FAIL,
    HASH_MAP_NULL_ARGUMENT,
    HASH_MAP_KEY_EXIST,
    HASH_MAP_SOCKET_NOT_FOUND,
    HASH_MAP_ERROR
} HashMapErrors;

typedef bool (*compareSocket)(Socket,Socket,HashMapErrors *error);
typedef HashMapErrors (*freeSocket)(Socket);
typedef Socket (*copySocket)(Socket,HashMapErrors *error);


bool compareKeys(SocketID key1,SocketID key2);

HashMap createHashMap();
bool insertSocket(HashMap hashMap,Socket socket);
Socket getSocket(HashMap hashMap,SocketID key);
bool hashmapRemove(HashMap hashMap, SocketID key);
bool hashDestroy(HashMap hashMap);
int getHashMapNumberOfSockets(HashMap hashMap);
bool hasKey(HashMap hashMap, SocketID socketId);

int getHashMapSize(HashMap hashMap);

#define HASH_MAP_FOREACH(sock_id, hashmap) \
    for (SocketID sock_id = hashMapGetFirst(hashmap); \
        sock_id != NULL; \
        sock_id = hashMapGetNext(hashmap))

#endif //PROJECT_HASHMAP_H
