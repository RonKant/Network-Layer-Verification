#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define DEFAULT_QUEUE_CAPACITY 20


typedef char QueueElement;

typedef struct Queue_t* Queue;

Queue QueueCreate();
int QueueSize(Queue q);

void QueueDestroy(Queue q);
bool enqueue(Queue q, char element);
QueueElement dequeue(Queue q);
bool QueueIsFull(Queue q);
bool QueueIsEmpty(Queue q);
int QueueCapacity(Queue q);

QueueElement QueueGetFirst(Queue q);
QueueElement QueueGetNext(Queue q);

QueueElement QueueGetElement(Queue q, int index);

#define QUEUE_FOR_EACH(item, queue) \
    for (QueueElement item = QueueGetFirst(queue); \
        item != NULL; \
        item = QueueGetNext(queue))


#endif // QUEUE_H
