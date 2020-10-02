//
// Created by Ido Yam on 02/05/2020.
//

#ifndef PROJECT_HASHMAP_H
#define PROJECT_HASHMAP_H

#include <stdbool.h>
#include "util_types.h"
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

//typedef struct Key_t *Key;

typedef struct DictElement_t *DictElement;
typedef struct HashMap_t *HashMap;

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

bool dictElementCompare(DictElement d1,DictElement d2);
DictElement dictElementCopy(DictElement d);
void dictElementFree(DictElement d);

typedef SocketID (*copyKey)(SocketID,HashMapErrors *error);
typedef HashMapErrors (*freeKey)(SocketID);
typedef bool (*compareKey)(SocketID ,SocketID);

bool compareKeys(SocketID key1,SocketID key2);
SocketID copyKeyFunction(SocketID key,HashMapErrors *error);
HashMapErrors keyFree(SocketID);

HashMap createHashMap(int size);
int hashCode(HashMap hashMap, SocketID key);
HashMapErrors insertSocket(HashMap hashMap,SocketID key,Socket socket);
Socket getSocket(HashMap hashMap,SocketID key,HashMapErrors *error);
void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error);
void hashDestroy(HashMap hashMap, HashMapErrors *error);
int getHashMapSize(HashMap hashMap);
int getHashMapNumberOfSockets(HashMap hashMap);

SocketID hashMapGetFirst(HashMap hashMap);
void hashMapSetFirst(HashMap hashMap);
SocketID hashMapGetNext(HashMap hashMap);


#define HASH_MAP_FOREACH(sock_id, hashmap) \
    for (SocketID sock_id = hashMapGetFirst(hashmap); \
        sock_id != NULL; \
        sock_id = hashMapGetNext(hashmap))

#endif //PROJECT_HASHMAP_H
