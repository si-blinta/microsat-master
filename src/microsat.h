#ifndef MICRO_SAT_H
#define MICRO_SAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum
{
  END = -9,
  UNSAT = 0,
  SAT = 1,
  MARK = 2,
  IMPLIED = 6,
  MEM_MAX = (2 << 12)
}; // 64 Kbytes is maximum memory that a dpu can allocate.(to modify)

struct solver
{ // The variables in the struct are described in the allocate procedure
  int *DB, nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, *buffer, nConflicts, *model,
      *reason, *falseStack, *falses, *first, *forced, *processed, *assigned, *next, *prev, head, res, fast, slow;
};
void unassign(struct solver *S, int lit);
void restart(struct solver *S);
void assign(struct solver *S, int *reason, int forced);
void addWatch(struct solver *S, int lit, int mem);
int *getMemory(struct solver *S, int mem_size);
int *addClause(struct solver *S, int *in, int size, int irr);
void reduceDB(struct solver *S, int k);
void bump(struct solver *S, int lit);
int implied(struct solver *S, int lit);
int *analyze(struct solver *S, int *clause);
int propagate(struct solver *S);
int solve(struct solver *S);
#ifndef DPU
void initCDCL(struct solver *S, int n, int m);
static void read_until_new_line(FILE *input);
int parse(struct solver *S, char *filename);
#endif // DPU
void show_solver_info_debug(struct solver S);
void show_solver_stats(struct solver S);
#endif