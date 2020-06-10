#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "byte_queue.h"

struct QNode {
    char byte;
    struct QNode* next;
};

struct byte_queue_t {
    struct QNode* front, *rear;
    int current_size;
};

struct QNode* newNode(char c) {
    struct QNode* temp = (struct QNode*)malloc(sizeof(*temp));
    if (temp == NULL) {
        return NULL;
    }
    temp->byte = c;
    temp->next = NULL;
    return temp;
}

ByteQueue createQueue() {
    ByteQueue q = (ByteQueue)malloc(sizeof(*q));
    if (q == NULL) {
        return NULL;
    }
    q->front = q->rear = NULL;
    q->current_size = 0;
    return q;
}

int queueSize(ByteQueue q) {
    return q->current_size;
}

bool enQueue(ByteQueue q, char c) {
    struct QNode* temp = newNode(c);
    if (temp == NULL) {
        return false;
    }

    if (q->rear == NULL) {
        q->front = q->rear = temp;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }
    q->current_size++;
    return true;
}

bool isEmpty(ByteQueue q) {
    return (q->front == NULL);
}

char deQueue(ByteQueue q) {
    if (q->front == NULL) {
        return NULL;
    }

    struct QNode* temp = q->front;
    q->front = q->front->next;
    
    if (q->front == NULL) {
        q->rear = NULL;
    }

    char c = temp->byte;
    free(temp);
    q->current_size--;
    return c;
}

char* deQueueString(ByteQueue q, int max_length) {
    int count = 0;
    char* result = (char*)malloc(max_length+1);
    if (result == NULL) {
        return NULL;
    }

    while (count != max_length && !isEmpty(q)) {
        result[count++] = deQueue(q);
    }

    result[count] = '\0';
    return result;
}

void enQueueString(ByteQueue q, char* to_enqueue) {
    for (int i = 0; i < strlen(to_enqueue); ++i) {
        enQueue(q, to_enqueue[i]);
    }
}