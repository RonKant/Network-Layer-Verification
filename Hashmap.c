//
// Created by tomer on 14-Jun-20.
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


struct HashMap_t{
    int size;
    Node* table;
    socketCopy socketCopyFunction;
    freeSocket freeSocketFuncition;
    compareSocket compareSocketFunction;
    freeKey freeKeyFunction;
    compareKey compareKeyFunction;
    copyKey copyKeyFunction;

};
//change later - instead of sending functions to create just put these functions inside

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
HashMap createHashMap(int size,socketCopy socketCopyFunction,freeSocket freeSocketFuncition ,compareSocket compareSocketFunction
        ,copyKey copyKeyFunction,freeKey freeKeyFunction,compareKey compareKeyFunction){
    HashMap hashMap = malloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = malloc(sizeof(hashMap->table)*size);
    for(int i=0; i<size; i++){
        hashMap->table[i] = NULL;
    }
    hashMap->socketCopyFunction=socketCopyFunction;
    hashMap->freeSocketFuncition=freeSocketFuncition;
    hashMap->compareKeyFunction=compareKeyFunction;
    hashMap->compareKeyFunction =compareKeyFunction;
    hashMap->copyKeyFunction=copyKeyFunction;
    hashMap->freeKeyFunction=freeKeyFunction;
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
void hashmapRemove(HashMap hashMap, Key key) {
    if (hashMap == NULL)
        return;
    int pos = hashCode(hashMap, key);
    Node posList = hashMap->table[pos];
    Node tmp = posList;
    if (!tmp)
        return; //the list is empty
    if (tmp->next == NULL) {
        if (hashMap->compareKeyFunction(key, tmp->key))//one element in the posList
        {
            hashMap->freeKeyFunction(tmp->key);
            hashMap->freeSocketFuncition(tmp->socket);
            free(tmp);
            hashMap->table[pos] = NULL;
            return;
        } else{
            return;
        }
    }
    if(hashMap->compareKeyFunction(key,tmp->key))//more than one element but we want to remove the first
    {
        hashMap->table[pos] = tmp->next;
        hashMap->freeKeyFunction(tmp->key);
        hashMap->freeSocketFuncition(tmp->socket);
        free(tmp);
        return;
    }
    while(tmp && tmp->next){
        if(hashMap->compareKeyFunction(tmp->next->key,key)){
            Node tmpNext = tmp->next->next;
            hashMap->freeKeyFunction(tmp->next->key);
            hashMap->freeSocketFuncition(tmp->next->socket);
            free(tmp->next);
            tmp->next = tmpNext;
            return;
        }
        tmp=tmp->next;
    }
    //fail - what do we do?

}
void hashDestroy(HashMap hashMap){
    if(!hashMap)
        return;
    for(int i=0; i<hashMap->size;i++){
        Node tmp= hashMap->table[i];
        while(tmp){
            hashmapRemove(hashMap,tmp->key);
            tmp = hashMap->table[i];

        }
    }
    free(hashMap->table);
    free(hashMap);
}




