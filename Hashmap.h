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
    int iterator_index;
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

bool socketCompare(Socket,Socket,HashMapErrors *error);
Socket socketCopy(Socket socket,HashMapErrors *error);
HashMapErrors socketFree(Socket socket);


typedef SocketID (*copyKey)(SocketID,HashMapErrors *error);
typedef HashMapErrors (*freeKey)(SocketID);
typedef bool (*compareKey)(SocketID ,SocketID);

bool compareKeys(SocketID key1,SocketID key2);
SocketID copyKeyFunction(SocketID key,HashMapErrors *error);
HashMapErrors keyFree(SocketID);

HashMap createHashMap();
bool insertSocket(HashMap hashMap,Socket socket);
Socket getSocket(HashMap hashMap,SocketID key);
bool hashmapRemove(HashMap hashMap, SocketID key);
bool hashDestroy(HashMap hashMap);
int getHashMapNumberOfSockets(HashMap hashMap);
bool hasKey(HashMap hashMap, SocketID socketId);

void hashMapSetSize(HashMap hashMap);
int getHashMapSize(HashMap hashMap);
bool HashMapSocketRemoveCond(Socket socket, SocketID socketId);

bool HashMapcmp(HashMap hashMap1, HashMap hashMap2);

SocketID hashMapGetFirst(HashMap hashmap);
SocketID hashMapGetNext(HashMap hashmap);

#define HASH_MAP_FOREACH(sock_id, hashmap) \
    for (SocketID sock_id = hashMapGetFirst(hashmap); \
        sock_id != NULL; \
        sock_id = hashMapGetNext(hashmap))

#endif //PROJECT_HASHMAP_H
