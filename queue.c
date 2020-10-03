//
// Created by Ido Yam on 01/10/2020.
//

#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "seahorn/seahorn.h"
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

Queue createQueue_g(size_t mem_size,compareElem compareElem1, freeElem freeElem1, copyElem copyElem1){
    Queue queue = xmalloc(sizeof(*queue));
    //assume(queue!=NULL);
    queue->head = NULL;
    queue->tail = NULL;
    queue->sizeOfQueue = 0;
    queue->memSize = mem_size;
    queue->compare_func=compareElem1;
    queue->free_func = freeElem1;
    queue->copy_elem = copyElem1;
    queue->nd_inserted = NULL;
    queue->picked_someone = false;
    return queue;
}

bool enqueue(Queue q, Element e) {
    assume(e != NULL && q!=NULL);
    Node new_Node = xmalloc(sizeof(*new_Node));
    //assume(q->copy_elem != NULL  /*q->compare_func != NULL*/);
    //Element copied = q->copy_elem(e);
    //assume(q->compare_func(copied,e) == true);
    new_Node->value = e;
    //assume(new_Node->value != NULL);
    //assume(q->compare_func(new_Node->value,e) == true);
    new_Node->next = NULL;
    if (q->sizeOfQueue == 0) {
        q->head =  new_Node;
        q->tail = new_Node;
    } else {
        q->tail->next = new_Node;
        q->tail = new_Node;
    }
    if (q->picked_someone == false) {
        int pick_this_item = nd();
        if (pick_this_item == 1) {
            //q->nd_inserted = xmalloc(q->memSize);
            q->nd_inserted = e;
            q->picked_someone = true;
        }
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
    assume(q != NULL && error != NULL);
    if(q->sizeOfQueue>0){
        Node tmp = q->head;
        assume(tmp != NULL);
        if (q->sizeOfQueue>1){
            q->head = tmp->next;
        }
        else{
            q->head =NULL;
            q->tail =NULL;
        }
        q->sizeOfQueue--;
        /*assume(q->free_func != NULL);
        q->free_func(tmp->value);
        free(tmp);*/
        *error = Queue_SUCCESS;
        return tmp->value;
    } else {
        *error = Queue_EMPTY;
        return NULL;
    }
}
int size(Queue q){
    return q->sizeOfQueue;
}
Element findByCondition(Queue q, conditionFunction cond, QueueErrors *error) {
    if (q == NULL) {
        *error = Queue_NULL_ARGUMENT;
        return NULL;
    }
    assume(q != NULL && error != NULL);
    *error = Queue_SUCCESS;
    Node tmp = q->head;
    for (int i=0; i< q->sizeOfQueue; i++, tmp=tmp->next){
        assume(tmp!=NULL);
        if (cond(tmp->value)){
            Element e = xmalloc(q->memSize);
            if (e == NULL){
                *error = Queue_ALLOCATION_FAIL;
                return NULL;
            }
            assume(e!=NULL);
            myMemCpy(e,tmp->value,q->memSize);
            return e;
        }
    }
    return NULL;
}
Element removeByCondition(Queue q, conditionFunction cond, QueueErrors *error){
    if (q == NULL) { *error = Queue_NULL_ARGUMENT; }
    assume(q != NULL && error!=NULL);
    if(isEmpty_g(q)) {*error = Queue_EMPTY; return NULL;}
    *error = Queue_SUCCESS;
    Node tmp = q->head;
    assume(tmp!=NULL);
    if (cond(tmp->value)) return dequeue(q, error);
    if (q->sizeOfQueue>1) {
        Node prev = tmp;
        tmp = tmp->next;
        assume(tmp!=NULL);
        for (int i = 1; i < q->sizeOfQueue; i++, prev = tmp, tmp = tmp->next) {
            if (cond(tmp->value)) {
                prev->next = tmp->next;
                q->sizeOfQueue--;
                return tmp;
            }
        }
    }
    return NULL;
}
void clearQueue(Queue q, QueueErrors *error)
{
    if (q == NULL)
    {
        *error = Queue_NULL_ARGUMENT;
    }
    bool isDestroyedElement = false;
    *error = Queue_SUCCESS;
    while (!isEmpty_g(q))
    {
        Node n = dequeue(q,error);
        if(n->value == q->nd_inserted){
            isDestroyedElement = true;
        }
        q->free_func(n->value);
        free(n->next);
        free(n);
    }
    assert(isDestroyedElement);
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

