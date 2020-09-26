//
// Created by tomer on 29-Jul-20.
//
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "seahorn/seahorn.h"


static void *xmalloc(size_t sz){
    void *p;
    p=malloc(sz);
 //   assume(p>0);
 //   sassert(p=-1);
    return p;
}

Queue createQueue_g( size_t mem_size){
    Queue queue = xmalloc(sizeof(*queue));
    queue->sizeOfQueue = 0;
    queue->memSize = mem_size;
    return queue;
}

bool enqueue(Queue q, Element e) {
    node *new_node = (node *) malloc(sizeof(node));
    new_node->data = xmalloc(q->memSize);
    memcpy(new_node->data, e, q->memSize);
    new_node->next = NULL;
    if (!q->sizeOfQueue) {
        q->head = q->tail = new_node;
    } else {
        q->tail->next = new_node;
        q->tail = new_node;
    }
    q->sizeOfQueue++;
    return true;
}
bool isEmpty_g(Queue q){
    return !q->sizeOfQueue;
}
Element dequeue(Queue q, QueueErrors *error){
    if (q == NULL) {*error = Queue_NULL_ARGUMENT; }
    if (q->sizeOfQueue){
        node* tmp = q->head;
        Element e = xmalloc(q->memSize);
        if (e == NULL){
            *error = Queue_ALLOCATION_FAIL;
            return NULL;
        }
        memcpy(e,tmp->data,q->memSize);
        if (q->sizeOfQueue>1){
            q->head = tmp->next;
        }
        else{
            q->head =NULL;
            q->tail =NULL;
        }
        q->sizeOfQueue--;
        free(tmp->data);
        free(tmp);
        return e;
    }
    return NULL;}
int size(Queue q){
    return q->sizeOfQueue;
}
Element findByCondition(Queue q, conditionFunction cond, QueueErrors *error){
    if (q == NULL) {*error = Queue_NULL_ARGUMENT; }
    *error = Queue_SUCCESS;
    node* tmp = q->head;
    for (int i=0; i< q->sizeOfQueue; i++, tmp=tmp->next){
        if (cond(tmp->data)){
            Element e = xmalloc(q->memSize);
            if (e == NULL){
                *error = Queue_ALLOCATION_FAIL;
                return NULL;
            }
            memcpy(e,tmp->data,q->memSize);
            return e;
        }
    }
    return NULL;
}
Element removeByCondition(Queue q, conditionFunction cond, QueueErrors *error){
    if (q == NULL) { *error = Queue_NULL_ARGUMENT; }
    if(isEmpty_g(q)) {*error = Queue_EMPTY; return NULL;}
    *error = Queue_SUCCESS;
    node *tmp = q->head;
    if (cond(tmp->data)) return dequeue(q, error);
    if (q->sizeOfQueue>1) {
        node* prev = tmp;
        tmp = tmp->next;
        for (int i = 1; i < q->sizeOfQueue; i++, prev = tmp, tmp = tmp->next) {
            if (cond(tmp->data)) {
                Element e = xmalloc(q->memSize);
                if (e == NULL){
                    *error = Queue_ALLOCATION_FAIL;
                    return NULL;
                }
                memcpy(e, tmp->data, q->memSize);
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
        dequeue(q,error);
    }
}



void destroyQueue(Queue q, QueueErrors *error)
{
    if (q == NULL) return;
    clearQueue(q, error);
    free(q);
}

/*
 * int main(int argc, char**argv) {
    Queue q = createQueue_g(sizeof(int));
    int n = nd();
    assume(n>=1);
    int r = nd();
    assume(r>=0);
    assume(r<=n);
    //assume(r==2);
    int i;
    int j = 0;
    for (i = 0; i<n; i++) {
        enqueue(q, &i);
        j++;
    }
   // dequeue(q);
    int rr = j;
    int h = 0;
    for (h = 0; h<rr; h++) {
        int y = dequeue(q);
        int e = n-h;
        sassert(e=y);
        rr--;
    }

    int t = size(q);
    sassert(t == rr);
    return 0;
}
 */