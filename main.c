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

    for(int i =0; i<15; i++){
        dst[i] = nd();
        assume(dst[i] >=0 && dst[i] <=9);
        dst[i] = dst[i] + '0';

    }
    dst[15] = '\0';

}

/*
 * this verifies everything. (not get)
 */
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

    //ID5 for socket5:
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
    bool value = insertSocket(hash_map1,socket1);
    assume(value == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);

    //insertion of the second socket
    bool value2 = insertSocket(hash_map1,socket3);
    assume(value2 == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);

    //insertion of the third socket
    bool value3 = insertSocket(hash_map1,socket4);
    assume(value3 == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 3);

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

    if(hash_map1->ghost_v == socket1){
        assume(hash_map1->ghost_has_v = 0);
    }
    if(hash_map1->ghost_v == socket3){
        assume(hash_map1->ghost_has_v = 0);
    }
    if(hash_map1->ghost_v == socket4){
        assume(hash_map1->ghost_has_v = 0);
    }
    //removing socket1
    b = hashmapRemove(hash_map1,socket1ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 2);

    if(hash_map1->ghost_v == socket1)
        sassert(getSocket(hash_map1,socket1ID) == NULL);

    //removing socket3
    b = hashmapRemove(hash_map1,socket3ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 1);

    //removing socket4
    b = hashmapRemove(hash_map1,socket4ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map1) == 0);

}
/*
 * this verifies: insertion + removing + checking after removal
 */
void hashMapTests_Part2() {
    ////creation TEST
    HashMap hash_map2 = createHashMap();
    sassert(hash_map2 != NULL);
    sassert(getHashMapSize(hash_map2) == 5);
    sassert(getHashMapNumberOfSockets(hash_map2) == 0);

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

    //ID5 for socket5:
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    assume(socket5ID != socket1ID);
    strcpy_t(socket5ID->src_ip,"192.168.1.252");
    strcpy_t(socket5ID->dst_ip,"192.168.1.254");
    socket5ID->src_port = nd();
    socket5ID->dst_port = nd();

    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    sassert(compareKeys(socket1->id,socket1ID) ==true);
    assume(socket1->id == socket1ID);

    if(hash_map2->ghost_v == socket1){
        assume(hash_map2->ghost_has_v == 1);
    }

    //insertion of the first socket
    bool value = insertSocket(hash_map2,socket1);
    assume(value == true);
    sassert(getHashMapNumberOfSockets(hash_map2) == 1);

    bool b = hasKey(hash_map2,socket5ID);
    if(hash_map2->ghost_v->id == socket5ID)
        sassert(!b);

    b = hasKey(hash_map2,socket1ID);
    if(hash_map2->ghost_v == socket1)
        sassert(b);
    sassert(hash_map2->table[0] == socket1);
    assume(socket5ID != socket1->id);

    //removing socket1
    b = hashmapRemove(hash_map2,socket1ID);
    sassert(b == true);
    sassert(getHashMapNumberOfSockets(hash_map2) == 0);

    b = hasKey(hash_map2,socket1ID);
    if(hash_map2->ghost_v == socket1)
        sassert(!b);

    assume(socket5ID != socket1ID);

    //Trying to remove an element that isnt in the hasmap - should not work
    b = hashmapRemove(hash_map2,socket5ID);
    sassert(b == false);
    sassert(getHashMapNumberOfSockets(hash_map2) == 0);
}
/*
 * this verifies: insertion + get
 */
