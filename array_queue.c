#include <stdio.h>

#include "array_queue.h"
#include "socket_utils.h"
#include "network.h"
#include "Functions.h"
#include "seahorn/seahorn.h"

extern void* nd_ptr();

struct Element_t{
    QueueElement element;
    void* ghost_p;
    void* ghost_q;
    int ghost_flag;
};

struct Queue_t {
    int head;
    int tail;
    int size;
    int capacity;
    Element* array; //CHECK
    int iterator;
};
Element ElementCreate(){
    Element element = xmalloc(sizeof(Element));
    if(element == NULL)
        return NULL;
    element->element = NULL;
    element->ghost_p = nd_ptr();
    element->ghost_q = nd_ptr();
    element->ghost_flag = 0;
    return element;
}
bool ElementPut(Element element, QueueElement queueElement){
    element->ghost_flag = (element->ghost_p == queueElement);
    if(element->ghost_flag){
        element->ghost_p = queueElement;
        return true;
    }
    return false;
}
//EDIT
QueueElement ElementGet(Element element){
    void* q = xmalloc(sizeof(Socket));
    if(element->ghost_flag && q == element->ghost_q){
        q = element->ghost_q;
    }
    return q;
}
Queue QueueCreate(int capacity) {
    if (capacity <= 0) return NULL;
    Queue queue = xmalloc(sizeof(Queue));
    if (NULL == queue) return NULL;

    queue->head = 0;
    queue->tail = capacity - 1;
    queue->size = 0;
    queue->capacity = capacity;

    queue->array = xmalloc(sizeof(Element) * capacity);
    if (NULL == queue->array) {
        free(queue);
        return NULL;
    }

    for (int i = 0; i < capacity; i = i+1 ) (queue->array)[i] = ElementCreate();
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

    bool put_element = ElementPut((q->array)[q->tail],element);
    if(put_element){
        q->size = q->size + 1;
        QueueElement queueGet = ElementGet((q->array)[q->tail]);
        sassert(queueGet == element);
        return true;
    }

    return false;
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
QueueElement QueueGetElement(Queue q, int index){
    if(q == NULL || q->array == NULL)
        return NULL;
    return q->array[index];
}