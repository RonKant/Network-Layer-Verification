# include "array_queue.h"


int main() {
    Queue q = QueueCreate();
    if (q == NULL) {
        return -1;
    }
    printf("1.\n");
    if (!QueueIsEmpty(q)) return -1;
    printf("2.\n");
    QUEUE_FOR_EACH(item, q) return -1;
    printf("3.\n");
    char items[DEFAULT_QUEUE_CAPACITY];
    for (int i = 0; i < DEFAULT_QUEUE_CAPACITY; ++i) {
        items[i] = i % 100 + 1;
    }

    for (int i = 0; i < DEFAULT_QUEUE_CAPACITY; ++i) {
        enqueue(q, items[i]);
    }

    if (!QueueIsFull(q)) return -1;
    printf("4.\n");
    Queue copy_q = QueueCopy(q);
    if (NULL == copy_q) return -1;
    printf("5.\n");

    for (int i = 0; i < DEFAULT_QUEUE_CAPACITY; ++i) {
        char dequeued = dequeue(copy_q);
        if (dequeued != i % 100 + 1) return -1;
    }
    printf("5.\n");
    QueueDestroy(copy_q);



    QueueDestroy(q);
    printf("Success.\n");
    return 0;
}
