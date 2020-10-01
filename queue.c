//
// Created by tomer on 29-Jul-20.
//
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#include "seahorn/seahorn.h"
extern int nd(void);

struct Node_t
{
    Element value;
    Node next;
};

Element getValue(Node Node){
    if(Node == NULL)
        return NULL;
    return Node->value;
}
Node getNext(Node Node){
    if(Node == NULL)
        return NULL;
    return Node->next;
}
void setNext(Node node, Node next){
    if(node == NULL)
        return;
    node->next = next;
}

Queue createQueue_g(compareElem compareElem1, freeElem freeElem1, copyElem copyElem1){
    Queue queue = xmalloc(sizeof(*queue));
    //assume(queue!=NULL);
    queue->head = NULL;
    queue->tail = NULL;
    queue->sizeOfQueue = 0;
    queue->compare_func=compareElem1;
    queue->free_func = freeElem1;
    queue->copy_elem = copyElem1;

    queue->iterator = NULL;
    return queue;
}

bool enqueue(Queue q, Element e) {
    Node new_Node = xmalloc(sizeof(*new_Node));
    new_Node->value = q->copy_elem(e);

    new_Node->next = NULL;
    if (!q->sizeOfQueue) {
        q->head = q->tail = new_Node;
    } else {
        q->tail->next = new_Node;
        q->tail = new_Node;
    }
    q->sizeOfQueue++;
    return true;
}
bool isEmpty_g(Queue q){
    return !q->sizeOfQueue;
}
Element dequeue(Queue q, QueueErrors *error){
    if (q == NULL){
        *error = Queue_NULL_ARGUMENT;
        return NULL;
    }
    if (q->sizeOfQueue>0){
        Node tmp = q->head;
        Element e = q->copy_elem(tmp->value);
        if (e == NULL){
            *error = Queue_ALLOCATION_FAIL;
            return NULL;
        }
        if (q->sizeOfQueue>1){
            q->head = tmp->next;
        }
        else{
            q->head =NULL;
            q->tail =NULL;
        }
        q->sizeOfQueue--;
        q->free_func(tmp->value);
        free(tmp);
        *error = Queue_SUCCESS;
        return e;
    }
    return NULL;
}


int size(Queue q){
    return q->sizeOfQueue;
}
Element findByCondition(Queue q, conditionFunction cond, QueueErrors *error) {
    if (q == NULL) {
        *error = Queue_NULL_ARGUMENT;
        return NULL;
    }
    *error = Queue_SUCCESS;
    Node tmp = q->head;
    for (int i=0; i< q->sizeOfQueue; i++, tmp=tmp->next){
        if (cond(tmp->value)){
            return tmp->value;
        }
    }
    return NULL;
}
Element removeByCondition(Queue q, conditionFunction cond, QueueErrors *error){
    if (q == NULL) { *error = Queue_NULL_ARGUMENT; }
    if(isEmpty_g(q)) {*error = Queue_EMPTY; return NULL;}
    *error = Queue_SUCCESS;
    Node tmp = q->head;
    if (cond(tmp->value)) return dequeue(q, error);
    if (q->sizeOfQueue>1) {
        Node prev = tmp;
        tmp = tmp->next;
        for (int i = 1; i < q->sizeOfQueue; i++, prev = tmp, tmp = tmp->next) {
            if (cond(tmp->value)) {
                Element e = q->copy_elem(q);
                if (e == NULL){
                    *error = Queue_ALLOCATION_FAIL;
                    return NULL;
                }
                prev->next = tmp->next;
                free(tmp->next);
                free(tmp);
                q->sizeOfQueue--;
                return e;
            }
        }
    }
    return NULL;
}
void clearQueue(Queue q, QueueErrors *error)
{
    if (q == NULL)
    {
        if (error != NULL) *error = Queue_NULL_ARGUMENT;
    }
    if (error != NULL) *error = Queue_SUCCESS;

    while (!isEmpty_g(q))
    {
        q->free_func(dequeue(q,error));
    }
}
void destroyQueue(Queue q, QueueErrors *error)
{
    clearQueue(q, error);
    free(q);
}
Element getHead(Queue q){
    if(q != NULL)
        return q->head;
    return NULL;
}

Queue copyQueue(Queue q) {
    Queue new_queue = createQueue_g(q->compare_func, q->free_func, q->copy_elem);
    return new_queue;
}

Element queueGetFirst(Queue queue) {
    if (NULL == queue) return NULL;
    queue->iterator = queue->head;
    if (queue->iterator == NULL) {
        return NULL;
    }
    return queue->iterator->value;
}
Element queueGetNext(Queue queue) {
    if (NULL ==  queue) return NULL;
    if (queue->iterator->next == NULL) {
        return NULL;
    }
    queue->iterator = queue->iterator->next;
    return queue->iterator->value;
}


/*int main(int argc, char**argv) {
    Queue q = createQueue_g(sizeof(int));
    QueueErrors *error = xmalloc(sizeof(*error));
    int e=1;


    Node new_Node = (Node)xmalloc(sizeof(Node));
    //assume(new_Node!=NULL);
    new_Node->value = xmalloc(q->memSize);
    //assume(new_Node->value!=NULL);
    *((int *)new_Node->value) = 1;
    int e2=1;
    int*t =&e2;
    char *csrc = (char *)t;
    char *cdest = (char *)new_Node->value;

    // Copy contents of src[] to dest[]
    for (int i=0; i< sizeof(int); i++)
        cdest[i] = csrc[i];

    myMemCpy(new_Node->value, &e, q->memSize);
    //sassert(1==*((int*)new_Node->value));
    //sassert(NULL!=new_Node->value);
    new_Node->next = NULL;
    q->head = q->tail = new_Node;


    Node tmp = q->head;
    int* er = xmalloc(q->memSize);
    //asssume(er != NULL);
    memcpy(er,tmp->value,q->memSize);

    //sassert(1 == *er);

    //enqueue(q,&i);
    int *m=0;
    m = dequeue(q,error);
    //assume(*error==Queue_SUCCESS);
    //int m1 = *m;
    //assume(m!=NULL);
    //assume(m1==2147483646);
    //sassert(1 == m1);
    return 0;
}


int main(int argc, char**argv) {
    Queue q = createQueue_g(sizeof(int));
    QueueErrors *error = xmalloc(sizeof(*error));
    int n = nd();
    assume(n>=1);
    int r = nd();
    assume(r>=0);
    assume(r<=n);
    assume(r==1);
    int total = n-r;
    int i=1;
    enqueue(q,&i);
    int *m=0;
    m = dequeue(q,error);
    sassume(*error==Queue_SUCCESS);
    int m1 = *m;
    sassume(m!=NULL);
    sassume(m1!=NULL);
    int z = nd();
    sassert(1 != m1);
    for (i = 0; i<1; i++) {
        enqueue(q, &i);
    }
    int *y=0;
    int h = 0;
    for (h = 0; h<r; h++) {
        y = dequeue(q,error);
        int y1 = *y;
        //sassert(h==y1);
    }

    int t = size(q);
    //sassert(t == total);
    return 0;
}
*/

