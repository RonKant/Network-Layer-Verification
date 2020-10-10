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

// #include "seahorn/seahorn.h"

// extern void* nd_ptr();
// extern void* nd();

bool compareKeys(SocketID key1,SocketID key2){
   
    // if (! (key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
    //        key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip))) {

    //             printf("key1: (%s, %d), (%s, %d).\n", key1->src_ip, key1->src_port, key1->dst_ip, key1->dst_port);
    // printf("key2: (%s, %d), (%s, %d).\n", key2->src_ip, key2->src_port, key2->dst_ip, key2->dst_port);


    //     printf("%d %d %d %d.\n", key1->dst_port == key2->dst_port, !strcmp_t(key1->src_ip,key2->src_ip),
    //         key1->src_port == key2->src_port, !strcmp_t(key1->dst_ip,key2->dst_ip));
    //        }
    if(key1 == key2)
        return true;
    //assume(key1 != NULL && key2 != NULL);
    //assume(key1->dst_port >0 && key1->src_port >0&&key2->dst_port >0 && key2->src_port >0);
    //assume(strcmp_t(key1->src_ip,"") != 0 && strcmp_t(key1->dst_ip,"") != 0 && strcmp_t(key2->src_ip,"") != 0 && strcmp_t(key2->dst_ip,"") != 0);
    //return key1->dst_port == key2->dst_port && key1->src_port == key2->src_port;
    return key1->dst_port == key2->dst_port && !strcmp_t(key1->src_ip,key2->src_ip) &&
           key1->src_port == key2->src_port && !strcmp_t(key1->dst_ip,key2->dst_ip);
}
HashMap createHashMap(){
    // unsigned long offset = 0;
    // unsigned long global_size;
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=HASH_MAP_DEFAULT_SIZE;
    // global_size = sizeof(Socket)*HASH_MAP_DEFAULT_SIZE;
    for(int i=0; i<HASH_MAP_DEFAULT_SIZE; i = i+1){
        // offset = sizeof(Socket)*i;
        // sassert(offset < global_size); //PROBLEM
        // sassert(offset>=0);
        hashMap->table[i] = NULL;
        hashMap->socket_id[i] = NULL;
    }
    hashMap->number_of_sockets = 0;
    // hashMap->ghost_v = nd_ptr();
    // hashMap->ghost_has_v = false;
    hashMap->iterator_index = 0;
    return hashMap;
}
bool hasKey(HashMap hashMap, SocketID socketId){
    if (NULL == hashMap || NULL == socketId) {
        return false;
    }
    for (int i = 0; i < HASH_MAP_DEFAULT_SIZE; ++i) {
        if (NULL == hashMap->socket_id[i]) continue;
        if (compareKeys(hashMap->socket_id[i], socketId)) return true;
    }
    return false;
}
bool insertSocket(HashMap hashMap,Socket socket){
    if(!hashMap || !socket)
        return false;
    if(hashMap->number_of_sockets == HASH_MAP_DEFAULT_SIZE)
        return false;
    /*(if(hasKey(hashMap,socket->id))
        return false;*/
    // unsigned long offset = 0;
    // unsigned long global_size = sizeof(Socket)*HASH_MAP_DEFAULT_SIZE;
    for(int i =0 ; i<HASH_MAP_DEFAULT_SIZE; i = i+1){
        // offset = sizeof(Socket)*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if(hashMap->table[i] == NULL)
            continue;
        if(socket->id == hashMap->table[i]->id)
            return false;
    }
    for(int j =0 ; j<HASH_MAP_DEFAULT_SIZE; j = j+1){
        //offset = sizeof(Socket)*j;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if(hashMap->table[j] == NULL) {
            hashMap->table[j] = socket;
            hashMap->socket_id[j] = socket->id;
            hashMap->number_of_sockets = hashMap->number_of_sockets +1;
            if(socket == hashMap->ghost_v)
                hashMap-> ghost_has_v = true;
            return true;
        }
    }
    return false;
}

Socket getSocket(HashMap hashMap,SocketID key){
    // unsigned long offset = 0;
    // unsigned long global_size;
    if (hashMap == NULL || key == NULL)
        return NULL;
    if (getHashMapNumberOfSockets(hashMap) == 0)
        return NULL;
    if(hasKey(hashMap,key) == false){
        return NULL;
    }
    // global_size = sizeof(Socket)*HASH_MAP_DEFAULT_SIZE;
    for(int i =0 ; i<HASH_MAP_DEFAULT_SIZE; i = i+1){
        //offset = sizeof(Socket)*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if(hashMap->table[i] == NULL ||hashMap->socket_id[i] == NULL)
            continue;
        if(compareKeys(key,hashMap->socket_id[i]))
            return hashMap->table[i];
    }
    return NULL;
}

bool hashmapRemove(HashMap hashMap, SocketID key) {

    // int offset = 0;
    // unsigned int global_size;
    if (hashMap == NULL || key == NULL) {
        return false;
    }
    // offset = 0;
    // global_size = sizeof((hashMap->table))*HASH_MAP_DEFAULT_SIZE;
    for(int i =0 ; i<HASH_MAP_DEFAULT_SIZE; i = i+1) {
        // offset = sizeof((hashMap->table))*i;
        // sassert(offset < global_size); //PROBLEM
        // sassert(offset>=0);
        if (hashMap->table[i] == NULL)
            continue;
        if (compareKeys(key, hashMap->table[i]->id)) {
            //destroy_socket(hashMap->table[i]);
            hashMap->table[i] = NULL;
            hashMap->number_of_sockets = hashMap->number_of_sockets -1;
            return true;
        }
    }
    return false;
}
bool hashDestroy(HashMap hashMap){

    //int offset = 0;
    //unsigned int global_size;
    if(!hashMap)
    {
       return false;
    }
    //offset = 0;
    //global_size = sizeof((hashMap->table))*HASH_MAP_DEFAULT_SIZE;
    for(int i =0 ; i<HASH_MAP_DEFAULT_SIZE; i = i+1) {
        //offset = sizeof((hashMap->table))*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if (hashMap->table[i] == NULL)
            continue;
        //destroy_socket(hashMap->table[i]);
        destroy_socket(hashMap->table[i]);
    }

    free(hashMap);
    return true;
}
int getHashMapSize(HashMap hashMap){
    return hashMap->size;
}
int getHashMapNumberOfSockets(HashMap hashMap){
    if(hashMap == NULL)
        return -1;
    return hashMap->number_of_sockets;
}

SocketID hashMapGetFirst(HashMap hashmap) {
    if (NULL == hashmap) return NULL;
    for (int i = 0; i < getHashMapSize(hashmap); ++i) {
        if (hashmap->socket_id[i] != NULL) {
            hashmap->iterator_index = i;
            return hashmap->socket_id[i];
        }
    }
    return NULL;
}
SocketID hashMapGetNext(HashMap hashmap) {
    if (NULL == hashmap) return NULL;

    if (hashmap->iterator_index >= getHashMapSize(hashmap)) return NULL;

    hashmap->iterator_index++;

    for (int i = hashmap->iterator_index; i < getHashMapSize(hashmap); ++i) {
        if (hashmap->socket_id[i] != NULL) {
            hashmap->iterator_index = i;
            return hashmap->socket_id[i];
        }
    }
    return NULL;
}