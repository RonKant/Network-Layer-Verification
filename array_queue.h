#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef void* QueueElement;
typedef void* Parameter;
typedef bool (*conditionFunction)(QueueElement, Parameter);
typedef void (*freeElem)(QueueElement);

typedef struct Queue_t* Queue;

Queue QueueCreate(int capacity);
void QueueDestroy(Queue q, freeElem free_func);
Queue QueueCopy(Queue q);
bool enqueue(Queue q, QueueElement element);
QueueElement dequeue(Queue q);
bool QueueIsFull(Queue q);
bool QueueIsEmpty(Queue q);
int QueueSize(Queue q);
int QueueCapacity(Queue q);
QueueElement QueueFindByCondition(Queue q, conditionFunction cond);
QueueElement QueueRemoveByCondition(Queue q, conditionFunction cond);

QueueElement QueueGetFirst(Queue q);
QueueElement QueueGetNext(Queue q);

#define QUEUE_FOR_EACH(item, queue) \
    for (QueueElement item = QueueGetFirst(queue); \
        item != NULL; \
        item = QueueGetNext(queue))


#endinf QUEUE_H