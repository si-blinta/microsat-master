#ifndef DIVIDE_CONQUER_H
#define DIVIDE_CONQUER_H
#include "microsat.h"
#define QUEUE_SIZE 100
#define MAX_ASSIGNMENT_SIZE 1000// Define the maximum size of each assignment array

typedef struct {
    int assignments[QUEUE_SIZE][MAX_ASSIGNMENT_SIZE]; // Queue of assignment arrays
    int sizes[QUEUE_SIZE]; // Array to store the size of each assignment
    int head;
    int size;
} assignment_queue_t;


void print_queue(assignment_queue_t *queue);
// Initialize the queue
void init_queue(assignment_queue_t *queue);

// Check if the queue is empty
int is_empty(assignment_queue_t *queue);

// Check if the queue is full
int is_full(assignment_queue_t *queue);

// Enqueue an array of assignments
int enqueue(assignment_queue_t *queue, int assignment[], int assignment_size);

// Dequeue an array of assignments
int dequeue(assignment_queue_t *queue, int assignment[], int *assignment_size);

// Clear the queue
void clear_queue(assignment_queue_t *queue);

#endif
