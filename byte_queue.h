#ifndef __BYTE_QUEUE_H_
#define __BYTE_QUEUE_H_

/**
 * represents a queue data structure for bytes (characters)
 */
typedef struct byte_queue_t* ByteQueue;

ByteQueue createQueue();

/**
 * return value is true if and only if operation successful
 */
bool enQueue(ByteQueue q, char c);

bool isEmpty(ByteQueue q);

char deQueue(ByteQueue q);

int queueSize(ByteQueue q);

/**
 * returns at most max_length characters dequeued at once from q
 */
char* deQueueString(ByteQueue q, int max_length);

/**
 * enqueues an amount of characters at once
 */
void enQueueString(ByteQueue q, char* to_enqueue);

#endif