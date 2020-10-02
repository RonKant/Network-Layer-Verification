#include <stdio.h>
#include "Hashmap.h"
#include "util_types.h"
#include "network.h"
#include "ip.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include<stdint.h>
#include<stddef.h>
#include "queue.h"
#include<seahorn/seahorn.h>

extern int nd();

void hashMapTests(){
    ////creation TEST
    HashMapErrors *err = xmalloc(sizeof(*err));
    HashMap hash_map1 = createHashMap(2);
    sassert(hash_map1 != NULL);
    /////insertion TEST
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    socket1ID->src_ip = xmalloc(sizeof(*socket1ID->src_ip)*15+1);
    sassert(socket1ID->src_ip != NULL);
    socket1ID->src_ip = "192.168.1.240";
    socket1ID->dst_ip = xmalloc(sizeof(*socket1ID->src_ip)*15+1);
    sassert(socket1ID->dst_ip != NULL);
    socket1ID->dst_ip = "192.168.1.242";
    socket1ID->src_port = 1;
    socket1ID->dst_port = 2;

    Socket socket1 = xmalloc(sizeof(*socket1));
    socket1->id = socket1ID;
    socket1->send_window = xmalloc(sizeof(*socket1->send_window)*10+1);
    sassert(socket1->send_window != NULL);
    socket1->recv_window = xmalloc(sizeof(*socket1->recv_window)*10+1);
    sassert(socket1->recv_window != NULL);

    SocketID socketID2 = socket1ID;
    Socket socket2 = socket1;

    sassert(getHashMapSize(hash_map1) == 2);
    sassert(getHashMapNumberOfSockets(hash_map1) == 0);

    HashMapErrors value = insertSocket(hash_map1,socket1ID,socket1);
    sassert(value == HASH_MAP_SUCCESS);
    /*sassert(compareKeys(socket1ID,socketID2) == 1);

    //HashMapErrors value = insertSocket(hash_map1,socketID2,socket2);
    //assume(value != HASH_MAP_NULL_ARGUMENT && value != HASH_MAP_ALLOCATION_FAIL && value != HASH_MAP_SUCCESS);
    //sassert(value == HASH_MAP_KEY_EXIST);


    SocketID socket3ID= xmalloc(sizeof(*socket1ID));
    socket3ID->src_ip = xmalloc(sizeof((*socket3ID->src_ip)*13)+1);
    sassert(socket3ID->src_ip != NULL);
    socket3ID->src_ip = "192.168.1.244";
    socket3ID->dst_ip = xmalloc(sizeof((*socket3ID->dst_ip)*13)+1);
    sassert(socket3ID->dst_ip != NULL);
    socket3ID->dst_ip = "192.168.1.246";
    socket3ID->src_port = 1;
    socket3ID->dst_port = 2;

    Socket socket3 = xmalloc(sizeof(*socket3));
    socket3->id = socket3ID;
    socket3->send_window = xmalloc(sizeof(*socket3->send_window)*10+1);
    sassert(socket3->send_window != NULL);
    socket3->recv_window = xmalloc(sizeof(*socket3->recv_window)*10+1);
    sassert(socket3->recv_window != NULL);

    sassert(getHashMapSize(hash_map1) == 2);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);

    HashMapErrors value = insertSocket(hash_map1,socket3ID,socket3);
    assume(value != HASH_MAP_NULL_ARGUMENT && value != HASH_MAP_ALLOCATION_FAIL && value != HASH_MAP_KEY_EXIST);
    sassert(value == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);

    SocketID socket4ID= xmalloc(sizeof(*socket4ID));
    socket4ID->src_ip = xmalloc(sizeof((*socket4ID->src_ip)*13)+1);
    sassert(socket4ID->src_ip != NULL);
    socket4ID->src_ip = "192.168.1.248";
    socket4ID->dst_ip = malloc(sizeof((*socket4ID->src_ip)*13)+1);
    sassert(socket4ID->dst_ip != NULL);
    socket4ID->dst_ip = "192.168.1.250";
    socket4ID->src_port = 1;
    socket4ID->dst_port = 2;

    Socket socket4 = xmalloc(sizeof(*socket4));
    socket4->id = socket4ID;
    socket4->send_window = xmalloc(sizeof(*socket4->send_window)*10+1);
    sassert(socket4->send_window != NULL);
    socket4->recv_window = xmalloc(sizeof(*socket4->recv_window)*10+1);
    sassert(socket4->recv_window != NULL);

    HashMapErrors value2 = insertSocket(hash_map1,socket4ID,socket4);
    assume(value2 != HASH_MAP_NULL_ARGUMENT && value2 != HASH_MAP_ALLOCATION_FAIL && value2 != HASH_MAP_KEY_EXIST);
    sassert(value2 == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 3);

    //getSocket Tests
    SocketID socket5ID= malloc(sizeof(*socket5ID));
    socket5ID->src_ip = malloc(sizeof((*socket5ID->src_ip)*13)+1);
    sassert(socket5ID->src_ip != NULL);
    socket5ID->src_ip = "192.168.2.248";
    socket5ID->dst_ip = malloc(sizeof((*socket5ID->src_ip)*13)+1);
    sassert(socket5ID->dst_ip != NULL);
    socket5ID->dst_ip = "192.168.2.250";
    socket5ID->src_port = 1;
    socket5ID->dst_port = 2;


    Socket getCheckSocket1 = getSocket(hash_map1,socket4ID,err);
    sassert(*err == HASH_MAP_SUCCESS && compareKeys(getCheckSocket1->id,socket4ID));
    Socket getCheckSocket2 = getSocket(hash_map1,socket5ID,err);
    sassert(*err == HASH_MAP_SOCKET_NOT_FOUND && getCheckSocket2 == NULL);

    //hashmapRemove Tests
    hashmapRemove(hash_map1,NULL,err);
    sassert(*err == HASH_MAP_NULL_ARGUMENT);
    hashmapRemove(NULL,socket1ID,err);
    sassert(*err == HASH_MAP_NULL_ARGUMENT);
    hashmapRemove(NULL,NULL,err);
    sassert(*err == HASH_MAP_NULL_ARGUMENT);

    hashmapRemove(hash_map1,socket5ID,err);
    sassert(*err == HASH_MAP_ERROR);
    sassert(getHashMapNumberOfSockets(hash_map1) == 3);

    hashmapRemove(hash_map1,socket4ID,err);
    sassert(*err == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);
    Socket getCheckSocket3 = getSocket(hash_map1,socket4ID,err);
    sassert(*err == HASH_MAP_SOCKET_NOT_FOUND && getCheckSocket3 == NULL);

    hashmapRemove(hash_map1,socket3ID,err);
    sassert(*err == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);
    hashmapRemove(hash_map1,socket3ID,err);
    sassert(*err == HASH_MAP_SOCKET_NOT_FOUND);

    ////////////////hashDestroy Function/////////////////////
    hashDestroy(NULL,err);
    sassert(*err == HASH_MAP_NULL_ARGUMENT);
    hashDestroy(hash_map1,err);
    sassert(*err == HASH_MAP_SUCCESS);
    printf("---------\n");
    printf("HASHMAP TESTS SUCCESS\n");
    printf("---------\n");
    */
}

