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
#include "queue.h"
#include "seahorn/seahorn.h"
int offset = 0;
unsigned int global_size;
struct DictElement_t
{
    SocketID key;
    Socket socket;
};
struct HashMap_t{
    int size;
    int number_of_sockets;
    Queue* table;
    Node iterator;
    copySocket socketCopyFunction;
    freeSocket freeSocketFuncition;
    compareSocket compareSocketFunction;
    freeKey freeKeyFunction;
    compareKey compareKeyFunction;
    copyKey copyKeyFunction;
};

bool compareKeys(SocketID key1,SocketID key2){
    if(key1 == key2)
        return true;
    assume(key1 != NULL && key2 != NULL);
    assume(key1->dst_port >0 && key1->src_port >0&&key2->dst_port >0 && key2->src_port >0);
    assume(key1->dst_ip != NULL && key1->src_ip != NULL && key2->dst_ip != NULL && key2->src_ip != NULL);
    return key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
           key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip);
}
SocketID copyKeyFunction(SocketID key,HashMapErrors *error){
    assume(key != NULL);
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
    new->dst_ip = xmalloc(sizeof((key->dst_ip)+1));
    new->src_ip = xmalloc(sizeof((key->src_ip)+1));

    strcpy_t(new->src_ip,key->src_ip);
    strcpy_t(new->dst_ip,key->dst_ip);
    *error = HASH_MAP_SUCCESS;
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
    s_copy->state = socket->state;
    s_copy->send_window = xmalloc(sizeof(socket->send_window));
    if(s_copy->send_window == NULL){
        *error = HASH_MAP_ALLOCATION_FAIL;
        return NULL;
    }
    if(socket->send_window != NULL)
        strcpy_t(socket->send_window,s_copy->send_window);
    s_copy->send_window_size = socket->send_window_size;
    s_copy->max_send_window_size = socket->max_send_window_size;
    s_copy->seq_of_first_send_window = socket->seq_of_first_send_window;

    s_copy->recv_window = xmalloc(sizeof(socket->recv_window));
    if(s_copy->recv_window == NULL){
        *error = HASH_MAP_ALLOCATION_FAIL;
        return NULL;
    }
    if(socket->recv_window != NULL)
        strcpy_t(socket->recv_window,s_copy->recv_window);
    s_copy->recv_window_size = socket->recv_window_size;
    s_copy->max_recv_window_size = socket->max_recv_window_size;
    s_copy->seq_of_first_recv_window = socket->seq_of_first_recv_window;
    *error = HASH_MAP_SUCCESS;
    return s_copy;
}
HashMapErrors socketFree(Socket socket){
    if(socket == NULL)
        return HASH_MAP_NULL_ARGUMENT;
    free(socket->send_window);
    free(socket->recv_window);

    free(socket);
    return HASH_MAP_SUCCESS;
}
bool socketCompare(Socket socket1,Socket socket2,HashMapErrors *error){
    if(socket1 == NULL || socket2 == NULL)
        return false;
    *error = HASH_MAP_SUCCESS;
    return compareKeys(socket1->id,socket2->id);
}
bool dictElementCompare(DictElement d1,DictElement d2){
    if(d1 == NULL || d2 ==NULL )
        return false;
    return compareKeys(d1->key,d2->key);
}
DictElement dictElementCopy(DictElement d){
    if(d == NULL)
        return NULL;
    DictElement copy = xmalloc(sizeof(*copy));
    HashMapErrors* err = xmalloc(sizeof(*err));
    copy->key = copyKeyFunction(d->key,err);
    copy->socket = socketCopy(d->socket,err);
    return copy;
}
void dictElementFree(DictElement d){
    if(d == NULL)
        return;
    socketFree(d->socket);
    keyFree(d->key);
    free(d);
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
        // creates an empty queue
        //***check***
        hashMap->table[i] = createQueue_g(sizeof(DictElement),dictElementCompare,dictElementFree,dictElementCopy);
    }
    hashMap->compareSocketFunction =  socketCompare;
    hashMap->socketCopyFunction=socketCopy;
    hashMap->freeSocketFuncition=socketFree;

    hashMap->compareKeyFunction=compareKeys;
    hashMap->copyKeyFunction=copyKeyFunction;
    hashMap->freeKeyFunction=keyFree;
    hashMap->number_of_sockets = 0;
    hashMap->iterator = NULL;
    return hashMap;
}
int hashCode(HashMap hashMap, SocketID key){
    int ret = 1;
    assume (ret >= 0 && ret < hashMap->size);
    return ret;
}
HashMapErrors insertSocket(HashMap hashMap,SocketID key,Socket socket){
    if(!hashMap || !socket)
        return HASH_MAP_NULL_ARGUMENT;
    assume(hashMap>0 && key>0 && socket>0);
    int pos = hashCode(hashMap,key);
    //int pos = 1;
    sassert(pos >= 0 && pos < hashMap->size);
    Queue posQueue = hashMap->table[pos];
    DictElement newDictElement = xmalloc(sizeof(*newDictElement));

    Node tmp = posQueue->head;
    while(tmp != NULL){
        assume(((DictElement)getValue(tmp))->socket->id !=NULL);
        bool res = compareKeys(key,(((DictElement)getValue(tmp))->socket->id));
        if(res){
            //exists
            free(newDictElement);
            return HASH_MAP_KEY_EXIST;
        }
        tmp=getNext(tmp);
    }
    HashMapErrors *err = xmalloc(sizeof(*err));
    Socket newSocket = socketCopy(socket,err);
    if(*err != HASH_MAP_SUCCESS){
        free(newDictElement);
        return HASH_MAP_ALLOCATION_FAIL;
    }
    newDictElement->socket = newSocket;
    newDictElement->key = copyKeyFunction(socket->id,err);
    if(*err != HASH_MAP_SUCCESS){
        free(newDictElement);
        free(newSocket);
        return HASH_MAP_ALLOCATION_FAIL;
    }
    sassert(newSocket !=NULL);
    enqueue(posQueue,newDictElement);
    hashMap->number_of_sockets++;
    return HASH_MAP_SUCCESS;
}
Socket getSocket(HashMap hashMap,SocketID key,HashMapErrors *error){
    int pos = hashCode(hashMap,key);
    Queue posQueue = hashMap->table[pos];
    Node tmp = getHead(posQueue);
    while(tmp){
        if(hashMap->compareKeyFunction(((DictElement)getValue(tmp))->socket->id,key)) {
            *error = HASH_MAP_SUCCESS;
            //check if return a copy of the value
            return ((DictElement)getValue(tmp))->socket;
        }
        tmp=getNext(tmp);
    }
    *error = HASH_MAP_SOCKET_NOT_FOUND;
    return NULL;
}
SocketID hashMapGetFirst(HashMap hashMap) {
    //sassert(hashMap != NULL);
    if (hashMap->size == 0) return NULL;
    hashMap->iterator = getHead(hashMap->table[0]);
    return ((DictElement)getValue(hashMap->iterator))->socket->id;
}
void hashMapSetFirst(HashMap hashMap){
    if(hashMap == NULL || hashMap->size == 0)
        return;
    hashMap->iterator = getHead(hashMap->table[0]);
}
bool hashMapHasNext(HashMap hashMap){
    return getNext(hashMap->iterator) == NULL;
}
SocketID hashMapGetNext(HashMap hashMap){
    if(hashMap == NULL || hashMap->size == 0)
        return NULL;
    if(hashMap->size == 1){
        return (((DictElement)getValue(hashMap->iterator))->socket->id);
    }
    else{
        // if the list where the node ends is done than we need the next list
        if(!hashMapHasNext(hashMap)){
            int index_in_table = hashCode(hashMap,((DictElement)getValue(hashMap->iterator))->socket->id);
            if (index_in_table == hashMap->size - 1) {
                // reached end of iteration
                return NULL;
            }
            int index = index_in_table+1;
            while(hashMap->table[index]->sizeOfQueue == 0){
                index = index + 1;
                if(index == hashMap->size - 1){
                    // reached end of iteration
                    return NULL;
                }
            }
            return (((DictElement)getValue(getHead((hashMap->table[index]))))->socket->id);
        }
        else{
            return (((DictElement)(getValue(getNext(hashMap->iterator))))->socket->id);
        }
    }
}

