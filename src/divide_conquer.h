#ifndef DIVIDE_CONQUER_H
#define DIVIDE_CONQUER_H
#include "microsat.h"
#define QUEUE_SIZE 100
#define MAX_ASSIGNMENT_SIZE 1000 // Define the maximum size of each assignment array
#define DB_SIZE 10000

typedef struct {
    int assignments[QUEUE_SIZE][MAX_ASSIGNMENT_SIZE]; // Queue of assignment arrays
    int sizes[QUEUE_SIZE]; // Array to store the size of each assignment
    int head;
    int size;
} assignment_queue_t;

typedef struct {
    int DB[QUEUE_SIZE][DB_SIZE];
    int sizes[QUEUE_SIZE];
    int head;
    int size;
} solver_queue_t;

typedef struct {
    int vars[QUEUE_SIZE][11];
    int offsets[QUEUE_SIZE][11];
    int head;
    int size;
} vars_offsets_queue_t;

void print_queue(assignment_queue_t *queue);
void init_queue(assignment_queue_t *queue);
int is_empty(assignment_queue_t *queue);
int is_full(assignment_queue_t *queue);
int enqueue(assignment_queue_t *queue, int assignment[], int assignment_size);
int dequeue(assignment_queue_t *queue, int assignment[], int *assignment_size);
void clear_queue(assignment_queue_t *queue);

void print_solver_queue(solver_queue_t *queue);
void init_solver_queue(solver_queue_t *queue);
int is_solver_queue_empty(solver_queue_t *queue);
int is_solver_queue_full(solver_queue_t *queue);
int enqueue_solver_queue(solver_queue_t *queue, int DB[], int DB_size);
int dequeue_solver_queue(solver_queue_t *queue, int DB[], int *DB_size);
void clear_solver_queue(solver_queue_t *queue);

void print_vars_offsets_queue(vars_offsets_queue_t *queue);
void init_vars_offsets_queue(vars_offsets_queue_t *queue);
int is_vars_offsets_queue_empty(vars_offsets_queue_t *queue);
int is_vars_offsets_queue_full(vars_offsets_queue_t *queue);
int enqueue_vars_offsets_queue(vars_offsets_queue_t *queue, int vars[], int offsets[]);
int dequeue_vars_offsets_queue(vars_offsets_queue_t *queue, int vars[], int offsets[]);
void clear_vars_offsets_queue(vars_offsets_queue_t *queue);
#endif