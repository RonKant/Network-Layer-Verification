#include <stdio.h>

#include "array_queue.h"

struct Queue_t {
    int head, tail, size, capacity;
    QueueElement* array;
    int iterator;
    freeElem free_func;
};

Queue QueueCreate(int capacity, freeElem free_func) {
    if (capacity <= 0) return NULL;
    Queue queue = (Queue)malloc(sizeof(*queue));
    if (NULL == queue) return NULL;

    queue->capacity = capacity;
    queue->head = queue->size = 0;
    queue->tail = capacity - 1;

    queue->free_func = free_func;

    queue->array = (QueueElement*)malloc(sizeof(QueueElement) * capacity);
    if (NULL == queue->array) {
        free(queue);
        return NULL;
    }

    for (int i = 0; i < capacity; ++i) (queue->array)[i] = NULL;
    return queue;
}

void QueueDestroy(Queue q) {
    if (NULL == q) return;
    if (NULL == q->array) return;
    
    if (NULL != q->free_func) {
        QUEUE_FOR_EACH(item, q) {
            (q->free_func)(item);
        }
    }

    free(q->array);
    free(q);
}

Queue QueueCopy(Queue q, copyElem copy) {
    Queue copy_q = QueueCreate(q->capacity, q->free_func);
    QUEUE_FOR_EACH(item, q) {
        QueueElement item_copy = copy(item);
        if (false == enqueue(copy_q, item_copy)) {
            QueueDestroy(copy_q);
            return NULL;
        }
    }
    return copy_q;
}

bool enqueue(Queue q, QueueElement element) {
    if (QueueIsFull(q)) return false;

    q->tail = (q->tail + 1) % (q->capacity);
    (q->array)[q->tail] = element;
    q->size++;

    return true;
}

QueueElement dequeue(Queue q) {
    if (QueueIsEmpty(q)) return NULL;

    QueueElement element = (q->array)[q->head];
    (q->array)[q->head] = NULL;
    q->head = (q->head + 1) % (q->capacity);

    q->size--;
    return element;
}

bool QueueIsFull(Queue q) {
    return q->size == q->capacity;
}

bool QueueIsEmpty(Queue q) {
    return 0 == q->size;
}

int QueueSize(Queue q) {
    return q->size;
}

int QueueCapacity(Queue q) {
    return q->capacity;
}

QueueElement QueueFindByCondition(Queue q, conditionFunction cond, Parameter param) {
    QUEUE_FOR_EACH(item, q) {
        if (cond(item, param)) return item;
    }
    return NULL;
}

QueueElement QueueRemoveByCondition(Queue q, conditionFunction cond, Parameter param) {
    Queue temp_q = QueueCreate(q->capacity, q->free_func);
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
    for (; temp_q->size > 0;) dequeue(temp_q);
    QueueDestroy(temp_q);
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

    q->iterator++;
    if (q->iterator == q->size) return NULL;

    return (q->array)[(q->head + q->iterator) % (q->capacity)];

}