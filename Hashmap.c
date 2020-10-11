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

#include "seahorn/seahorn.h"

extern void* nd_ptr();
extern int nd();

bool compareKeys(SocketID key1,SocketID key2){
    if(key1 == key2)
        return true;
    return key1->dst_port == key2->dst_port /*&& !strcmp_t(key1->src_ip,key2->src_ip) */&&
           key1->src_port == key2->src_port /*&& !strcmp_t(key1->dst_ip,key2->dst_ip)*/;
}
HashMap createHashMap(){
    unsigned long offset = 0;
    unsigned long global_size;
    HashMap hashMap = xmalloc(sizeof(*hashMap));
    hashMap->size=5;
    global_size = sizeof(Socket)*5;
    for(int i=0; i<5; i = i+1){
        offset = sizeof(Socket)*i;
        sassert(offset < global_size);
        sassert(offset>=0);
        hashMap->table[i] = NULL;
        hashMap->socket_id[i] = NULL;
    }
    hashMap->number_of_sockets = 0;
    hashMap->ghost_v = nd_ptr();
    hashMap->ghost_has_v = false;
    return hashMap;
}
bool hasKey(HashMap hashMap, SocketID socketId){
    if(hashMap->ghost_v->id == socketId)
        return hashMap->ghost_has_v;
    return nd();
}
bool insertSocket(HashMap hashMap,Socket socket){
    if(!hashMap || !socket)
        return false;
    if(hashMap->number_of_sockets == 5)
        return false;
    unsigned long offset = 0;
    unsigned long global_size = sizeof(Socket)*5;
    for(int i =0 ; i<5; i = i+1){
        offset = sizeof(Socket)*i;
        sassert(offset < global_size);
        sassert(offset>=0);
        if(hashMap->table[i] == NULL)
            continue;
        if(socket->id == hashMap->table[i]->id)
            return false;
        if(compareKeys(hashMap->table[i]->id,socket->id))
            return false;
    }
    for(int j =0 ; j<5; j = j+1){
        offset = sizeof(Socket)*j;
        sassert(offset < global_size);
        sassert(offset>=0);
        if(hashMap->table[j] == NULL) {
            hashMap->table[j] = socket;
            hashMap->socket_id[j] = socket->id;
            hashMap->number_of_sockets = hashMap->number_of_sockets +1;
            if(socket == hashMap->ghost_v)
                hashMap->ghost_has_v = true;
            return true;
        }
    }
    return false;
}

Socket getSocket(HashMap hashMap,SocketID key){

    if(hashMap->ghost_v->id == key)
    {
        if(hashMap->ghost_has_v == 1)
        {
            assume(hashMap->ghost_v != NULL);
            return hashMap->ghost_v;
        }
    }
    return NULL;
    /*unsigned long offset = 0;
    unsigned long global_size;
    if (hashMap == NULL || key == NULL)
        return NULL;
    if (getHashMapNumberOfSockets(hashMap) == 0)
        return NULL;
    if(hasKey(hashMap,key) == false){
        return NULL;
    }
    global_size = sizeof(Socket)*5;
    for(int i =0 ; i<5; i = i+1){
        //offset = sizeof(Socket)*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if(hashMap->table[i] == NULL ||hashMap->socket_id[i] == NULL)
            continue;
        if(compareKeys(key,hashMap->socket_id[i]))
            return hashMap->table[i];
    }
    return NULL;
    */
}

bool hashmapRemove(HashMap hashMap, SocketID key) {

    int offset = 0;
    unsigned int global_size;
    if (hashMap == NULL || key == NULL) {
        return false;
    }
    offset = 0;
    global_size = sizeof((hashMap->table))*5;
    for(int i =0 ; i<5; i = i+1) {
        offset = sizeof((hashMap->table))*i;
        sassert(offset < global_size);
        sassert(offset>=0);
        if (hashMap->table[i] == NULL)
            continue;
        if (compareKeys(key, hashMap->table[i]->id)) {
            hashMap->socket_id[i] = NULL;
            hashMap->table[i] = NULL;
            hashMap->number_of_sockets = hashMap->number_of_sockets -1;
            if(hashMap->ghost_v->id == key)
                hashMap->ghost_has_v = 0;
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
    //global_size = sizeof((hashMap->table))*5;
    for(int i =0 ; i<5; i = i+1) {
        //offset = sizeof((hashMap->table))*i;
        //sassert(offset < global_size); //PROBLEM
        //sassert(offset>=0);
        if (hashMap->table[i] == NULL)
            continue;
        //destroy_socket(hashMap->table[i]);
        free(hashMap->table[i]);
    }
    free(hashMap->table);
    free(hashMap->socket_id);
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
