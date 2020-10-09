#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define DEFAULT_QUEUE_CAPACITY 8192

#define QUEUE_EMPTY_ELEMENT '\0'

typedef char QueueElement;

typedef struct Queue_t* Queue;

Queue QueueCreate();
void QueueDestroy(Queue q);
bool enqueue(Queue q, QueueElement element);
QueueElement dequeue(Queue q);
bool QueueIsFull(Queue q);
bool QueueIsEmpty(Queue q);
int QueueSize(Queue q);
int QueueCapacity(Queue q);

Queue QueueCopy(Queue q);

QueueElement QueueGetFirst(Queue q);
QueueElement QueueGetNext(Queue q);

QueueElement QueueGetElement(Queue q, int index);

#define QUEUE_FOR_EACH(item, queue) \
    for (QueueElement item = QueueGetFirst(queue); \
        item != QUEUE_EMPTY_ELEMENT; \
        item = QueueGetNext(queue))


#endif // QUEUE_H