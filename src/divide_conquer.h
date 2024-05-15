#ifndef DIVIDE_CONQUER_H
#define DIVIDE_CONQUER_H
#include "microsat.h"

typedef struct 
{
    int* lit;
    int nr_lit;

}assignement_t;

typedef struct
{
    assignement_t *assignement;
    int head;
    int size;

}assignement_queue_t;


assignement_t* get_assignement(struct solver *S,int max_lit);






















#endif