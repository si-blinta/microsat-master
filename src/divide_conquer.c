#include "divide_conquer.h"
#include <stdlib.h>
#include <string.h>
void print_queue(assignment_queue_t *queue) {
    if (is_empty(queue)) {
        printf("Queue is empty.\n");
        return;
    }
    
    printf("Queue contents:\n");
    for (int i = 0; i < queue->size; i++) {
        int index = (queue->head + i) % QUEUE_SIZE;
        printf("Assignment %d (size %d): [", i + 1, queue->sizes[index]);
        for (int j = 0; j < queue->sizes[index]; j++) {
            printf("%d", queue->assignments[index][j]);
            if (j < queue->sizes[index] - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}
// Initialize the queue
void init_queue(assignment_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}

// Check if the queue is empty
int is_empty(assignment_queue_t *queue) {
    return queue->size == 0;
}

// Check if the queue is full
int is_full(assignment_queue_t *queue) {
    return queue->size == QUEUE_SIZE;
}

// Enqueue an array of assignments
int enqueue(assignment_queue_t *queue, int assignment[], int assignment_size) {
    if (is_full(queue) || assignment_size > MAX_ASSIGNMENT_SIZE) {
        return 0; // Queue is full or assignment size exceeds maximum allowed
    }
    int tail = (queue->head + queue->size) % QUEUE_SIZE;
    memcpy(queue->assignments[tail], assignment, sizeof(int) * assignment_size);
    queue->sizes[tail] = assignment_size;
    queue->size++;
    return 1; // Enqueue successful
}

// Dequeue an array of assignments
int dequeue(assignment_queue_t *queue, int assignment[], int *assignment_size) {
    if (is_empty(queue)) {
        return 0; // Queue is empty
    }
    int current_head = queue->head;
    if(assignment_size != NULL)
        *assignment_size = queue->sizes[current_head];
    memcpy(assignment, queue->assignments[current_head], sizeof(int) * (*assignment_size));
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return 1; // Dequeue successful
}

// Clear the queue
void clear_queue(assignment_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}
