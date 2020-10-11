#include <stdio.h>

#include "array_queue.h"
#include "Functions.h"
#include "seahorn/seahorn.h"

extern int nd();


struct Queue_t {
    int head;
    int tail;
    int size;
    int capacity;
    QueueElement array[DEFAULT_QUEUE_CAPACITY];
    int iterator;
    int index_choosen;
    bool was_dequeued;
    QueueElement choosen;
    int nums_of_dequeue;
    QueueElement first_nd;
    QueueElement second_nd;
    bool first_is_out   ;
};

Queue QueueCreate() {
    Queue queue = (Queue)xmalloc(sizeof(*queue));
    if (NULL == queue) return NULL;

    queue->capacity = DEFAULT_QUEUE_CAPACITY;
    queue->head = 0;
    queue->tail = DEFAULT_QUEUE_CAPACITY - 1;
    queue->size = 0;
    queue->iterator = 0;
    queue->index_choosen = -1;
    queue->was_dequeued = false;
    queue->choosen = 'a';
    queue->nums_of_dequeue = 0;
    queue->first_nd = 'z';
    queue->second_nd = 'z';
    queue->first_is_out = false;
    for (int i = 0; i < DEFAULT_QUEUE_CAPACITY; i = i+1 ) (queue->array)[i] = NULL;
    return queue;
}

void QueueDestroy(Queue q) {
    assume(q!=NULL);
    //if (NULL == q) return;
    //if (NULL == q->array) return;

    QUEUE_FOR_EACH(item, q) {
        free(item);
    }

    free(q->array);
    free(q);
}

bool enqueue(Queue q, char element) {
    assume(q!=NULL);
    assume(element!=NULL);

    //if(q == NULL || element == NULL)
      //  return false;
    if (QueueIsFull(q)) return false;

    q->tail = (q->tail + 1);
    if(q->tail >= QueueCapacity(q))
        q->tail -= QueueCapacity(q);
    (q->array)[q->tail] = element;
    q->size = q->size + 1;
    int f = nd();
    assume(f == 0 || f == 1);
    //int f = 1;
    if (q->index_choosen == -1){
        if(f){
            q->choosen = element;
            q->index_choosen = q->size-1;
        }
    }
    if (f) {
        if (q->first_nd == 'z' || q->second_nd == 'z') {
            if (q->first_nd == 'z') {
                q->first_nd = element;
            } else if (q->second_nd == 'z') {
                q->second_nd = element;
            }
        }
    }

    return true;
}

QueueElement dequeue(Queue q) {
    assume(q!=NULL);
    //if (QueueIsEmpty(q)) return NULL;

    QueueElement element = (q->array)[q->head];
    (q->array)[q->head] = NULL;
    q->head = (q->head + 1);
    if(q->head >= q->capacity)
        q->head -= q->capacity;
    q->size--;
    if (element == q->choosen){
        q->was_dequeued = true;
    }
    if (q->index_choosen!=-1) {
        q->nums_of_dequeue = q->nums_of_dequeue + 1;
    }
    if (q->nums_of_dequeue>q->index_choosen && q->index_choosen>-1){
        sassert(q->was_dequeued);
    }
    if (element == q->first_nd){
        q->first_is_out = true;
    }
    if (element == q->second_nd && q->second_nd!='z'){
        sassert(q->first_is_out);
    }
    return element;
}

bool QueueIsFull(Queue q) {
    assume(q!=NULL);
    //if(q == NULL)
      //  return false;
    if(QueueSize(q) == QueueCapacity(q))
        return true;
    return false;
}

bool QueueIsEmpty(Queue q) {
    assume(q!=NULL);
    //if(q == NULL)
      //  return false;
    if(QueueSize(q) == 0)
        return true;
    return false;
}

int QueueSize(Queue q) {
  assume(q!=NULL);
  //  if(q == NULL)
    //    return -1;
    return q->size;
    //return 1;
}

int QueueCapacity(Queue q) {
    assume(q!=NULL);
    //if(q == NULL)
      //  return -1;
    return q->capacity;
}

QueueElement QueueGetFirst(Queue q) {
    assume(q!=NULL);
    if (NULL == q || NULL == q->array) return NULL;
    if (QueueIsEmpty(q)) return NULL;

    q->iterator = 0;
    return (q->array)[q->head];
}

QueueElement QueueGetNext(Queue q) {
    assume(q!=NULL);
    //if (NULL == q ) return NULL;
    if (q->iterator+1 >= q->size) return NULL;
    q->iterator = q->iterator +1;
    int index = (q->head + q->iterator);
    if(index >= q->capacity)
        index-=q->capacity;
    return (q->array)[index];

}
QueueElement QueueGetElement(Queue q, int index){
    assume(q!=NULL);
    return q->array[index];
}
