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
#include "socket_utils.h"
#include "array_queue.h"
#include "Functions.h"
//#include<seahorn/seahorn.h>

extern int nd();
/*
void nd_ip(char* dst){
    int ip1 = nd();
    //assume(ip1 >=0 && ip1<=255);
    char ip1_str[20];
    sprintf(ip1_str,"%d",ip1); // TODO: change to our sprintf_t
    strcat_t(ip1_str,"."); // TODO: change to our strcat_t

    int ip2 = nd();
    //assume(ip2 >=0 && ip2<=255);
    char ip2_str[5];
    sprintf(ip2_str,"%d",ip2); // TODO: change to our sprintf_t
    strcat_t(ip2_str,"."); // TODO: change to our strcat_t
    strcat_t(ip1_str,ip2_str); // TODO: change to our strcat_t

    int ip3 = nd();
    //assume(ip3 >=0 && ip3<=255);
    char ip3_str[5];
    sprintf(ip3_str,"%d",ip3); // TODO: change to our sprintf_t
    strcat_t(ip3_str,"."); // TODO: change to our strcat_t
    strcat_t(ip1_str,ip3_str); // TODO: change to our strcat_t

    int ip4 = nd();
    //assume(ip4 >=0 && ip4<=255);
    char ip4_str[5];
    sprintf(ip4_str,"%d",ip4); // TODO: change to our sprintf_t
    strcat_t(ip1_str,ip4_str); // TODO: change to our strcat_t
    strcpy_t(dst,ip1_str);

}
*/
void hashMapTests(){
    ////creation TEST
    HashMapErrors *err = xmalloc(sizeof(*err));
    HashMap hash_map1 = createHashMap(1);
    assert(hash_map1 != NULL);
    assert(getHashMapSize(hash_map1) == 1);
    /////insertion TEST

    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    socket1ID->src_ip = xmalloc(sizeof(*socket1ID->src_ip)*20);
    assert(socket1ID->src_ip != NULL);
    //nd_ip(socket1ID->src_ip);
    strcpy_t(socket1ID->src_ip,"192.168.1.240");
    socket1ID->dst_ip = xmalloc(sizeof(*socket1ID->dst_ip)*20);
    assert(socket1ID->dst_ip != NULL);
    //nd_ip(socket1ID->dst_ip);
    strcpy_t(socket1ID->dst_ip,"192.168.1.242");
    ////assume((socket1ID->src_ip,socket1ID->dst_ip)!= 0);
    socket1ID->src_port = 1;
    socket1ID->dst_port = 2;
    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    assert(socket1->id != NULL);
    assert(socket1->id == socket1ID);
    assert(socket1->send_window != NULL);
    assert(socket1->recv_window != NULL);
    assert(getHashMapNumberOfSockets(hash_map1) == 0);
    Socket check1 = getSocket(hash_map1,socket1ID);
    assert(check1 == NULL);

    //insertion of the first socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-||)
    HashMapErrors value = insertSocket(hash_map1,socket1ID,socket1);
    //assume(value == HASH_MAP_SUCCESS);
    assert(getHashMapNumberOfSockets(hash_map1) == 1);
    //assert(getHashMapNumberOfSockets(hash_map1) == 50); /////////////////

    //creation of socket2 with the same id as socket 1
    Socket check2 = getSocket(hash_map1,socket1ID);
    assert(check2 == socket1);

    //HashMapErrors value = insertSocket(hash_map1,socketID2,socket2);
    //assert(value == HASH_MAP_KEY_EXIST);

    //ID3 for Socket3
    SocketID socket3ID= xmalloc(sizeof(*socket3ID));
    socket3ID->src_ip = xmalloc(sizeof((*socket3ID->dst_ip))*20);
    assert(socket3ID->src_ip != NULL);
    strcpy_t(socket3ID->src_ip,"192.168.1.244");
    socket3ID->dst_ip = xmalloc(sizeof((*socket3ID->dst_ip))*20);
    assert(socket3ID->dst_ip != NULL);
    strcpy_t(socket3ID->dst_ip,"192.168.1.246");
    socket3ID->src_port = 1;
    socket3ID->dst_port = 2;
    //creation of socket
    Socket socket3 = create_new_socket();
    assert(socket3 != NULL);
    socket3->id = socket3ID;
    //insertion of the second socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-||)
    HashMapErrors value2 = insertSocket(hash_map1,socket3ID,socket3);
    //assume(value2 == HASH_MAP_SUCCESS);
    assert(getHashMapNumberOfSockets(hash_map1) == 2);
    Socket check3 = getSocket(hash_map1,socket3ID);
    ////assume(check3 != socket1 && check3!=NULL);
    assert(check3 == socket3);
    //ID4 for socket4:
    SocketID socket4ID= xmalloc(sizeof(*socket4ID));
    socket4ID->src_ip = xmalloc(sizeof((*socket4ID->dst_ip))*20);
    assert(socket4ID->src_ip != NULL);
    strcpy_t(socket4ID->src_ip,"192.168.1.248");
    socket4ID->dst_ip = xmalloc(sizeof((*socket4ID->dst_ip))*20);
    assert(socket4ID->dst_ip != NULL);
    strcpy_t(socket4ID->dst_ip,"192.168.1.250");
    socket4ID->src_port = 1;
    socket4ID->dst_port = 3;
    //creation of socket4:
    Socket socket4 = create_new_socket();
    socket4->id = socket4ID;
    //insertion of the third socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-[socket4]<-||)
    HashMapErrors value3 = insertSocket(hash_map1,socket4ID,socket4);
    //assume(value3 == HASH_MAP_SUCCESS);
    assert(getHashMapNumberOfSockets(hash_map1) == 3);
    Socket check4 = getSocket(hash_map1,socket4ID);
    assert(check4 == socket4);

    //getSocket Tests:
    //input: socket5 that isn't in the hashmap
    //expected output: err should return with the value: HASH_MAP_SOCKET_NOT_FOUND
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    socket5ID->src_ip = xmalloc(sizeof((*socket5ID->dst_ip))*20);
    assert(socket5ID->src_ip != NULL);
    strcpy_t(socket5ID->src_ip,"192.168.1.250");
    socket5ID->dst_ip = xmalloc(sizeof((*socket5ID->dst_ip))*20);
    assert(socket5ID->dst_ip != NULL);
    strcpy_t(socket5ID->dst_ip,"192.168.1.252");
    socket5ID->src_port =1;
    socket5ID->dst_port = 4;
    //Socket getCheckSocket2 = getSocket(hash_map1,socket5ID,err);
    //////assume(*err = HASH_MAP_SOCKET_NOT_FOUND);
    //assert(getCheckSocket2 == NULL);
    //input: socket4 that is in the hashmap
    //expected output: err should return with the value: HASH_MAP_SUCCESS
    Socket getCheckSocket1 = getSocket(hash_map1,socket4ID);
    ////assume(*err = HASH_MAP_SUCCESS);
    assert(compareKeys(getCheckSocket1->id,socket4ID));

    //hashmapRemove Tests
    //Trying to remove a null argument - should not work
    hashmapRemove(hash_map1,NULL,err);
    assert(*err == HASH_MAP_NULL_ARGUMENT);
    hashmapRemove(NULL,socket1ID,err);
    assert(*err == HASH_MAP_NULL_ARGUMENT);
    hashmapRemove(NULL,NULL,err);
    assert(*err == HASH_MAP_NULL_ARGUMENT);
    //Trying to remove an element that isnt in the hasmap - should not work
    hashmapRemove(hash_map1,socket5ID,err);
    assert(getHashMapNumberOfSockets(hash_map1) == 3);
    //removing socket4
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-||)
    hashmapRemove(hash_map1,socket4ID,err);
    assert(getHashMapNumberOfSockets(hash_map1) == 2);
    Socket getCheckSocket3 = getSocket(hash_map1,socket4ID);
    assert(getCheckSocket3 == NULL);
    //removing socket3
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-||)
    hashmapRemove(hash_map1,socket3ID,err);
    assert(getHashMapNumberOfSockets(hash_map1) == 1);
    hashmapRemove(hash_map1,socket3ID,err);

    ////////////////hashDestroy Function/////////////////////
    hashDestroy(NULL,err);
    assert(*err == HASH_MAP_NULL_ARGUMENT);
    hashDestroy(hash_map1,err);
    assert(*err == HASH_MAP_SUCCESS);
    printf("---------\n");
    printf("HASHMAP TESTS SUCCESS\n");

}



