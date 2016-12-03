#include "ac5x.h"
#include "ac5x_circular_queue.h"

AC5X_CIRCULAR_QUEUE * ac5x_circular_queue_create(int max) {
    AC5X_CIRCULAR_QUEUE *queue = malloc(sizeof(AC5X_CIRCULAR_QUEUE));
    memset(queue, 0, sizeof(AC5X_CIRCULAR_QUEUE));
    queue->buffer = malloc(max * sizeof(void *));
    return queue;
}

void ac5x_circular_queue_destroy(AC5X_CIRCULAR_QUEUE *queue) {
    if (NULL != queue) {
        if (queue->buffer) {
            free(queue->buffer);
        }

        free(queue);
    }
}

int ac5x_circular_queue_push(AC5X_CIRCULAR_QUEUE *q, void *data) {
    int next = q->head + 1;
    if (next >= q->maxLen)
        next = 0;

    // Cicular buffer is full
    if (next == q->tail)
        return -1;  // quit with an error

    q->buffer[q->head] = data;
    q->head = next;
    return 0;
}

void *ac5x_circular_queue_pop(AC5X_CIRCULAR_QUEUE *q) {
    void *data = NULL;
    // if the head isn't ahead of the tail, we don't have any characters
    if (q->head == q->tail)
        return NULL;  // quit with an error

    data = q->buffer[q->tail];
    q->buffer[q->tail] = 0;  // clear the data (optional)

    int next = q->tail + 1;
    if(next >= q->maxLen)
        next = 0;

    q->tail = next;

    return data;
}

void *ac5x_circular_queue_top(AC5X_CIRCULAR_QUEUE *q) {
    if (q->head == q->tail) {
        return NULL;
    }

    return (q->buffer[q->tail]);
}