void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error) {
    if (hashMap == NULL || key == NULL) {
        *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    int pos = hashCode(hashMap, key);
    Queue posQueue = hashMap->table[pos];
    Node tmp = getHead(posQueue);
    if (!tmp) {
        *error = HASH_MAP_SOCKET_NOT_FOUND;
        return; //the queue is empty
    }
    //if iterator points to the element we remove
    if(hashMap->iterator != NULL && compareKeys(key,((DictElement)getValue(hashMap->iterator))->socket->id)){
        if(hashMap->size == 1){
            hashMap->iterator = NULL;
        }
        else{
            // if the queue where the node ends is done than we need the next list
            if(!hashMapHasNext(hashMap)){
                int index_in_table = hashCode(hashMap,((DictElement)(getValue(hashMap->iterator)))->socket->id);
                if (index_in_table == hashMap->size - 1) {
                    // reached end of iteration
                    hashMap->iterator = NULL;
                }
                else{
                    while(hashMap->table[index_in_table]->sizeOfQueue == 0){
                        index_in_table = index_in_table + 1;
                        if(index_in_table == hashMap->size - 1){
                            // reached end of iteration
                            hashMap->iterator = NULL;
                        }
                    }
                    hashMap->iterator = getHead(hashMap->table[index_in_table+1]);
                }
            }
            else{
                hashMap->iterator = getNext(hashMap->iterator);
            }
        }
    }
    if (getNext(tmp) == NULL){
        if (hashMap->compareKeyFunction(key,(((DictElement)getValue(tmp))->key)))//one element in the posList
        {
            hashMap->freeKeyFunction(((DictElement)getValue(tmp))->key);
            hashMap->freeSocketFuncition(((DictElement)getValue(tmp))->socket);
            free(tmp);
            hashMap->table[pos] = NULL;
            hashMap->number_of_sockets--;
            *error = HASH_MAP_SUCCESS;
            posQueue->sizeOfQueue = posQueue->sizeOfQueue-1;
            return;
        }else{
            *error = HASH_MAP_SOCKET_NOT_FOUND;
            return;
        }
    }
    if(hashMap->compareKeyFunction(key,((DictElement)getValue(tmp))->key))//more than one element but we want to remove the first
    {
        hashMap->table[pos]->head = getNext(tmp);
        hashMap->freeKeyFunction(((DictElement)getValue(tmp))->key);
        hashMap->freeSocketFuncition(((DictElement)getValue(tmp))->socket);
        free(tmp);
        hashMap->number_of_sockets--;
        *error = HASH_MAP_SUCCESS;
        posQueue->sizeOfQueue = posQueue->sizeOfQueue-1;
        return;
    }
    while(tmp && getNext(tmp)){
        if(hashMap->compareKeyFunction(((DictElement)getValue(getNext(tmp)))->key,key)){
            Node tmpNext = getNext(getNext(tmp));
            hashMap->freeKeyFunction(((DictElement)getValue(getNext(tmp)))->key);
            hashMap->freeSocketFuncition(((DictElement)getValue(getNext(tmp)))->socket);
            free(getNext(tmp));
            setNext(tmp,tmpNext);
            hashMap->number_of_sockets--;
            *error = HASH_MAP_SUCCESS;
            posQueue->sizeOfQueue = posQueue->sizeOfQueue-1;
            return;
        }
        tmp=getNext(tmp);
    }
    *error = HASH_MAP_ERROR;
}
void hashDestroy(HashMap hashMap, HashMapErrors *error){
    if(!hashMap)
    {
        *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    QueueErrors* queueErrors = xmalloc(sizeof(*queueErrors));
    for(int i=0; i<hashMap->size;i++){
        Queue tmp= hashMap->table[i];
        destroyQueue(tmp,queueErrors);
        if(*queueErrors != Queue_SUCCESS) {
            *queueErrors = Queue_ERROR;
            return;

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