typedef struct toupleInt_t
{
    int left;
    int right;
}*ToupleInt;
bool compareTouples(ToupleInt t1, ToupleInt t2){
    ////assume(t1 != NULL && t2!= NULL);
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
    ////assume(t != NULL);
    ToupleInt copy = xmalloc(sizeof(*copy));
    copy->right = t->right;
    copy->left = t->left;
    ////assume(compareTouples(copy,t) == true);
    return copy;
}
void queueTests(){
    Queue queue = QueueCreate(5);
    assert(QueueCapacity(queue) == 5);
    assert(QueueSize(queue) == 0);

    ToupleInt t1 = xmalloc(sizeof(*t1));
    t1->left = 1;
    t1->right = 1;
    enqueue(queue,t1);
    assert(QueueSize(queue) == 1);
    ToupleInt t2 = xmalloc(sizeof(*t2));
    ////assume(t1 != t2);
    t2->left = 1;
    t2->right = 2;
    enqueue(queue,t2);
    assert(QueueSize(queue) == 2);
    ToupleInt t3 = xmalloc(sizeof(*t3));
    t3->left = 1;
    t3->right = 3;
    enqueue(queue,t3);
    assert(QueueSize(queue) == 3);
    ToupleInt first = dequeue(queue);
    //assume(first != NULL);
    assert(first == t1);
    assert(QueueSize(queue) == 2);
    ToupleInt second = dequeue(queue);
    //assume(second != NULL);
    assert(second == t2);
    assert(QueueSize(queue) == 1);
    printf("---------\n");
    printf("QUEUE TESTS SUCCESS\n");
}

