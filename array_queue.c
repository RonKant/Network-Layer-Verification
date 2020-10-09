#include <stdio.h>

#include "array_queue.h"
#include "Functions.h"
//#include "seahorn/seahorn.h"

struct Queue_t {
    int head;
    int tail;
    int size;
    int capacity;
    QueueElement* array; //CHECK
    int iterator;
};

Queue QueueCreate(int capacity) {
    if (capacity <= 0) return NULL;
    Queue queue = (Queue)xmalloc(sizeof(*queue));
    if (NULL == queue) return NULL;

    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = capacity - 1;
    queue->size = 0;
    queue->array = (QueueElement*)xmalloc(sizeof(QueueElement) * capacity);
    if (NULL == queue->array) {
        free(queue);
        return NULL;
    }

    for (int i = 0; i < capacity; i = i+1 ) (queue->array)[i] = NULL;
    return queue;
}

void QueueDestroy(Queue q, freeElem free_func) {
    if (NULL == q) return;
    if (NULL == q->array) return;

    if (NULL != free_func) {
        QUEUE_FOR_EACH(item, q) {
            free_func(item);
        }
    }

    free(q->array);
    free(q);
}

Queue QueueCopy(Queue q, copyElem copy) {
    Queue copy_q = QueueCreate(q->capacity);
    QUEUE_FOR_EACH(item, q) {
        QueueElement item_copy = copy(item);
        enqueue(copy_q, item_copy);
    }
    return copy_q;
}

bool enqueue(Queue q, QueueElement element) {
    if(q == NULL || element == NULL)
        return false;
    if (QueueIsFull(q)) return false;

    q->tail = (q->tail + 1);
    if(q->tail >= QueueCapacity(q))
        q->tail -= QueueCapacity(q);
    (q->array)[q->tail] = element;
    q->size = q->size + 1;

    return true;
}

QueueElement dequeue(Queue q) {
    if (QueueIsEmpty(q)) return NULL;

    QueueElement element = (q->array)[q->head];
    (q->array)[q->head] = NULL;
    q->head = (q->head + 1);
    if(q->head >= q->capacity)
        q->head -= q->capacity;
    q->size--;
    return element;
}

bool QueueIsFull(Queue q) {
    if(q == NULL)
        return false;
    if(QueueSize(q) == QueueCapacity(q))
        return true;
    return false;
}

bool QueueIsEmpty(Queue q) {
    if(q == NULL)
        return false;
    if(QueueSize(q) == 0)
        return true;
    return false;
}

int QueueSize(Queue q) {
    if(q == NULL)
        return -1;
    return q->size;
}

int QueueCapacity(Queue q) {
    if(q == NULL)
        return -1;
    return q->capacity;
}

QueueElement QueueFindByCondition(Queue q, conditionFunction cond, Parameter param) {
    QUEUE_FOR_EACH(item, q) {
        if (cond(item, param)) return item;
    }
    return NULL;
}

QueueElement QueueRemoveByCondition(Queue q, conditionFunction cond, Parameter param) {
    Queue temp_q = QueueCreate(q->capacity);
    QueueElement removed = NULL;
    QUEUE_FOR_EACH(item, q) {
        if (NULL != removed || false == cond(item, param)) {
            enqueue(temp_q, item);
        } else if (cond(item, param)) {
            removed = item;
        }
    }
    for (; q->size > 0;) dequeue(q);
    QUEUE_FOR_EACH(item, temp_q) enqueue(q, item);
    QueueDestroy(temp_q, NULL);
    return removed;
}

QueueElement QueueGetFirst(Queue q) {
    if (NULL == q || NULL == q->array) return NULL;
    if (QueueIsEmpty(q)) return NULL;

    q->iterator = 0;
    return (q->array)[q->head];
}

QueueElement QueueGetNext(Queue q) {
    if (NULL == q || NULL == q->array) return NULL;

    q->iterator = q->iterator +1;
    if (q->iterator == q->size) return NULL;
    int index = (q->head + q->iterator);
    if(index >= q->capacity)
        index-=q->capacity;
    return (q->array)[index];

}
QueueElement QueueGetElement(Queue q, int index) {
    if (q == NULL || q->array == NULL)
        return NULL;
    return q->array[index];
}