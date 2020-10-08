#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef void* QueueElement;
typedef void* Parameter;
typedef bool (*conditionFunction)(QueueElement, Parameter);
typedef void (*freeElem)(QueueElement);
typedef QueueElement (*copyElem)(QueueElement);

typedef struct Element_t* Element;
typedef struct Queue_t* Queue;

Element ElementCreate();
bool ElementPut(Element element, QueueElement queueElement);
QueueElement ElementGet(Element element);

Queue QueueCreate(int capacity);
void QueueDestroy(Queue q,  freeElem free_func);
Queue QueueCopy(Queue q, copyElem copy);
bool enqueue(Queue q, QueueElement element);
QueueElement dequeue(Queue q);
bool QueueIsFull(Queue q);
bool QueueIsEmpty(Queue q);
int QueueSize(Queue q);
int QueueCapacity(Queue q);
QueueElement QueueFindByCondition(Queue q, conditionFunction cond, Parameter param);
QueueElement QueueRemoveByCondition(Queue q, conditionFunction cond, Parameter param);

QueueElement QueueGetFirst(Queue q);
QueueElement QueueGetNext(Queue q);

QueueElement QueueGetElement(Queue q, int index);

#define QUEUE_FOR_EACH(item, queue) \
    for (QueueElement item = QueueGetFirst(queue); \
        item != NULL; \
        item = QueueGetNext(queue))


#endif // QUEUE_H