void xmallocTest(){
    int* p1 = xmalloc(sizeof(*p1));
    *p1 = 55;
    int* p2 = xmalloc(sizeof(*p2));
    *p2 = 66;
    //////assume(*p1 != 66);
    assert(p1 != p2);
}

void getFirstIterTest(){
    HashMap hash_map2 = createHashMap(0);
    assert(hashMapGetFirst(hash_map2) == NULL);
    Queue  queue2 = QueueCreate(5);
    ToupleInt check1 = QueueGetFirst(queue2);
    assert(check1 == NULL);
    ToupleInt t1 = xmalloc(sizeof(*t1));
    t1->left = 1;
    t1->right = 1;
    bool b1 = enqueue(queue2,t1);
    assert(b1 == true);
    ToupleInt check3 = QueueGetFirst(queue2);
    assert(check3 == t1);
    ToupleInt check4 = QueueGetNext(queue2);
    assert(check4 == NULL);
    Queue  queue3 = QueueCreate(5);
    ToupleInt t2 = xmalloc(sizeof(*t2));
    ////assume(t2 != t1);
    t2->right = 1;
    t2->left = 2;
    ToupleInt t3 = xmalloc(sizeof(*t3));
    ////assume(t3!=t2 && t3!=t1);
    t3->left = 1;
    t3->right = 3;
    bool b2 = enqueue(queue3,t2);
    assert(b2 == true);
    bool b3 = enqueue(queue3,t3);
    assert(b3 == true);
    ToupleInt check5 = QueueGetFirst(queue3);
    assert(check5 == t2);
    ToupleInt check6 = QueueGetNext(queue3);
    assert(check6 == t3);
    ToupleInt check7 = QueueGetNext(queue3);
    assert(check7 == NULL);
    printf("---------\n");
    printf("HASHMAP ITER TESTS SUCCESS\n");
}
void getSocketTest(){
    HashMapErrors* err = xmalloc(sizeof(*err));
    HashMap hash_map3 = createHashMap(1);
    assert(hash_map3 != NULL);
    assert(getHashMapSize(hash_map3) == 1);
    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    socket1ID->src_ip = xmalloc(sizeof(*socket1ID->src_ip)*20);
    assert(socket1ID->src_ip != NULL);
    strcpy_t(socket1ID->src_ip,"192.168.1.240");
    socket1ID->dst_ip = xmalloc(sizeof(*socket1ID->dst_ip)*20);
    assert(socket1ID->dst_ip != NULL);
    strcpy_t(socket1ID->dst_ip,"192.168.1.242");
    socket1ID->src_port = 1;
    socket1ID->dst_port = 2;
    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    //GET_SOCKET_CREATION
    Socket check1 = getSocket(hash_map3,socket1ID);
    assert(check1 == NULL);
    HashMapErrors value = insertSocket(hash_map3,socket1ID,socket1);
    assert(getHashMapNumberOfSockets(hash_map3) == 1);
    //assume(value == HASH_MAP_SUCCESS);
    //ID1 for Socket2
    SocketID socket2ID= xmalloc(sizeof(*socket1ID));
    //assume(socket2ID != socket1->id && socket2ID != socket1ID);
    socket2ID->src_ip = xmalloc(sizeof(*socket2ID->src_ip)*20);
    assert(socket2ID->src_ip != NULL);
    strcpy_t(socket1ID->src_ip,"192.168.1.246");
    socket2ID->dst_ip = xmalloc(sizeof(*socket2ID->dst_ip)*20);
    assert(socket2ID->dst_ip != NULL);
    strcpy_t(socket1ID->dst_ip,"192.168.1.248");
    socket2ID->src_port = 1;
    socket2ID->dst_port = 2;
    //Socket2 creation
    Socket socket2 = create_new_socket();
    //assume(socket2 != socket1);
    socket2->id = socket2ID;
    //assume(socket1->id == socket1ID && compareKeys(socket1->id,socket2ID) == false);
    HashMapErrors value2 = insertSocket(hash_map3,socket2ID,socket2);
    //assume(value2 == HASH_MAP_SUCCESS);
    assert(getHashMapNumberOfSockets(hash_map3) == 2);
    //assume(socket2->id == socket2ID);
    Socket check2 = getSocket(hash_map3,socket1ID);
    assert(check2 == socket1);
    Socket check3 = getSocket(hash_map3,socket2ID);
    assert(check3 == socket2);
    printf("---------\n");
    printf("HASHMAP GET SOCKET TEST SUCCESS\n");

}
int main() {
    queueTests();
    getFirstIterTest();
    getSocketTest();
    hashMapTests();
    //xmallocTest();

    char* test = malloc(sizeof(*test)*20);
    //nd_ip(test);
    printf("%s",test);
}

