#include <stdio.h>
//#include "Hashmap.h"
//#include "util_types.h"
//#include "network.h"
//#include "ip.h"
//#include <sassert.h>
#include <stdlib.h>
#include <string.h>
#include<stdint.h>
#include<stddef.h>
//#include "socket_utils.h"
#include "array_queue.h"
#include "ip.h"
//#include "Functions.h"
#include<seahorn/seahorn.h>

extern int nd();

void iteratorTest(){
    Queue queue = QueueCreate();
    sassert(QueueCapacity(queue) == DEFAULT_QUEUE_CAPACITY);
    sassert(QueueSize(queue) == 0);
    //sassert(QueueGetNext(queue) == NULL);
    char a = '0';
    enqueue(queue,a);
    sassert(QueueSize(queue)==1);
    sassert(QueueGetFirst(queue)== '0');
    sassert(QueueGetNext(queue) == NULL);
    queue = QueueCreate();
    int n = nd();
    int d = nd();
    assume( n>d && d>=0 && n<3 );
    int nums[20];
    for (int i = 0; i<n;i++){
        enqueue(queue,'0'+i);
    }
   for (int i = 0; i<d ; i++) {

            int c = i;
            assume(enqueue(queue, &c));
    }

    assume(enqueue(queue, &d));

    for (int i = d+1; i<n ; i++) {

        int c = i;
        assume(enqueue(queue, &c));
    }

    char curr = QueueGetFirst(queue);
    for (int i = 0; i <d; i++){
        curr = QueueGetNext(queue);
    }
    sassert(curr == '0'+d);



}
void queueTest(){
    Queue queue = QueueCreate();
    int a = 0;

    sassert(0== QueueSize(queue));
    int next_size = 3;
    for(int i =0; i<next_size;i++){
        enqueue(queue,'0'+i);
    }
    for(int i =0; i<next_size;i++){
        dequeue(queue);
    }
    sassert(0== QueueSize(queue));
    int nums_of_enqueue = nd();
    int nums_of_dequeue = nd();
    assume(nums_of_enqueue>=nums_of_dequeue && nums_of_dequeue >= 0 && nums_of_enqueue<=DEFAULT_QUEUE_CAPACITY);
    for(int i = 0; i<nums_of_enqueue; i++){
        enqueue(queue,'a');
    }
    for(int i = 0; i<nums_of_dequeue; i++){
        dequeue(queue);
    }
    sassert(QueueSize(queue) == nums_of_enqueue-nums_of_dequeue);
}
void ipTest(){
    int n = str_to_int("054",1);
    n = str_to_int("054",2);
    n = nd();
    assume(n>=0 && n<10000);
    char stmp[10];
    int size = 5;
    int_to_str(stmp,size,n);
    sassert(n==str_to_int(stmp,size));
}

int main() {
    iteratorTest();
    queueTest();
    ipTest();
}

