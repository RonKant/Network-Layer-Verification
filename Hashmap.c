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

#define QUEUE_SIZE_HASH 200

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
    if (NULL != error) *error =    HASH_MAP_SUCCESS;
    return copy_socket_id(key);

    ////*needed_//assume(key != NULL);
}
HashMapErrors keyFree(SocketID key){
    if(!key)
        return HASH_MAP_NULL_ARGUMENT;
    free(key->dst_ip);
    free(key->src_ip);
    free(key);
    return HASH_MAP_SUCCESS;
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

HashMap createHashMap(int size){
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=size;
    hashMap->table = xmalloc(sizeof(*(hashMap->table))*size);
    offset = 0;
    global_size = sizeof(*(hashMap->table))*size;
    for(int i=0; i<getHashMapSize(hashMap); i = i+1){
        offset = sizeof(*(hashMap->table))*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        hashMap->table[i] = QueueCreate(QUEUE_SIZE_HASH);
    }
    hashMap->number_of_sockets = 0;
    hashMap->current_iterated_queue = 0;
    return hashMap;
}
int hashCode(HashMap hashMap, SocketID key){
    int ret = 0;
    //assume (ret >= 0 && ret < hashMap->size);
    return ret;
}
HashMapErrors insertSocket(HashMap hashMap,SocketID key,Socket socket){
    if(!hashMap || !socket || !key)
        return HASH_MAP_NULL_ARGUMENT;
    ////*needed_//assume(hashMap>0 && key>0 && socket>0);
    //int pos = hashCode(hashMap,key);
    int pos = 0;
    ////sassert(pos >= 0 && pos < hashMap->size);
    Queue posQueue = hashMap->table[pos];

    QUEUE_FOR_EACH(item, posQueue) {
        if (compareKeys(key,((Socket)(item))->id)) {
            return HASH_MAP_KEY_EXIST;
        }
    }
    bool b = enqueue(posQueue,socket);
    hashMap->number_of_sockets = hashMap->number_of_sockets +1;
    if(b == true){
        return HASH_MAP_SUCCESS;
    } else
        return HASH_MAP_ERROR;
}
Socket getSocket(HashMap hashMap,SocketID key){
    if (hashMap == NULL || key == NULL)
        return NULL;
    if (getHashMapNumberOfSockets(hashMap) == 0)
        return NULL;
    int pos = hashCode(hashMap, key);
    Queue posQueue = hashMap->table[pos];
    // for(int i=0; i<QueueSize(posQueue); i=i+1){
    //     if(QueueGetElement(posQueue,i) != NULL){
    //         if (compareKeys(key, ((Socket)(QueueGetElement(posQueue, i)))->id))
    //             return QueueGetElement(posQueue, i);
    //     }
    // }
    QUEUE_FOR_EACH(item, posQueue) {
        if (compareKeys(key, ((Socket)(item))->id)) {
            return item;
        }
    }
    
    return NULL;
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
void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error) {
    if (hashMap == NULL || key == NULL) {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    int pos = hashCode(hashMap, key);
    Queue posQueue = hashMap->table[pos];
    if(QueueRemoveByCondition(posQueue, (conditionFunction) HashMapSocketRemoveCond, key)!=NULL)
        hashMap->number_of_sockets = hashMap->number_of_sockets-1;
    if (NULL != error) *error = HASH_MAP_SUCCESS;
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