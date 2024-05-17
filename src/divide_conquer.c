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
// Functions for solver_queue_t
void print_solver_queue(solver_queue_t *queue) {
    if (is_solver_queue_empty(queue)) {
        printf("Solver queue is empty.\n");
        return;
    }
    
    printf("Solver queue contents:\n");
    for (int i = 0; i < queue->size; i++) {
        int index = (queue->head + i) % QUEUE_SIZE;
        printf("DB %d (size %d): [", i + 1, queue->sizes[index]);
        for (int j = 0; j < queue->sizes[index]; j++) {
            printf("%d", queue->DB[index][j]);
            if (j < queue->sizes[index] - 1) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

void init_solver_queue(solver_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}

int is_solver_queue_empty(solver_queue_t *queue) {
    return queue->size == 0;
}

int is_solver_queue_full(solver_queue_t *queue) {
    return queue->size == QUEUE_SIZE;
}

int enqueue_solver_queue(solver_queue_t *queue, int DB[], int DB_size) {
    if (is_solver_queue_full(queue) || DB_size > DB_SIZE) {
        return 0; // Queue is full or DB size exceeds maximum allowed
    }
    int tail = (queue->head + queue->size) % QUEUE_SIZE;
    memcpy(queue->DB[tail], DB, sizeof(int) * DB_size);
    queue->sizes[tail] = DB_size;
    queue->size++;
    return 1; // Enqueue successful
}

int dequeue_solver_queue(solver_queue_t *queue, int DB[], int *DB_size) {
    if (is_solver_queue_empty(queue)) {
        return 0; // Queue is empty
    }
    int current_head = queue->head;
    if (DB_size != NULL)
        *DB_size = queue->sizes[current_head];
    memcpy(DB, queue->DB[current_head], sizeof(int) * (*DB_size));
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return 1; // Dequeue successful
}

void clear_solver_queue(solver_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}
void print_vars_offsets_queue(vars_offsets_queue_t *queue) {
    if (is_vars_offsets_queue_empty(queue)) {
        printf("Vars/Offsets queue is empty.\n");
        return;
    }

    printf("Vars/Offsets queue contents:\n");
    for (int i = 0; i < queue->size; i++) {
        int index = (queue->head + i) % QUEUE_SIZE;
        printf("Vars/Offsets %d:\n", i + 1);
        printf("Vars: [");
        for (int j = 0; j < 11; j++) {
            printf("%d", queue->vars[index][j]);
            if (j < 10) {
                printf(", ");
            }
        }
        printf("]\nOffsets: [");
        for (int j = 0; j < 11; j++) {
            printf("%d", queue->offsets[index][j]);
            if (j < 10) {
                printf(", ");
            }
        }
        printf("]\n");
    }
}

void init_vars_offsets_queue(vars_offsets_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}

int is_vars_offsets_queue_empty(vars_offsets_queue_t *queue) {
    return queue->size == 0;
}

int is_vars_offsets_queue_full(vars_offsets_queue_t *queue) {
    return queue->size == QUEUE_SIZE;
}

int enqueue_vars_offsets_queue(vars_offsets_queue_t *queue, int vars[], int offsets[]) {
    if (is_vars_offsets_queue_full(queue)) {
        return 0; // Queue is full
    }
    int tail = (queue->head + queue->size) % QUEUE_SIZE;
    memcpy(queue->vars[tail], vars, sizeof(int) * 11);
    memcpy(queue->offsets[tail], offsets, sizeof(int) * 11);
    queue->size++;
    return 1; // Enqueue successful
}

int dequeue_vars_offsets_queue(vars_offsets_queue_t *queue, int vars[], int offsets[]) {
    if (is_vars_offsets_queue_empty(queue)) {
        return 0; // Queue is empty
    }
    int current_head = queue->head;
    memcpy(vars, queue->vars[current_head], sizeof(int) * 11);
    memcpy(offsets, queue->offsets[current_head], sizeof(int) * 11);
    queue->head = (queue->head + 1) % QUEUE_SIZE;
    queue->size--;
    return 1; // Dequeue successful
}

void clear_vars_offsets_queue(vars_offsets_queue_t *queue) {
    queue->head = 0;
    queue->size = 0;
}