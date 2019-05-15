#ifndef REQQUEUE_H
#define REQQUEUE_H

//https://gist.github.com/ArnonEilat/4471278

#include "types.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct Node_t {
    tlv_request_t info;
    struct Node_t *prev;
} NODE;

/* the HEAD of the Queue, hold the amount of node's that are in the queue*/
typedef struct Queue {
    NODE *head;
    NODE *tail;
    int size;
    int limit;
} Queue;

Queue *ConstructQueue(int limit);

void DestructQueue(Queue *queue);

bool Enqueue(Queue *pQueue, tlv_request_t *info);

NODE* Dequeue(Queue *pQueue);

bool isEmpty(Queue* pQueue);

#endif