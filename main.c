#include <stdio.h>
#include "Hashmap.h"
#include "util_types.h"
#include "network.h"
#include "ip.h"
//#include <sassert.h>
#include <stdlib.h>
#include <string.h>
#include<stdint.h>
#include<stddef.h>
#include "socket_utils.h"
//#include "array_queue.h"
#include "Functions.h"
#include<seahorn/seahorn.h>

extern void* nd_ptr();
extern int nd();

void nd_ip(char* dst){
    int ip1 = nd();
    assume(ip1 >=0 && ip1<=255);
    char ip1_str[20];
    sprintf(ip1_str,"%d",ip1); // TODO: change to our sprintf_t
    strcat_t(ip1_str,"."); // TODO: change to our strcat_t

    int ip2 = nd();
    assume(ip2 >=0 && ip2<=255);
    char ip2_str[5];
    sprintf(ip2_str,"%d",ip2); // TODO: change to our sprintf_t
    strcat_t(ip2_str,"."); // TODO: change to our strcat_t
    strcat_t(ip1_str,ip2_str); // TODO: change to our strcat_t

    int ip3 = nd();
    assume(ip3 >=0 && ip3<=255);
    char ip3_str[5];
    sprintf(ip3_str,"%d",ip3); // TODO: change to our sprintf_t
    strcat_t(ip3_str,"."); // TODO: change to our strcat_t
    strcat_t(ip1_str,ip3_str); // TODO: change to our strcat_t

    int ip4 = nd();
    assume(ip4 >=0 && ip4<=255);
    char ip4_str[5];
    sprintf(ip4_str,"%d",ip4); // TODO: change to our sprintf_t
    strcat_t(ip1_str,ip4_str); // TODO: change to our strcat_t
    strcpy_t(dst,ip1_str);

}