typedef struct toupleInt_t
{
    int left;
    int right;
}*ToupleInt;
bool compareTouples(ToupleInt t1, ToupleInt t2){
    assume(t1 != NULL && t2!= NULL);
    if(t1->left == t2->left && t1->right == t2->right){
        return true;
    }
    return false;
}
void freeTouple(ToupleInt t){
    free(t);
}
ToupleInt copyTouple(ToupleInt t){
    if (t == NULL)
        return NULL;
    assume(t != NULL);
    ToupleInt copy = xmalloc(sizeof(*copy));
    copy->right = t->right;
    copy->left = t->left;
    assume(compareTouples(copy,t) == true);
    return copy;
}
void queueTests(){
    QueueErrors* queue_errors = xmalloc(sizeof(*queue_errors));
    Queue queue = createQueue_g(sizeof(ToupleInt), (compareElem) compareTouples, (freeElem) freeTouple,
                                (copyElem) copyTouple);
    sassert(queue->sizeOfQueue == 0);
    ToupleInt t1 = xmalloc(sizeof(*t1));
    t1->left = 1;
    t1->right = 1;
    enqueue(queue,t1);
    sassert(queue->sizeOfQueue == 1);
    ToupleInt t2 = xmalloc(sizeof(*t2));
    assume(t1 != t2);
    t2->left = 1;
    t2->right = 2;
    enqueue(queue,t2);
    sassert(queue->sizeOfQueue == 2);
    /*ToupleInt t3 = xmalloc(sizeof(*t3));
    t3->left = 1;
    t3->right = 3;
    enqueue(queue,t3);
    sassert(queue->sizeOfQueue == 3);*/
    ToupleInt first = dequeue(queue,queue_errors);
    assume(first != NULL);
    sassert(*queue_errors == Queue_SUCCESS);
    //EDIT
    sassert(first == t1);
    sassert(queue->sizeOfQueue == 1);
    ToupleInt second = dequeue(queue,queue_errors);
    assume(second != NULL);
    sassert(*queue_errors == Queue_SUCCESS);
    // EDIT
    sassert(second == t2);
    sassert(queue->sizeOfQueue == 0);
    printf("---------\n");
    printf("QUEUE TESTS SUCCESS\n");
    printf("---------\n");
}
void xmallocTest(){
    int* p1 = xmalloc(sizeof(*p1));
    *p1 = 55;
    int* p2 = xmalloc(sizeof(*p2));
    *p2 = 66;
    //assume(*p1 != 66);
    sassert(p1 != p2);
}
void ipToStrTest(){
    char* message = "010100000000000000000000192.168.056.112192.168.056.113";
    IPPacket ipPacket = str_to_ip(message);
    sassert( strcmp_t(ipPacket->src_ip , "194.168.056.112") == 0);
}
int main() {
    //queueTests();
    hashMapTests();
    //xmallocTest();
}

