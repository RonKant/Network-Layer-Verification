//
// Created by Ido Yam on 02/05/2020.
//

#include "Hashmap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "network.h"
#include "socket_utils.h"
#include "util_types.h"


#define sassert(X) ;
#define assume(X) ;

//#include "seahorn/seahorn.h"
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
    //sassert(str1 != NULL && str2 !=NULL);
    if (NULL == str1) {
        return NULL != str2;
    }
    while(*str1 && (*str1==*str2))
        str1++,str2++;
    return *(const unsigned char*)str1-*(const unsigned char*)str2;
}
char* strcpy_t(char* dest, char* source){
    //assume(dest > 0 && source > 0);
    if(dest == NULL)
        return NULL;
    char* ptr = dest;
    while(*source != '\0'){
        *dest = *source;
        dest++;
        source++;
    }
    *dest = '\0';
    return ptr;
}

void* xmalloc(size_t sz){
    void *p;
    p=malloc(sz);
    //assume(p>0);
    return p;
}

struct HashMap_t{
    int size;
    int number_of_sockets;
    Node* table;
    Node iterator;
    // copySocket socketCopyFunction;
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
// Socket socketCopy(Socket socket,HashMapErrors *error){
//     Socket s_copy = xmalloc(sizeof(*s_copy));
//     if(s_copy == NULL) {
//         *error = HASH_MAP_ALLOCATION_FAIL;
//         return NULL;
//     }
//     s_copy->id = copyKeyFunction(socket->id,error);
//     s_copy->state = socket->state;
//     s_copy->send_window = xmalloc(sizeof(socket->send_window));
//     if(s_copy->send_window == NULL){
//         *error = HASH_MAP_ALLOCATION_FAIL;
//         return NULL;
//     }
//     if(socket->send_window != NULL)
//         strcpy_t(socket->send_window,s_copy->send_window);
//     s_copy->send_window_size = socket->send_window_size;
//     s_copy->max_send_window_size = socket->max_send_window_size;
//     s_copy->seq_of_first_send_window = socket->seq_of_first_send_window;

//     s_copy->recv_window = xmalloc(sizeof(socket->recv_window));
//     if(s_copy->recv_window == NULL){
//         *error = HASH_MAP_ALLOCATION_FAIL;
//         return NULL;
//     }
//     if(socket->recv_window != NULL)
//         strcpy_t(socket->recv_window,s_copy->recv_window);
//     s_copy->recv_window_size = socket->recv_window_size;
//     s_copy->max_recv_window_size = socket->max_recv_window_size;
//     s_copy->seq_of_first_recv_window = socket->seq_of_first_recv_window;

//     return s_copy;
// }

// Socket socketCopy(Socket socket, HashMapErrors *error) {
//     Socket s_copy = xmalloc(sizeof(*s_copy));
//     if (NULL == s_copy) {
//         *error = HASH_MAP_ALLOCATION_FAIL;
//         return NULL;
//     }

//     s_copy->id = copyKeyFunction(socket->id, error);
//     s_copy->state = socket->state;

//     s_copy->listen_fifo_read_end = socket->listen_fifo_read_end;
//     s_copy->listen_fifo_write_end = socket->listen_fifo_write_end;
//     s_copy->accept_fifo_write_end = socket->accept_fifo_write_end;
//     s_copy->out_fifo_read_end= socket->out_fifo_read_end;
//     s_copy->in_fifo_write_end = socket->in_fifo_write_end;
//     s_copy->end_fifo_read_end = socket->end_fifo_read_end;
//     s_copy->end_fifo_write_end = socket->end_fifo_write_end;

//     s_copy->send_window = socket->send_window;
//     s_copy->seq_of_first_send_window = socket->
// }


HashMapErrors socketFree(Socket socket){
    return HASH_MAP_SUCCESS;
    if(socket == NULL)
        return HASH_MAP_NULL_ARGUMENT;
    destroy_socket(socket);
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
        (hashMap->table)[i] = NULL;
    }
    hashMap->compareSocketFunction =  socketCompare;
    // hashMap->socketCopyFunction=socketCopy;
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
    if(!hashMap || key ==NULL || !socket)
        return HASH_MAP_NULL_ARGUMENT;
    assume(hashMap>0 && key>0 && socket>0);
    //int pos = hashCode(hashMap,key);
    int pos =1;
    //sassert(pos >= 0 && pos < hashMap->size);
    Node posList = hashMap->table[pos];
    Node newNode = xmalloc(sizeof(*newNode));