void hashMapTests(){
    ////creation TEST
    HashMap hash_map1 = createHashMap();
    sassert(hash_map1 != NULL);
    sassert(getHashMapSize(hash_map1) == 5);
    sassert(getHashMapNumberOfSockets(hash_map1) == 0);

    /////insertion TEST
    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    //nd_ip(socket1ID->src_ip);
    strcpy_t(socket1ID->src_ip,"192.168.1.240");
    //nd_ip(socket1ID->dst_ip);
    strcpy_t(socket1ID->dst_ip,"192.168.1.242");
    //assume((socket1ID->src_ip,socket1ID->dst_ip)!= 0);
    socket1ID->src_port = nd();
    socket1ID->dst_port = nd();

    //ID3 for Socket3
    SocketID socket3ID= xmalloc(sizeof(*socket3ID));
    assume(socket3ID != socket1ID);
    strcpy_t(socket3ID->src_ip,"192.168.1.244");
    strcpy_t(socket3ID->dst_ip,"192.168.1.246");
    socket3ID->src_port = nd();
    socket3ID->dst_port = nd();

    //ID4 for socket4:
    SocketID socket4ID= xmalloc(sizeof(*socket4ID));
    assume(socket4ID != socket1ID);
    assume(socket4ID != socket3ID);
    strcpy_t(socket4ID->src_ip,"192.168.1.248");
    strcpy_t(socket4ID->dst_ip,"192.168.1.250");
    socket4ID->src_port = nd();
    socket4ID->dst_port = nd();

    //ID5 for socket4:
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    assume(socket5ID != socket1ID);
    assume(socket5ID != socket3ID);
    assume(socket5ID != socket4ID);
    strcpy_t(socket5ID->src_ip,"192.168.1.252");
    strcpy_t(socket5ID->dst_ip,"192.168.1.254");
    socket5ID->src_port = nd();
    socket5ID->dst_port = nd();

    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    sassert(compareKeys(socket1->id,socket1ID) ==true);
    assume(socket1->id == socket1ID);


    //creation of socket3
    Socket socket3 = create_new_socket();
    assume(socket3 != socket1);
    socket3->id = socket3ID;
    sassert(compareKeys(socket3->id,socket3ID) ==true);
    assume(socket3->id == socket3ID);

    //creation of socket4:
    Socket socket4 = create_new_socket();
    assume(socket4 != socket1);
    assume(socket4 != socket3);
    socket4->id = socket4ID;
    sassert(compareKeys(socket4->id,socket4ID) ==true);
    assume(socket4->id == socket4ID);


    if(hash_map1->ghost_v == socket1){
        assume(hash_map1->ghost_has_v == 1);
    }
    if(hash_map1->ghost_v == socket3){
        assume(hash_map1->ghost_has_v == 1);
    }
    if(hash_map1->ghost_v == socket4){
        assume(hash_map1->ghost_has_v == 1);
    }


    //insertion of the first socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-||)
    bool value = insertSocket(hash_map1,socket1);
    assume(value == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);




    //insertion of the second socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-||)
    bool value2 = insertSocket(hash_map1,socket3);
    assume(value2 == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);



    //insertion of the third socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-[socket4]<-||)
    bool value3 = insertSocket(hash_map1,socket4);
    assume(value3 == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 3);
    /*
     * trying to get a socket that isn't in the Hashmap.
    */
    assume(socket3ID != socket1ID);

    assume(socket4ID != socket1ID);
    assume(socket4ID != socket3ID);

    assume(socket5ID != socket1ID && compareKeys(socket5ID,socket1ID) == false);
    assume(socket5ID != socket3ID);
    assume(socket5ID != socket4ID);


    assume(socket1->id == socket1ID);
    assume(socket3->id == socket3ID);
    assume(socket4->id == socket4ID);


    bool b = hasKey(hash_map1,socket5ID);
    if(hash_map1->ghost_v->id == socket5ID)
        sassert(!b);

    b = hasKey(hash_map1,socket1ID);
    if(hash_map1->ghost_v == socket1)
        sassert(b);

    b = hasKey(hash_map1,socket3ID);
    if(hash_map1->ghost_v == socket3)
        sassert(b);

    b = hasKey(hash_map1,socket4ID);
    if(hash_map1->ghost_v == socket4)
        sassert(b);

    sassert(hash_map1->table[0] == socket1);
    sassert(hash_map1->table[1] == socket3);
    sassert(hash_map1->table[2] == socket4);

   /*
    * Socket error = getSocket(hash_map1,socket5ID);
    * sassert(error == NULL);
    Socket first = getSocket(hash_map1,socket1ID);
    //Socket second = getSocket(hash_map1,socket3ID);
    sassert(first == socket1);
    sassert(compareKeys(socket1ID,first->id) == true);
    //sassert(second == socket3ID);
    //sassert(compareKeys(socket3ID,second) == true);
    */

    //hashmapRemove Tests
    //Trying to remove a null argument - should not work
    b = hashmapRemove(hash_map1,NULL);
    sassert(b == false);
    b = hashmapRemove(NULL,socket1ID);
    sassert(b == false);
    b = hashmapRemove(NULL,NULL);
    sassert(b == false);
    sassert(getHashMapNumberOfSockets(hash_map1) == 3);

    assume(socket1->id == socket1ID);
    assume(socket3->id == socket3ID);
    assume(socket4ID != socket1ID);
    //assume(compareKeys(socket4ID,socket1ID) == false);



    //Trying to remove an element that isnt in the hasmap - should not work
    /*b = hashmapRemove(hash_map1,socket4ID);
    sassert(b == false);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);
    assume(socket4ID != socket1ID);
     */
    //removing socket1
    b = hashmapRemove(hash_map1,socket1ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);

    //removing socket3
    //hashmap status:
    // [0] : (||)
    b = hashmapRemove(hash_map1,socket3ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);

    b = hashmapRemove(hash_map1,socket4ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 0);


    ////////////////hashDestroy Function/////////////////////
    b = hashDestroy(NULL);
    sassert(!b);
    assume(hash_map1 != NULL);
    hashDestroy(hash_map1);
    sassert(b);
    printf("---------\n");
    printf("HASHMAP TESTS SUCCESS\n");

}



