#include "array_queue.h"

struct Queue_t {
    int head, tail, size, capacity;
    QueueElement* array;
    int iterator;
}

Queue QueueCreate(int capacity) {
    if (capacity <= 0) return NULL;
    Queue queue = (Queue)malloc(sizeof(*queue));
    if (NULL == queue) return NULL;

    queue->capacity = capacity;
    queue->head = queue->size = 0;
    queue->tail = capacity - 1;

    queue->array = (QueueElement*)malloc(sizeof(QueueElement) * capacity);
    if (NULL == queue->array) {
        free(queue);
        return NULL;
    }

    for (int i = 0; i < capacity; ++i) (queue->array)[i] = NULL;
    return queue;
}

void QueueDestroy(Queue q, freeElem free_func) {
    if (NULL == q || NULL == free_func) return;
    if (NULL == q->array) return;
    
    QUEUE_FOR_EACH(item, q) {
        free_func(item);
    }
    free(q->array);
    free(q);
}

Queue QueueCopy(Queue q) {
    Queue copy_q = QueueCreate(q->capacity);
    QUEUE_FOR_EACH(item, q) {
        if (false == copy_q->enqueue(item)) {
            QueueDestroy(copy_q);
            return NULL;
        }
    }
    return copy_q;
}

bool enqueue(Queue q, QueueElement element) {
    if (QueueIsFull(q)) return false;

    queue->tail = (queue->tail + 1) % (queue->capacity);
    (queue->array)[queue->tail] = element;
    queue->size++;

    return true;
}

QueueElement dequeue(Queue q) {
    if (QueueIsEmpty(q)) return NULL;

    QueueElement element = (queue->array)[queue->head];
    (queue->array)(queue->head) = NULL;
    queue->head = (queue->head + 1) % (queue->capacity);

    q->size--;
    return element;
}

bool QueueIsFull(Queue q) {
    return queue->size == queue->capacity;
}

bool QueueIsEmpty(Queue q) {
    return 0 == queue->size;
}

int QueueSize(Queue q) {
    return q->size;
}

int QueueCapacity(Queue q) {
    return q->capacity;
}

QueueElement QueueFindByCondition(Queue q, conditionFunction cond) {
    QUEUE_FOR_EACH(item, q) {
        if (cond(item)) return item;
    }
    return NULL;
}

QueueElement QueueRemoveByCondition(Queue q, conditionFunction cond) {
    Queue temp_q = QueueCreate(q->capacity);
    QueueElement removed = NULL;
    QUEUE_FOR_EACH(item, q) {
        if (NULL != removed || false == cond(item)) {
            enqueue(temp_q, item))
        } else if (cond(item)) {
            removed = item;
        }
    }

    for (int i = 0; i < q->size; ++i) dequeue(q);
    QUEUE_FOR_EACH(item, temp_q) enqueue(q, item);
    return removed;
}

QueueElement QueueGetFirst(Queue q) {
    if (NULL == q || NULL == q->array) return NULL;
    if (QueueIsEmpty(q)) return NULL;

    q->iterator = 0;
    return (queue->array)[queue->front];
}

QueueElement QueueGetNext(Queue q) {
    if (NULL == q || NULL == q->array) return NULL;

    q->iterator++;
    if (q->iterator == q->size) return NULL;

    return (queue->array)[(queue->front + q->iterator) % (q->capacity)];

}