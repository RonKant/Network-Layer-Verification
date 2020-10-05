//
// Created by Ido Yam on 02/05/2020.
//

#include "Hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "network.h"
#include "socket_utils.h"
#include "util_types.h"
#include "queue.h"
//#include "seahorn/seahorn.h"
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

    int current_iterated_queue;
};

bool compareKeys(SocketID key1,SocketID key2){
    if(key1 == key2)
        return true;
    //assume(key1 != NULL && key2 != NULL);
    //assume(key1->dst_port >0 && key1->src_port >0&&key2->dst_port >0 && key2->src_port >0);
    //assume(key1->dst_ip != NULL && key1->src_ip != NULL && key2->dst_ip != NULL && key2->src_ip != NULL);
    return key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
           key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip);
}
SocketID copyKeyFunction(SocketID key,HashMapErrors *error){
    if(key == NULL)
    {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return NULL;
    }

    return copy_socket_id(key);

    //assume(key != NULL);
}
HashMapErrors keyFree(SocketID key){
    if(!key)
        return HASH_MAP_NULL_ARGUMENT;
    free(key->dst_ip);
    free(key->src_ip);
    free(key);
    return HASH_MAP_SUCCESS;
}
Socket socketCopy(Socket socket,HashMapErrors *error){ // TODO: update this function
    if (NULL != error) *error = HASH_MAP_SUCCESS;

    Socket s_copy = copy_socket(socket);
    if (s_copy == NULL) {
        if (NULL != error) *error = HASH_MAP_ALLOCATION_FAIL;
    }

    return s_copy;
}
HashMapErrors socketFree(Socket socket){
    if(socket == NULL)
        return HASH_MAP_NULL_ARGUMENT;

    destroy_socket(socket);

    return HASH_MAP_SUCCESS;
}
bool socketCompare(Socket socket1,Socket socket2,HashMapErrors *error){
    if(socket1 == NULL || socket2 == NULL)
        return false;
    if (NULL != error) *error = HASH_MAP_SUCCESS;
    return compareKeys(socket1->id,socket2->id);
}
bool dictElementCompare(void* d1,void* d2){
    if(d1 == NULL || d2 ==NULL )
        return false;
    return compareKeys(((DictElement)d1)->key,((DictElement)d2)->key);
}
void* dictElementCopy(void* d){
    if(d == NULL)
        return NULL;
    DictElement copy = xmalloc(sizeof(*copy));
    copy->key = copyKeyFunction(((DictElement)d)->key,NULL);
    copy->socket = socketCopy(((DictElement)d)->socket,NULL);

    return copy;
}
void dictElementFree(void* d){
    if(d == NULL)
        return;
    socketFree(((DictElement)d)->socket);
    keyFree(((DictElement)d)->key);
    free(d);
}