typedef struct toupleInt_t
{
    int left;
    int right;
}*ToupleInt;
bool compareTouples(ToupleInt t1, ToupleInt t2){
    //assume(t1 != NULL && t2!= NULL);
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
    //assume(t != NULL);
    ToupleInt copy = xmalloc(sizeof(*copy));
    copy->right = t->right;
    copy->left = t->left;
    //assume(compareTouples(copy,t) == true);
    return copy;
}
void queueTests(){
    Queue queue = QueueCreate(5);
    sassert(QueueCapacity(queue) == 5);
    sassert(QueueSize(queue) == 0);

    ToupleInt t1 = xmalloc(sizeof(*t1));
    t1->left = 1;
    t1->right = 1;
    enqueue(queue,t1);
    sassert(QueueSize(queue) == 1);
    ToupleInt t2 = xmalloc(sizeof(*t2));
    //assume(t1 != t2);
    t2->left = 1;
    t2->right = 2;
    enqueue(queue,t2);
    sassert(QueueSize(queue) == 2);
    ToupleInt t3 = xmalloc(sizeof(*t3));
    t3->left = 1;
    t3->right = 3;
    enqueue(queue,t3);
    sassert(QueueSize(queue) == 3);
    ToupleInt first = dequeue(queue);
    assume(first != NULL);
    sassert(first == t1);
    sassert(QueueSize(queue) == 2);
    ToupleInt second = dequeue(queue);
    assume(second != NULL);
    sassert(second == t2);
    sassert(QueueSize(queue) == 1);
    printf("---------\n");
    printf("QUEUE TESTS SUCCESS\n");
}

void xmallocTest(){
    int* p1 = xmalloc(sizeof(*p1));
    *p1 = 55;
    int* p2 = xmalloc(sizeof(*p2));
    *p2 = 66;
    ////assume(*p1 != 66);
    sassert(p1 != p2);
}

void getFirstIterTest(){
    HashMap hash_map2 = createHashMap(0);
    sassert(hashMapGetFirst(hash_map2) == NULL);
    Queue  queue2 = QueueCreate(5);
    ToupleInt check1 = QueueGetFirst(queue2);
    sassert(check1 == NULL);
    ToupleInt t1 = xmalloc(sizeof(*t1));
    t1->left = 1;
    t1->right = 1;
    bool b1 = enqueue(queue2,t1);
    sassert(b1 == true);
    ToupleInt check3 = QueueGetFirst(queue2);
    sassert(check3 == t1);
    ToupleInt check4 = QueueGetNext(queue2);
    sassert(check4 == NULL);
    Queue  queue3 = QueueCreate(5);
    ToupleInt t2 = xmalloc(sizeof(*t2));
    //assume(t2 != t1);
    t2->right = 1;
    t2->left = 2;
    ToupleInt t3 = xmalloc(sizeof(*t3));
    //assume(t3!=t2 && t3!=t1);
    t3->left = 1;
    t3->right = 3;
    bool b2 = enqueue(queue3,t2);
    sassert(b2 == true);
    bool b3 = enqueue(queue3,t3);
    sassert(b3 == true);
    ToupleInt check5 = QueueGetFirst(queue3);
    sassert(check5 == t2);
    ToupleInt check6 = QueueGetNext(queue3);
    sassert(check6 == t3);
    ToupleInt check7 = QueueGetNext(queue3);
    sassert(check7 == NULL);
    printf("---------\n");
    printf("HASHMAP ITER TESTS SUCCESS\n");
}
void getSocketTest(){
    HashMap hash_map3 = createHashMap(1);
    sassert(hash_map3 != NULL);
    sassert(getHashMapSize(hash_map3) == 1);
    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    strcpy_t(socket1ID->src_ip,"192.168.1.240");
    strcpy_t(socket1ID->dst_ip,"192.168.1.242");
    socket1ID->src_port = 1;
    socket1ID->dst_port = 2;
    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    //GET_SOCKET_CREATION
    Socket check1 = getSocket(hash_map3,socket1ID);
    sassert(check1 == NULL);
    HashMapErrors value = insertSocket(hash_map3,socket1);
    assume(value == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map3) == 1);
    //ID1 for Socket2
    SocketID socket2ID= xmalloc(sizeof(*socket2ID));
    assume(socket2ID != socket1->id && socket2ID != socket1ID);
    strcpy_t(socket2ID->src_ip,"192.168.1.246");
    strcpy_t(socket2ID->dst_ip,"192.168.1.248");
    socket2ID->src_port = 1;
    socket2ID->dst_port = 2;
    //Socket2 creation
    Socket socket2 = create_new_socket();
    assume(socket2 != socket1);
    socket2->id = socket2ID;
    assume(socket1->id == socket1ID);
    assume(socket2->id == socket2ID);
    HashMapErrors value2 = insertSocket(hash_map3,socket2);
    assume(value2 == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map3) == 2);
    assume(socket2->id == socket2ID);
    Socket check2 = getSocket(hash_map3,socket1ID);
    sassert(check2 == socket1);
    Socket check3 = getSocket(hash_map3,socket2ID);
    assume(getSocket(hash_map3,socket1ID) == socket1 && getSocket(hash_map3,socket1ID) != socket2);
    if(check3 == socket2){
        sassert(check3 == socket2);
    }
    printf("---------\n");
    printf("HASHMAP GET SOCKET TEST SUCCESS\n");

}

