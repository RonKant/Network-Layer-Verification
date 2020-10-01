//
// Created by tomer on 29-Jul-20.
//
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "Functions.h"
#ifndef CODE_QUEUE_H
#define CODE_QUEUE_H



typedef void* Element;
typedef bool (*conditionFunction)(Element);
typedef bool (*compareElem)(Element,Element);
typedef void (*freeElem)(Element);
typedef Element (*copyElem)(Element);

typedef struct Node_t *Node;
Element getValue(Node node);
void* getKey(Node node);
Node getNext(Node node);
void setNext(Node node, Node next);

typedef struct QueueList
{
    int sizeOfQueue;
    size_t memSize;
    Node head;
    Node tail;
    compareElem compare_func;
    freeElem free_func;
    copyElem copy_elem;
}*Queue;

typedef enum {
    Queue_SUCCESS,
    Queue_ALLOCATION_FAIL,
    Queue_NULL_ARGUMENT,
    Queue_ERROR,
    Queue_EMPTY
} QueueErrors;

Queue createQueue_g(size_t mem_size,compareElem compareElem1, freeElem freeElem1, copyElem copyElem1);
void destroyQueue(Queue q, QueueErrors *error);
bool enqueue(Queue q, Element e);
bool isEmpty_g(Queue q);
Element dequeue(Queue q, QueueErrors *error);
int size(Queue q);
Element getHead(Queue q);
Element findByCondition(Queue q, conditionFunction cond, QueueErrors *error);
Element removeByCondition(Queue q, conditionFunction cond, QueueErrors *error);
Element removeSpecificElement(Queue q, Element e, QueueErrors *error);
Queue deepCopy(Queue q);

#endif //CODE_QUEUE_H