typedef struct {
	void **buffer;
	int head;
	int tail;
	const int maxLen;
}AC5X_CIRCULAR_QUEUE;

AC5X_CIRCULAR_QUEUE *ac5x_circular_queue_init(U32 max);
void ac5x_circular_queue_destroy(AC5X_CIRCULAR_QUEUE *queue);
int ac5x_circular_queue_push(AC5X_CIRCULAR_QUEUE *q, void *data);
void *ac5x_circular_queue_pop(AC5X_CIRCULAR_QUEUE *q);
void *ac5x_circular_queue_top(AC5X_CIRCULAR_QUEUE *q);