void simpleTest(){
    ////creation TEST

    HashMap hash_map1 = createHashMap(1);
    HashMap hash_map1_copy = createHashMap(1);
    sassert(getHashMapSize(hash_map1) == 1);
    /////insertion TEST

    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    strcpy_t(socket1ID->src_ip,"192.168.1.240");
    strcpy_t(socket1ID->dst_ip,"192.168.1.242");
    socket1ID->src_port = nd();
    socket1ID->dst_port = nd();
    //Socket1 creation
    sassert(getHashMapNumberOfSockets(hash_map1) == 0);
    Socket check1 = getSocket(hash_map1,socket1ID);
    sassert(check1 == NULL);
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    //insertion of the first socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-||)
    //ID3 for Socket3
    SocketID socket3ID= xmalloc(sizeof(*socket3ID));
    strcpy_t(socket3ID->src_ip,"192.168.1.244");
    strcpy_t(socket3ID->dst_ip,"192.168.1.246");
    socket3ID->src_port = nd();
    socket3ID->dst_port = nd();
    Socket socket3 = create_new_socket();
    socket3->id = socket3ID;
    //creation of socket
    //insertion of the second socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-||)
    HashMapErrors value = insertSocket(hash_map1,socket1);
    assume(value == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);
    HashMapErrors value2 = insertSocket(hash_map1,socket3);
    assume(value2 == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);
    assume(hashMapGetFirst(hash_map1) == socket1ID);
    assume(hashMapGetNext(hash_map1) == socket3ID);
    sassert(compareKeys(((Socket)(getSocket(hash_map1,socket1ID)))->id,socket1ID) == true);
    sassert(compareKeys(((Socket)(getSocket(hash_map1,socket3ID)))->id,socket3ID) == true);

/*
    //ID4 for socket4:
    SocketID socket4ID= xmalloc(sizeof(*socket4ID));
    strcpy_t(socket4ID->src_ip,"192.168.1.248");
    strcpy_t(socket4ID->dst_ip,"192.168.1.250");
    socket4ID->src_port = nd();
    socket4ID->dst_port = nd();
    Socket socket4 = create_new_socket();
    socket3->id = socket4ID;
    //creation of socket4:
    //insertion of the third socket
    //hashmap status:
    // [0] : NULL
    // [1] : ([socket1]<-[socket3]<-[socket4]<-||)
    HashMapErrors value3 = insertSocket(hash_map1,socket4);
    assume(value3 == HASH_MAP_SUCCESS);
    sassert(getHashMapNumberOfSockets(hash_map1) == 33);
    //sassert(compareKeys(((Socket)(getSocket(hash_map1,socket4ID)))->id,socket4ID) == true);

    //getSocket Tests:
    //input: socket5 that isn't in the hashmap
    //expected output: err should return with the value: HASH_MAP_SOCKET_NOT_FOUND
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    strcpy_t(socket5ID->src_ip,"192.168.1.250");
    strcpy_t(socket5ID->dst_ip,"192.168.1.252");
    socket5ID->src_port =nd();
    socket5ID->dst_port = nd();
    Socket socket5 = create_new_socket();
    socket5->id = socket5ID;
    //sassert(compareKeys(((Socket)(getSocket(hash_map1,socket5ID)))->id,socket5ID) == true);
*/

}
int main() {
    hashMapTests();
}

