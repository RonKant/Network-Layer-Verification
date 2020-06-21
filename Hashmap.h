//
// Created by Ido Yam on 02/05/2020.
//

#ifndef PROJECT_HASHMAP_H
#define PROJECT_HASHMAP_H

#include <stdbool.h>

#include "util_types.h"

typedef struct Node_t *Node;

typedef struct
Key_t{
    int localPort;
    char* localIp;
    int remotePort;
    char* remoteIp;
} *Key;

typedef struct HashMap_t *HashMap;


typedef Socket (*socketCopy)(Socket);
typedef void (*freeSocket)(Socket);
typedef bool (*compareSocket)(Socket,Socket);

typedef Key (*copyKey)(Key);
typedef void (*freeKey)(Key);
typedef bool (*compareKey)(Key,Key);

//HashMap createHashMap(int size,socketCopy,freeSocket,compareSocket,copyKey,freeKey,compareKey);
int hashCode(HashMap hashMap, Key key);
void insertSocket(HashMap hashMap,Key key,Socket socket);
Socket getSocket(HashMap hashMap,Key key);
void hashmapRemove(HashMap hashMap, Key key);
void hashDestroy(HashMap hashMap);

#endif //PROJECT_HASHMAP_H