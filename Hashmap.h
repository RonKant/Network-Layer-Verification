//
// Created by Ido Yam on 02/05/2020.
//

#ifndef PROJECT_HASHMAP_H
#define PROJECT_HASHMAP_H

#include <stdbool.h>


typedef struct Node_t *Node;

typedef struct Key_t *Key;

typedef struct HashMap_t *HashMap;

typedef void *Socket;
typedef Socket (*socketCopy)(Socket);
typedef bool (*compareKey)(Key,Key);
typedef Key (*copyKey)(Key);

HashMap createHashMap(int size,socketCopy socketCopyFunction,compareKey compareKeyFunction,copyKey copyKeyFunction);
int hashCode(HashMap hashMap, Key key);
void insertSocket(HashMap hashMap,Key key,Socket socket);
Socket getSocket(HashMap hashMap,Key key);



#endif //PROJECT_HASHMAP_H
