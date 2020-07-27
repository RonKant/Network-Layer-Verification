//
// Created by Ido Yam on 02/05/2020.
//

#include "Hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "network.h"
#include "util_types.h"

#include "seahorn/seahorn.h"
int offset = 0;
unsigned int global_size;
struct Node_t{
    SocketID key;
    Socket socket;
    Node next;
    //bool inUse;
};

/*struct Key_t{
    int localPort;
    char* localIp;
    int remotePort;
    char* remoteIp;
};*/
int strcmp_t(char* str1,char* str2){
    while(*str1 && (*str1==*str2))
        str1++,str2++;
    return *(const unsigned char*)str1-*(const unsigned char*)str2;
}
void *xmalloc(size_t sz){
    void *p;
    p=malloc(sz);
    assume(p>0);
    return p;
}
struct HashMap_t{
    int size;
    int number_of_sockets;
    Node* table;
    copySocket socketCopyFunction;
    freeSocket freeSocketFuncition;
    compareSocket compareSocketFunction;
    freeKey freeKeyFunction;
    compareKey compareKeyFunction;
    copyKey copyKeyFunction;

};
bool compareKeys(SocketID key1,SocketID key2){
    assume(key1->dst_port >0 && key1->src_port >0&&key2->dst_port >0 && key2->src_port >0);
    return key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
           key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip);
}
SocketID copyKeyFunction(SocketID key,HashMapErrors *error){
    if(key == NULL)
    {
        *error = HASH_MAP_NULL_ARGUMENT;
        return NULL;
    }
    SocketID new = xmalloc(sizeof(*new));
    if(!new)
    {
        *error = HASH_MAP_ALLOCATION_FAIL;
        return NULL;
    }
    new->src_port = key->src_port;
    new->dst_port = key->dst_port;
    //check
    new->dst_ip = xmalloc(sizeof((key->dst_port)+1));
    new->src_ip = xmalloc(sizeof((key->src_ip)+1));

    strcpy(new->src_ip,key->src_ip);
    strcpy(new->dst_ip,key->dst_ip);
    return new;
}
HashMapErrors keyFree(SocketID key){
    if(!key)
        return HASH_MAP_NULL_ARGUMENT;
    free(key->dst_ip);
    free(key->src_ip);
    free(key);
    return HASH_MAP_SUCCESS;
}
Socket socketCopy(Socket socket,HashMapErrors *error){
    Socket s_copy = xmalloc(sizeof(*s_copy));
    if(s_copy == NULL) {
        *error = HASH_MAP_ALLOCATION_FAIL;
        return NULL;
    }
    s_copy->id = copyKeyFunction(socket->id,error);
    s_copy->next_seq = socket -> next_seq;
    s_copy->last_ack = socket->last_ack;
    s_copy->state = socket->state;
    s_copy->send_window_size = socket->send_window_size;
    s_copy->recv_window_size = socket->recv_window_size;
    s_copy->buffer_window = xmalloc(sizeof(socket->buffer_window));
    if(s_copy->buffer_window == NULL){
        *error = HASH_MAP_ALLOCATION_FAIL;
        return NULL;
    }
    strcpy(socket->buffer_window,s_copy->buffer_window);
    return s_copy;
}
HashMapErrors socketFree(Socket socket){
    if(socket == NULL)
        return HASH_MAP_NULL_ARGUMENT;
    free(socket->buffer_window);
    free(socket);
    return HASH_MAP_SUCCESS;
}
bool socketCompare(Socket socket1,Socket socket2,HashMapErrors *error){
    if(socket1 == NULL || socket2 == NULL)
        return false;
    *error = HASH_MAP_SUCCESS;
    return compareKeys(socket1->id,socket2->id);
}
HashMap createHashMap(int size){
    assume(size>0);
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = xmalloc(sizeof(*(hashMap->table))*size);
    offset = 0;
    global_size = sizeof(*(hashMap->table))*size;
    for(int i=0; i<size; i++){
        offset = sizeof(*(hashMap->table))*i;
        sassert(offset < global_size); //PROBLEM
        sassert(offset>=0);
        hashMap->table[i] = NULL;
    }
    hashMap->compareSocketFunction =  socketCompare;
    hashMap->socketCopyFunction=socketCopy;
    hashMap->freeSocketFuncition=socketFree;

    hashMap->compareKeyFunction=compareKeys;
    hashMap->copyKeyFunction=copyKeyFunction;
    hashMap->freeKeyFunction=keyFree;
    hashMap->number_of_sockets = 0;
    return hashMap;
}
int hashCode(HashMap hashMap, SocketID key){
    int ret = 1;
    assume (ret >= 0 && ret < hashMap->size);
    return ret;
}
HashMapErrors insertSocket(HashMap hashMap,SocketID key,Socket socket){
    if(!hashMap || !key || !socket)
        return HASH_MAP_NULL_ARGUMENT;
    int pos = hashCode(hashMap,key);
    sassert(pos >= 0 && pos < hashMap->size);
    Node posList = hashMap->table[pos];
    Node newNode = xmalloc(sizeof(*newNode));
    if(!newNode)
        return HASH_MAP_ALLOCATION_FAIL;
    Node tmp = posList;
    while(tmp){
        if(compareKeys(key,tmp->key)){
            free(newNode);
            return HASH_MAP_KEY_EXIST;
        }
        tmp=tmp->next;
    }
    HashMapErrors *err=NULL;
    Socket newSocket = hashMap->socketCopyFunction(socket,err);
    newNode->socket=newSocket;
    newNode->key = hashMap->copyKeyFunction(key,err);
    newNode->next = posList;
    hashMap->table[pos] = newNode;
    hashMap->number_of_sockets++;
    return HASH_MAP_SUCCESS;
}

