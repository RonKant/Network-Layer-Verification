#include <stdio.h>

#include "array_queue.h"
#include "Functions.h"
// #include "seahorn/seahorn.h"

// extern int nd();

struct Queue_t {
    int head;
    int tail;
    int size;
    QueueElement array[DEFAULT_QUEUE_CAPACITY];
    int iterator;
    int index_choosen;
    // bool was_dequeued;
    // QueueElement choosen;
    // int nums_of_dequeue;
};

Queue QueueCreate() {
    Queue queue = (Queue)xmalloc(sizeof(*queue));
    if (NULL == queue) return NULL;

    queue->head = 0;
    queue->tail = DEFAULT_QUEUE_CAPACITY - 1;
    queue->size = 0;
    queue->iterator = 0;
    queue->index_choosen = -1;
    // queue->was_dequeued = false;
    // queue->choosen = 'a';
    // queue->nums_of_dequeue = 0;
    for (int i = 0; i < DEFAULT_QUEUE_CAPACITY; i = i+1 ) (queue->array)[i] = QUEUE_EMPTY_ELEMENT;
    return queue;
}

void QueueDestroy(Queue q) {
    free(q);
}

bool enqueue(Queue q, QueueElement element) {
    // assume(q!=NULL);
    // assume(element!=NULL);
    if(q == NULL)
       return false;

    if (QueueIsFull(q)) return false;

    q->tail = (q->tail + 1);
    if(q->tail >= QueueCapacity(q))
        q->tail -= QueueCapacity(q);
    (q->array)[q->tail] = element;
    q->size = q->size + 1;
    // if (q->index_choosen == -1){
    //     int f = nd();
    //     assume(f == 0 || f == 1);
    //     if(f){
    //         q->choosen = element;
    //         q->index_choosen = q->size-1;
    //     }
    // }

    return true;
}

QueueElement dequeue(Queue q) {
    // assume(q!=NULL);
    if (QueueIsEmpty(q)) return QUEUE_EMPTY_ELEMENT;

    QueueElement element = (q->array)[q->head];
    (q->array)[q->head] = QUEUE_EMPTY_ELEMENT;
    q->head = (q->head + 1);
    if(q->head >= QueueCapacity(q))
        q->head -= QueueCapacity(q);
    q->size--;
    // if (element == q->choosen){
    //     q->was_dequeued = true;
    // }
    // if (q->index_choosen!=-1) {
    //     q->nums_of_dequeue = q->nums_of_dequeue + 1;
    // }
    // if (q->nums_of_dequeue>q->index_choosen){
    //     //sassert(q->was_dequeued);
    // }
    return element;
}

bool QueueIsFull(Queue q) {
    // assume(q!=NULL);
    if(q == NULL)
       return false;
    if(QueueSize(q) == QueueCapacity(q))
        return true;
    return false;
}

bool QueueIsEmpty(Queue q) {
    // assume(q!=NULL);
    if(q == NULL)
       return false;
    if(QueueSize(q) == 0)
        return true;
    return false;
}

int QueueSize(Queue q) {
//   assume(q!=NULL);
   if(q == NULL)
       return -1;
    return q->size;
}

int QueueCapacity(Queue q) {
    // assume(q!=NULL);
    if(q == NULL)
       return -1;
    return DEFAULT_QUEUE_CAPACITY;
}

QueueElement QueueGetFirst(Queue q) {
    // assume(q!=NULL);
    if (NULL == q ) return QUEUE_EMPTY_ELEMENT;
    if (QueueIsEmpty(q)) return QUEUE_EMPTY_ELEMENT;

    q->iterator = 0;
    return (q->array)[q->head];
}

Queue QueueCopy(Queue q) {
    Queue copied = QueueCreate();
    if (NULL == copied) return NULL;
    QUEUE_FOR_EACH(item, q) {
        enqueue(copied, item);
    }
    return copied;
}

QueueElement QueueGetNext(Queue q) {
    // assume(q!=NULL);
    if (NULL == q ) return QUEUE_EMPTY_ELEMENT;
    if (q->iterator+1 >= q->size) return QUEUE_EMPTY_ELEMENT;
    q->iterator = q->iterator +1;
    int index = (q->head + q->iterator);
    if(index >= QueueCapacity(q))
        index-=QueueCapacity(q);
    return (q->array)[index];

}