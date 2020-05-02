//
// Created by Ido Yam on 02/05/2020.
//

#include "Hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct Node_t{
    Key key;
    Socket socket; //represents the socket
    Node next;
    //bool inUse;
};

struct Key_t{
    int localPort;
    char* localIp;
    int remotePort;
    char* remoteIp;
};

struct HashMap_t{
    int size;
    Node* table;
    socketCopy socketCopyFunction;
    compareKey compareKeyFunction;
    copyKey copyKeyFunction;

};

HashMap createHashMap(int size,socketCopy socketCopyFunction,compareKey compareKeyFunction,copyKey copyKeyFunction){
    HashMap hashMap = malloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = malloc(sizeof(hashMap->table)*size);
    for(int i=0; i<size; i++){
        hashMap->table[i] = NULL;
    }
    hashMap->socketCopyFunction=socketCopyFunction;
    hashMap->compareKeyFunction =compareKeyFunction;
    hashMap->copyKeyFunction=copyKeyFunction;
    return hashMap;
}
int hashCode(HashMap hashMap, Key key){
    return key->localPort%hashMap->size;
}
void insertSocket(HashMap hashMap,Key key,Socket socket){
    int pos = hashCode(hashMap,key);
    Node posList = hashMap->table[pos];
    Node newNode = malloc(sizeof(*newNode));
    Node tmp = posList;
    while(tmp){
        if(hashMap->compareKeyFunction(key,tmp->key)){
            free(newNode);
            //what happens if the key exists?
            return;
        }
        tmp=tmp->next;
    }
    Socket newSocket = hashMap->socketCopyFunction(socket);
    newNode->socket=newSocket;
    newNode->key = hashMap->copyKeyFunction(key);
    newNode->next = posList;
    hashMap->table[pos] = newNode;

}

Socket getSocket(HashMap hashMap,Key key){
    int pos = hashCode(hashMap,key);
    Node posList = hashMap->table[pos];
    Node tmp = posList;
    while(tmp){
        if(hashMap->compareKeyFunction(tmp->key,key))
            return tmp->socket;
        tmp=tmp->next;
    }
    //????
    return NULL;
}

/////////////////////////////////////
//for use when building a dictionary
bool compareKeys(Key key1,Key key2){
    return key1->remotePort == key2->remotePort && strcmp(key1->localIp,key2->localIp) &&
                            key1->localPort == key2->localPort && strcmp(key1->remoteIp,key2->remoteIp);
}
Key copyKeyFunction(Key key){
    Key new = malloc(sizeof(*new));
    new->localPort = key->localPort;
    new->remotePort = key->remotePort;
    strcpy(new->localIp,key->localIp);
    strcpy(new->remoteIp,key->remoteIp);
    return new;
}




