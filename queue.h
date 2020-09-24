//
// Created by tomer on 29-Jul-20.
//
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef CODE_QUEUE_H
#define CODE_QUEUE_H



typedef void* Element;
typedef bool (*conditionFunction)(Element);


typedef struct Node
{
    Element data;
    struct Node *next;
}node;

typedef struct QueueList
{
    int sizeOfQueue;
    size_t memSize;
    node *head;
    node *tail;
}*Queue;

typedef enum {
    Queue_SUCCESS,
    Queue_ALLOCATION_FAIL,
    Queue_NULL_ARGUMENT,
    Queue_ERROR,
    Queue_EMPTY
} QueueErrors;

Queue createQueue_g(size_t mem_size);
void destroyQueue(Queue q, QueueErrors *error);
bool enqueue(Queue q, Element e);
bool isEmpty_g(Queue q);
Element dequeue(Queue q, QueueErrors *error);
int size(Queue q);

Element findByCondition(Queue q, conditionFunction cond, QueueErrors *error);
Element removeByCondition(Queue q, conditionFunction cond, QueueErrors *error);


#endif //CODE_QUEUE_H
