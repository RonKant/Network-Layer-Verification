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
#include "Functions.h"
#include "array_queue.h"

#include "seahorn/seahorn.h"

extern void* nd_ptr();

int offset = 0;
unsigned int global_size;


struct HashMap_t{
    int size;
    int number_of_sockets;
    Queue* table;
    int current_iterated_queue;
    /*
    Socket* ghost_v;
    int* ghost_has_v;
    */
};

bool compareKeys(SocketID key1,SocketID key2){
    if(key1 == key2)
        return true;
    //assume(key1 != NULL && key2 != NULL);
    //assume(key1->dst_port >0 && key1->src_port >0&&key2->dst_port >0 && key2->src_port >0);
    //assume(strcmp_t(key1->src_ip,"") != 0 && strcmp_t(key1->dst_ip,"") != 0 && strcmp_t(key2->src_ip,"") != 0 && strcmp_t(key2->dst_ip,"") != 0);
    //return key1->dst_port == key2->dst_port && key1->src_port == key2->src_port;
    return key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
           key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip);
}
HashMap createHashMap(int size){
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = xmalloc(sizeof(*(hashMap->table))*size);
    offset = 0;
    global_size = sizeof(*(hashMap->table))*size;
    for(int i=0; i<getHashMapSize(hashMap); i = i+1){
        offset = sizeof(*(hashMap->table))*i;
        sassert(offset < global_size); //PROBLEM
        sassert(offset>=0);
        hashMap->table[i] = QueueCreate(5);
    }
    hashMap->number_of_sockets = 0;
    hashMap->current_iterated_queue = 0;
/*
    hashMap->ghost_v = xmalloc(sizeof(*hashMap->ghost_v)*(size*5));
    hashMap->ghost_has_v = xmalloc(sizeof(*hashMap->ghost_has_v)*(size*5));

    for(int i=0; i<5*size;i++){
        hashMap->ghost_v[i] = nd_ptr();
        hashMap->ghost_has_v = 0;
    }
*/
    return hashMap;
}
int hashCode(HashMap hashMap, SocketID key){
    int ret = 0;
    assume (ret >= 0 && ret < hashMap->size);
    return ret;
}
bool insertSocket(HashMap hashMap,Socket socket){
    if(!hashMap || !socket)
        return HASH_MAP_NULL_ARGUMENT;
    ////*needed_assume(hashMap>0 && key>0 && socket>0);
    //int pos = hashCode(hashMap,key);
    int pos = 0;
    //sassert(pos >= 0 && pos < hashMap->size);
    Queue posQueue = hashMap->table[pos];
    QUEUE_FOR_EACH(item, posQueue) {
        if (compareKeys(socket->id,((Socket)(item))->id)) {
            return false;
        }
    }
    bool b = enqueue(posQueue,socket);
    if(b == true){
        hashMap->number_of_sockets = hashMap->number_of_sockets +1;
        return true;
    } else
        return false;
    /*
    (hashMap->ghost_has_v[5*pos+QueueSize(posQueue)]) = hashMap->ghost_v[5*pos+QueueSize(posQueue)]==socket;
    if(hashMap->ghost_has_v[5*pos+QueueSize(posQueue)]){
        hashMap->ghost_v[5*pos+QueueSize(posQueue)]=socket;
        bool b = enqueue(posQueue,socket);
        hashMap->number_of_sockets = hashMap->number_of_sockets +1;
        if(b == true){
            return HASH_MAP_SUCCESS;
        } else
            return HASH_MAP_ERROR;
    }*/
}
Socket getSocket(HashMap hashMap,SocketID key){
    if (hashMap == NULL || key == NULL)
        return NULL;
    if (getHashMapNumberOfSockets(hashMap) == 0)
        return NULL;
    int pos = 0;
    Queue posQueue = hashMap->table[pos];

    QUEUE_FOR_EACH(item, posQueue) {
        if (compareKeys(key, ((Socket)(item))->id)) {
            return item;
        }
    }
    return QueueGetFirst(posQueue);
    //return NULL;
}

SocketID hashMapGetFirst(HashMap hashMap){
    if(hashMap == NULL)
        return NULL;
    if (getHashMapSize(hashMap) == 0) {
        return NULL;
    }
    for (int queue_num = 0; queue_num < hashMap->size; queue_num = queue_num +1) {
        Socket element_from_queue = (Socket)(QueueGetFirst((hashMap->table)[queue_num]));
        if (element_from_queue != NULL) {
            hashMap->current_iterated_queue = queue_num;
            return element_from_queue->id;
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

    Socket element_from_queue = (Socket)(QueueGetNext((hashMap->table)[hashMap->current_iterated_queue]));
    if (element_from_queue != NULL) {
        return element_from_queue->id;
    } else {
        // finished with this queue, move to the next one. (find first non empty one)
        for (int queue_num = hashMap->current_iterated_queue + 1; queue_num < hashMap->size; queue_num = queue_num+1) {
            element_from_queue = (Socket)(QueueGetFirst((hashMap->table)[queue_num]));
            if (element_from_queue != NULL) {
                hashMap->current_iterated_queue = queue_num;
                return element_from_queue->id;
            }
        }
    }

    // if reached here - all queues were empty.
    return NULL;
}
bool HashMapSocketRemoveCond(Socket socket, SocketID socketId){
    if(socket == NULL)
        return false;
    if(compareKeys(socket->id,socketId) == true)
        return true;
    return false;
}
bool hashmapRemove(HashMap hashMap, SocketID key) {
    if (hashMap == NULL || key == NULL) {
        return false;
    }
    int pos = 0;
    Queue posQueue = hashMap->table[pos];
    for(int i= 0 ; i<QueueSize(posQueue); i++){
        assume(QueueGetElement(posQueue,i) != NULL);
    }
    if (QueueRemoveByCondition(posQueue, (conditionFunction)HashMapSocketRemoveCond, key) != NULL){
        hashMap->number_of_sockets = hashMap->number_of_sockets - 1;
        return true;
    }
    return false;
}
void hashDestroy(HashMap hashMap, HashMapErrors *error){
    if(!hashMap)
    {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    for(int i=0; i<hashMap->size;i = i+1){
        Queue tmp= hashMap->table[i];
        QueueDestroy(tmp,destroy_socket);
    }
    free(hashMap->table);
    free(hashMap);
    if (NULL != error) *error = HASH_MAP_SUCCESS;
}
int getHashMapSize(HashMap hashMap){
    return hashMap->size;
}
int getHashMapNumberOfSockets(HashMap hashMap){
    if(hashMap == NULL)
        return -1;
    return hashMap->number_of_sockets;
}
bool HashMapcmp(HashMap hashMap1, HashMap hashMap2) {
    if (hashMap1 == NULL || hashMap2 == NULL)
        return hashMap1 == hashMap2;
    if (getHashMapNumberOfSockets(hashMap1) != getHashMapNumberOfSockets(hashMap2))
        return false;
    if (getHashMapSize(hashMap1) != getHashMapSize(hashMap2))
        return false;

    for (int i = 0; i < getHashMapSize(hashMap1); i++) {
        Queue posQueue1 = hashMap1->table[i];
        Queue posQueue2 = hashMap1->table[i];
        if (QueueSize(posQueue1) != QueueSize(posQueue2))
            return false;
        for (int j = 0; j < QueueSize(posQueue1); j++) {
            if (compareKeys(QueueGetElement(posQueue1, j), QueueGetElement(posQueue2, j)))
                return false;
        }
    }
    return true;
}