    Node tmp = posList;
    while(tmp != NULL){
        assume(tmp->key !=NULL);
        bool res = compareKeys(key,tmp->key);
        if(res){
            free(newNode);
            return HASH_MAP_KEY_EXIST;
        }
        tmp=tmp->next;
    }
    // HashMapErrors *err=NULL;
    Socket newSocket = socket;
    // Socket newSocket = socketCopy(socket,err);
    // sassert(newSocket !=NULL);
    newNode->socket=newSocket;
    // newNode->key = copyKeyFunction(key,err);
    newNode->key = key;
    // sassert(newNode->key !=NULL);
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
            if (NULL != error) *error = HASH_MAP_SUCCESS;
            return tmp->socket;
        }
        tmp=tmp->next;
    }
    if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
    return NULL;
}

SocketID hashMapGetFirst(HashMap hashMap) {
    assert(hashMap != NULL);
    if (hashMap->size == 0) return NULL;
    hashMap->iterator = hashMap->table[0];
    return hashMap->iterator->key;
}
void hashMapSetFirst(HashMap hashMap){
    if(hashMap == NULL || hashMap->size == 0)
        return;
    hashMap->iterator = hashMap->table[0];
}
bool hashMapHasNext(HashMap hashMap){
    return hashMap->iterator->next == NULL;
}
SocketID hashMapGetNext(HashMap hashMap){
    if(hashMap == NULL || hashMap->size == 0)
        return NULL;
    if(hashMap->size == 1){
        return (hashMap->iterator->key);
    }
    else{
        // if the list where the node ends is done than we need the next list
        if(!hashMapHasNext(hashMap)){
            int index_in_table = hashCode(hashMap,hashMap->iterator->key);
            if (index_in_table == hashMap->size - 1) {
                // reached end of iteration
                return NULL;
            }
            return (hashMap->table[(index_in_table+1)%hashMap->size]->key);
        }
        else{
            return (hashMap->iterator->next->key);
        }
    }
}

void hashmapRemove(HashMap hashMap, SocketID key, HashMapErrors *error) {
    if (hashMap == NULL || key == NULL) {
        if (NULL != error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    int pos = hashCode(hashMap, key);
    Node posList = hashMap->table[pos];
    Node tmp = posList;
    if (!tmp) {
        if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
        return; //the list is empty
    }
    //if iterator points to the element we remove
    if(hashMap->iterator != NULL && compareKeys(key,hashMap->iterator->key)){
        if(hashMap->size == 1){
            hashMap->iterator = NULL;
        }
        else{
            // if the list where the node ends is done than we need the next list
            if(!hashMapHasNext(hashMap)){
                int index_in_table = hashCode(hashMap,hashMap->iterator->key);
                hashMap->iterator = hashMap->table[(index_in_table+1)%hashMap->size];
            }
            else{
                hashMap->iterator = hashMap->iterator->next;
            }
        }
    }

    if (tmp->next == NULL) {
        if (hashMap->compareKeyFunction(key, tmp->key))//one element in the posList
        {
            hashMap->freeKeyFunction(tmp->key);
            hashMap->freeSocketFuncition(tmp->socket);
            free(tmp);
            hashMap->table[pos] = NULL;
            hashMap->number_of_sockets--;
            if (NULL != error) *error = HASH_MAP_SUCCESS;
            return;
        } else{
            if (NULL != error) *error = HASH_MAP_SOCKET_NOT_FOUND;
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
        if (error) *error = HASH_MAP_NULL_ARGUMENT;
        return;
    }
    for(int i=0; i<hashMap->size;i++){
        Node tmp= hashMap->table[i];
        while(tmp){
            hashmapRemove(hashMap,tmp->key,error);
            if((error) && (*error != HASH_MAP_SUCCESS)) {
                *error = HASH_MAP_ERROR;
                return;
            }
            tmp = hashMap->table[i];
        }
    }
    free(hashMap->table);
    free(hashMap);
    if (error) *error = HASH_MAP_SUCCESS;
}
int getHashMapSize(HashMap hashMap){
    return hashMap->size;
}
int getHashMapNumberOfSockets(HashMap hashMap){
    return hashMap->number_of_sockets;
}