HashMap createHashMap(int size){
    //assume(size>0);
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = xmalloc(sizeof(*(hashMap->table))*size);
    //offset = 0;
    //global_size = sizeof(*(hashMap->table))*size;
    for(int i=0; i<size; i++){
        //offset = sizeof(*(hashMap->table))*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        // creates an empty queue
        //***check***
        hashMap->table[i] = createQueue_g(dictElementCompare,dictElementFree,dictElementCopy);
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
    //assume (ret >= 0 && ret < hashMap->size);
    return ret;
}
HashMapErrors insertSocket(HashMap hashMap,SocketID key,Socket socket){
    if(!hashMap || !socket)
        return HASH_MAP_NULL_ARGUMENT;
    //assume(hashMap>0 && key>0 && socket>0);
    //int pos = hashCode(hashMap,key);
    int pos = 1;
    //sassert(pos >= 0 && pos < hashMap->size);
    Queue posQueue = hashMap->table[pos];
    DictElement newDictElement = xmalloc(sizeof(*newDictElement));
    Node tmp = posQueue->head;
    while(tmp != NULL){
        //assume(tmp->key !=NULL);
        bool res = compareKeys(key,(((DictElement)getValue(tmp))->socket->id));
        if(res){
            //exists
            free(newDictElement);
            return HASH_MAP_KEY_EXIST;
        }
        tmp=getNext(tmp);
    }
    HashMapErrors err = SUCCESS;
    Socket newSocket = socketCopy(socket,&err);
    if(err != HASH_MAP_SUCCESS){
        free(newDictElement);
        return HASH_MAP_ALLOCATION_FAIL;
    }
    newDictElement->socket = newSocket;
    newDictElement->key = socket->id; //copyKeyFunction(socket->id,&err);

    if(err != HASH_MAP_SUCCESS){
        free(newDictElement);
        destroy_socket(newSocket);
        return HASH_MAP_ALLOCATION_FAIL;
    }
    //sassert(newSocket !=NULL);
    enqueue(posQueue,newDictElement);
    destroy_socket(newDictElement->socket);
    free(newDictElement);
    hashMap->number_of_sockets++;
    return HASH_MAP_SUCCESS;
}

Socket getSocket(HashMap hashMap,SocketID key,HashMapErrors *error){
    int pos = hashCode(hashMap,key);
    Queue posQueue = hashMap->table[pos];

    Node temp = posQueue->head;
    while (temp != NULL) {
        DictElement value = temp->value;
        if (compareKeys(value->key, key)) {
            return value->socket;
        }
        temp = temp->next;
    }

    // QUEUE_FOR_EACH(queue_item, posQueue) {
    //     if (compareKeys(key, ((DictElement)(queue_item))->key)) {
    //         if (NULL != error) *error = HASH_MAP_SUCCESS;
    //         return ((DictElement)(queue_item))->socket;
    //     }
    // }

    if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
    return NULL;
}

SocketID hashMapGetFirst(HashMap hashMap) {
    //assert(hashMap != NULL);
    if (hashMap == NULL || hashMap->size == 0) {
        return NULL;
    }
    for (int queue_num = 0; queue_num < hashMap->size; ++queue_num) {
        DictElement element_from_queue = (DictElement)(queueGetFirst((hashMap->table)[queue_num]));
        if (element_from_queue != NULL) {
            hashMap->current_iterated_queue = queue_num;
            return element_from_queue->key;
        }
    }
    // none of the queues contained elements
    return NULL;
}

SocketID hashMapGetNext(HashMap hashMap){
    if(hashMap == NULL || hashMap->size == 0)
        return NULL;

    if (hashMap->current_iterated_queue < 0 || hashMap->current_iterated_queue >= hashMap->size) {
        return NULL;
    }

    DictElement element_from_queue = (DictElement)(queueGetNext((hashMap->table)[hashMap->current_iterated_queue]));
    if (element_from_queue != NULL) {
        return element_from_queue->key;
    } else {
        // finished with this queue, move to the next one. (find first non empty one)
        for (int queue_num = hashMap->current_iterated_queue + 1; queue_num < hashMap->size; ++queue_num) {
            DictElement element_from_queue = (DictElement)(queueGetFirst((hashMap->table)[queue_num]));
            if (element_from_queue != NULL) {
                hashMap->current_iterated_queue = queue_num;
                return element_from_queue->key;
            }
        }
    }

    // if reached here - all queues were empty.
    return NULL;
}

void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error) {
    if (hashMap == NULL || key == NULL) {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    int pos = hashCode(hashMap, key);
    Queue posQueue = hashMap->table[pos];
    Node tmp = getHead(posQueue);
    if (!tmp) {
        if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
        return; //the queue is empty
    }
    //if iterator points to the element we remove
    // if(compareKeys(key,((DictElement)getValue(hashMap->iterator))->socket->id)){
    //     if(hashMap->size == 1){
    //         hashMap->iterator = NULL;
    //     }
    //     else{
    //         // if the queue where the node ends is done than we need the next list
    //         if(!hashMapHasNext(hashMap)){
    //             int index_in_table = hashCode(hashMap,((DictElement)(getValue(hashMap->iterator)))->socket->id);
    //             if (index_in_table == hashMap->size - 1) {
    //                 // reached end of iteration
    //                 hashMap->iterator = NULL;
    //             }
    //             else{
    //                 while(hashMap->table[index_in_table]->sizeOfQueue == 0){
    //                     index_in_table = index_in_table + 1;
    //                     if(index_in_table == hashMap->size - 1){
    //                         // reached end of iteration
    //                         return;
    //                     }
    //                 }
    //                 hashMap->iterator = getHead(hashMap->table[index_in_table+1]);
    //             }
    //         }
    //         else{
    //             hashMap->iterator = getNext(hashMap->iterator);
    //         }
    //     }
    // }
    if (getNext(tmp) == NULL){
        if (hashMap->compareKeyFunction(key,(((DictElement)getValue(tmp))->key)))//one element in the posList
        {
            hashMap->freeKeyFunction(((DictElement)getValue(tmp))->key);
            hashMap->freeSocketFuncition(((DictElement)getValue(tmp))->socket);
            free(tmp);
            hashMap->table[pos] = NULL;
            hashMap->number_of_sockets--;
            if (NULL != error) *error = HASH_MAP_SUCCESS;
            return;
        }else{
            if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
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
        if (NULL != error) *error = HASH_MAP_SUCCESS;
        return;
    }
    while(tmp && getNext(tmp)){
        if(hashMap->compareKeyFunction(((DictElement)getValue(tmp))->key,key)){
            Node tmpNext = getNext(getNext(tmp));
            hashMap->freeKeyFunction(((DictElement)getValue(tmp))->key);
            hashMap->freeSocketFuncition(((DictElement)getValue(getNext(tmp)))->socket);
            free(getNext(tmp));
            setNext(tmp,tmpNext);
            hashMap->number_of_sockets--;
            if (NULL != error) *error = HASH_MAP_SUCCESS;
            return;
        }
        tmp=getNext(tmp);
    }
    if (NULL != error) *error = HASH_MAP_ERROR;
}
void hashDestroy(HashMap hashMap, HashMapErrors *error){
    if(!hashMap)
    {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    QueueErrors queueErrors = Queue_SUCCESS;
    for(int i=0; i<hashMap->size;i++){
        Queue tmp= hashMap->table[i];
        destroyQueue(tmp,&queueErrors);
        if(queueErrors != Queue_SUCCESS) {
            queueErrors = HASH_MAP_ERROR;
            return;

        }
    }
    free(hashMap->table);
    free(hashMap);
    if (NULL != error) *error = HASH_MAP_SUCCESS;
}
int getHashMapSize(HashMap hashMap){
    return hashMap->size;
}
int getHashMapNumberOfSockets(HashMap hashMap){
    return hashMap->number_of_sockets;
}