Socket getSocket(HashMap hashMap,SocketID key,HashMapErrors *error){
    int pos = hashCode(hashMap,key);
    Node posList = hashMap->table[pos];
    Node tmp = posList;
    while(tmp){
        if(hashMap->compareKeyFunction(tmp->key,key)) {
            *error = HASH_MAP_SUCCESS;
            return tmp->socket;
        }
        tmp=tmp->next;
    }
    *error = HASH_MAP_SOCKET_NOT_FOUND;
    return NULL;
}
void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error) {
    if (hashMap == NULL || key == NULL) {
        *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    int pos = hashCode(hashMap, key);
    Node posList = hashMap->table[pos];
    Node tmp = posList;
    if (!tmp) {
        *error = HASH_MAP_SOCKET_NOT_FOUND;
        return; //the list is empty
    }
    if (tmp->next == NULL) {
        if (hashMap->compareKeyFunction(key, tmp->key))//one element in the posList
        {
            hashMap->freeKeyFunction(tmp->key);
            hashMap->freeSocketFuncition(tmp->socket);
            free(tmp);
            hashMap->table[pos] = NULL;
            hashMap->number_of_sockets--;
            *error = HASH_MAP_SUCCESS;
            return;
        } else{
            *error = HASH_MAP_SOCKET_NOT_FOUND;
            return;
        }
    }
    if(hashMap->compareKeyFunction(key,tmp->key))//more than one element but we want to remove the first
    {
        hashMap->table[pos] = tmp->next;
        hashMap->freeKeyFunction(tmp->key);
        hashMap->freeSocketFuncition(tmp->socket);
        free(tmp);
        hashMap->number_of_sockets--;
        *error = HASH_MAP_SUCCESS;
        return;
    }
    while(tmp && tmp->next){
        if(hashMap->compareKeyFunction(tmp->next->key,key)){
            Node tmpNext = tmp->next->next;
            hashMap->freeKeyFunction(tmp->next->key);
            hashMap->freeSocketFuncition(tmp->next->socket);
            free(tmp->next);
            tmp->next = tmpNext;
            hashMap->number_of_sockets--;
            *error = HASH_MAP_SUCCESS;
            return;
        }
        tmp=tmp->next;
    }
    *error = HASH_MAP_ERROR;
}
void hashDestroy(HashMap hashMap, HashMapErrors *error){
    if(!hashMap)
    {
        *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    HashMapErrors *err = NULL;
    for(int i=0; i<hashMap->size;i++){
        Node tmp= hashMap->table[i];
        while(tmp){
            hashmapRemove(hashMap,tmp->key,error);
            if(*error != HASH_MAP_SUCCESS) {
                *error = HASH_MAP_ERROR;
                return;
            }
            tmp = hashMap->table[i];
        }
    }
    free(hashMap->table);
    free(hashMap);
    *error = HASH_MAP_SUCCESS;
}
int getHashMapSize(HashMap hashMap){
    return hashMap->size;
}
int getHashMapNumberOfSockets(HashMap hashMap){
    return hashMap->number_of_sockets;
}