void hashMapTests_Part3() {
    ////creation TEST
    HashMap hash_map2 = createHashMap();
    sassert(hash_map2 != NULL);
    sassert(getHashMapSize(hash_map2) == 5);
    sassert(getHashMapNumberOfSockets(hash_map2) == 0);

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

    //ID5 for socket5:
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    assume(socket5ID != socket1ID);
    strcpy_t(socket5ID->src_ip,"192.168.1.252");
    strcpy_t(socket5ID->dst_ip,"192.168.1.254");
    socket5ID->src_port = nd();
    socket5ID->dst_port = nd();

    //Socket1 creation
    Socket socket1 = create_new_socket();
    socket1->id = socket1ID;
    sassert(compareKeys(socket1->id,socket1ID) ==true);
    assume(socket1->id == socket1ID);

    if(hash_map2->ghost_v == socket1){
        assume(hash_map2->ghost_has_v == 1);
    }

    //insertion of the first socket
    bool value = insertSocket(hash_map2,socket1);
    assume(value == true);
    sassert(getHashMapNumberOfSockets(hash_map2) == 1);

    bool b = hasKey(hash_map2,socket5ID);
    if(hash_map2->ghost_v->id == socket5ID)
        sassert(!b);

    b = hasKey(hash_map2,socket1ID);
    if(hash_map2->ghost_v == socket1)
        sassert(b);
    sassert(hash_map2->table[0] == socket1);
    assume(socket5ID != socket1->id);

    if(hash_map2->ghost_v == socket1)
        sassert(getSocket(hash_map2,socket1ID) == socket1);
    if(hash_map2->ghost_v->id == socket5ID)
        sassert(getSocket(hash_map2,socket1ID) == NULL);
}
void hashMapTests_Part4() {
    ////creation TEST
    HashMap hash_map4 = createHashMap();
    sassert(hash_map4 != NULL);
    sassert(getHashMapSize(hash_map4) == 5);
    sassert(getHashMapNumberOfSockets(hash_map4) == 0);

    /////insertion TEST
    //ID1 for Socket1
    SocketID socket1ID= xmalloc(sizeof(*socket1ID));
    nd_ip(socket1ID->src_ip);
    nd_ip(socket1ID->dst_ip);
    assume(socket1ID->src_ip != 0 && socket1ID->dst_ip != 0);
    socket1ID->src_port = nd();
    socket1ID->dst_port = nd();

    //ID3 for Socket3
    SocketID socket3ID= xmalloc(sizeof(*socket3ID));
    nd_ip(socket3ID->src_ip);
    nd_ip(socket3ID->dst_ip);
    assume(socket3ID->src_ip != 0 && socket3ID->dst_ip != 0);
    assume(!(strcmp_t(socket3ID->src_ip,socket1ID->src_ip) == 0 && strcmp_t(socket3ID->dst_ip,socket1ID->dst_ip) == 0));
    socket3ID->src_port = nd();
    socket3ID->dst_port = nd();

    //ID4 for socket4:
    SocketID socket4ID= xmalloc(sizeof(*socket4ID));
    nd_ip(socket4ID->src_ip);
    nd_ip(socket4ID->dst_ip);
    assume(socket4ID->src_ip != 0 && socket4ID->dst_ip != 0);
    assume(!(strcmp_t(socket4ID->src_ip,socket1ID->src_ip) == 0 && strcmp_t(socket4ID->dst_ip,socket1ID->dst_ip) == 0));
    assume(!(strcmp_t(socket4ID->src_ip,socket3ID->src_ip) == 0 && strcmp_t(socket4ID->dst_ip,socket3ID->dst_ip) == 0));
    socket4ID->src_port = nd();
    socket4ID->dst_port = nd();

    //ID5 for socket5:
    SocketID socket5ID= xmalloc(sizeof(*socket5ID));
    nd_ip(socket5ID->src_ip);
    nd_ip(socket5ID->dst_ip);
    assume(socket5ID->src_ip != 0 && socket5ID->dst_ip != 0);
    assume(!(strcmp_t(socket5ID->src_ip,socket1ID->src_ip) == 0 && strcmp_t(socket5ID->dst_ip,socket1ID->dst_ip) == 0));
    assume(!(strcmp_t(socket5ID->src_ip,socket3ID->src_ip) == 0 && strcmp_t(socket5ID->dst_ip,socket3ID->dst_ip) == 0));
    assume(!(strcmp_t(socket5ID->src_ip,socket4ID->src_ip) == 0 && strcmp_t(socket5ID->dst_ip,socket4ID->dst_ip) == 0));
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


    if(hash_map4->ghost_v == socket1){
        assume(hash_map4->ghost_has_v == 1);
    }
    if(hash_map4->ghost_v == socket3){
        assume(hash_map4->ghost_has_v == 1);
    }

    if(hash_map4->ghost_v == socket4){
        assume(hash_map4->ghost_has_v == 1);
    }

    assume(socket3ID != socket1ID);

    assume(socket4ID != socket1ID);
    assume(socket4ID != socket3ID);

    assume(socket5ID != socket1ID && compareKeys(socket5ID,socket1ID) == false);
    assume(socket5ID != socket3ID);
    assume(socket5ID != socket4ID);


    assume(socket1->id == socket1ID);
    assume(socket3->id == socket3ID);
    assume(socket4->id == socket4ID);


    //insertion of the first socket
    bool value = insertSocket(hash_map4,socket1);
    assume(value == true);
    sassert(getHashMapNumberOfSockets(hash_map4) == 1);
    sassert(hash_map4->table[0] == socket1);

    //insertion of the second socket
    bool value2 = insertSocket(hash_map4,socket3);
    assume(value2 == true);
    sassert(getHashMapNumberOfSockets(hash_map4) == 2);
    sassert(hash_map4->table[1] == socket3);


    //insertion of the third socket

    bool value3 = insertSocket(hash_map4,socket4);
    assume(value3 == true);
    sassert(getHashMapNumberOfSockets(hash_map4) == 3);
    sassert(hash_map4->table[2] == socket4);



    assume(socket3ID != socket1ID);

    assume(socket4ID != socket1ID);
    assume(socket4ID != socket3ID);

    assume(socket5ID != socket1ID && compareKeys(socket5ID,socket1ID) == false);
    assume(socket5ID != socket3ID);
    assume(socket5ID != socket4ID);


    assume(socket1->id == socket1ID);
    assume(socket3->id == socket3ID);
    assume(socket4->id == socket4ID);


    bool b = hasKey(hash_map4,socket5ID);
    if(hash_map4->ghost_v->id == socket5ID)
        sassert(!b);

    b = hasKey(hash_map4,socket1ID);
    if(hash_map4->ghost_v == socket1)
        sassert(b);

    b = hasKey(hash_map4,socket3ID);
    if(hash_map4->ghost_v == socket3)
        sassert(b);


    b = hasKey(hash_map4,socket4ID);
    if(hash_map4->ghost_v == socket4)
        sassert(b);

/*
    if(hash_map4->ghost_v == socket1){
        assume(hash_map4->ghost_has_v = 0);
    }
    if(hash_map4->ghost_v == socket3){
        assume(hash_map4->ghost_has_v = 0);
    }
   /* if(hash_map4->ghost_v == socket4){
        assume(hash_map4->ghost_has_v = 0);
    }
   */
    //removing socket1
   /* b = hashmapRemove(hash_map4,socket1ID);
    assume(b == true);
    sassert(getHashMapNumberOfSockets(hash_map4) == 1);

    b = hasKey(hash_map4,socket1ID);
    if(hash_map4->ghost_v == socket1)
        sassert(!b);
        */
}

int main() {
    //hashMapTests();
    //hashMapTests_Part2();
    //hashMapTests_Part3();
    hashMapTests_Part4();

}

