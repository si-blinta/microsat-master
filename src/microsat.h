#ifndef MICRO_SAT_H
#define MICRO_SAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DPU
#include <defs.h>
#endif
#define MAX_CLAUSE_SIZE 2
#define MAX_LEARNT_CLAUSES 5
enum
{
  END = -9,
  UNSAT = 0,
  SAT = 1,
  STOPPED=3,
  MARK = 2,
  IMPLIED = 6,
  MEM_MAX = 10000000,
  NO_RELAUNCH = 99
}; // 64 Kbytes is maximum memory that a dpu can allocate.(to modify)
enum
{
  MODEL,
  NEXT,
  PREV,
  BUFFER,
  REASON,
  FALSESTACK,
  FALSES,
  FIRST,
  CLAUSES
};
#ifndef DPU
struct solver
{ // The variables in the struct are described in the allocate procedure
  int *DB, nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, *buffer, nConflicts, *model,
      *reason, *falseStack, *falses, *first, *forced, *processed, *assigned, *next, *prev, head, res, fast, slow;
};
#else   
struct solver
{ // The variables in the struct are described in the allocate procedure
  int nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, nConflicts, head, res, fast, slow;
  int __mram_ptr* DB;
  int __mram_ptr* buffer;
  int __mram_ptr*model;
  int __mram_ptr*reason;
  int __mram_ptr*falseStack;
  int __mram_ptr* falses;
  int __mram_ptr*first;
  int __mram_ptr*forced;
  int __mram_ptr*processed;
  int __mram_ptr*assigned;
  int __mram_ptr*next;
  int __mram_ptr*prev;
};
#endif
#ifndef DPU
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
int solve(struct solver *S,int stop_it);

void initCDCL(struct solver *S, int n, int m);
static void read_until_new_line(FILE *input);
int parse(struct solver *S, char *filename);

void show_solver_info_debug(struct solver S);
void show_solver_stats(struct solver S);
void show_result(struct solver S);
void assign_decision (struct solver* S, int lit);
void unassign_last_decision(struct solver *S);
#else
void unassign(struct solver *S, int lit);
void restart(struct solver *S);
void assign(struct solver *S, int __mram_ptr* reason, int forced);
void addWatch(struct solver *S, int lit, int mem);
int __mram_ptr* getMemory(struct solver *S, int mem_size);
int __mram_ptr* addClause(struct solver *S, int __mram_ptr* in, int size, int irr);
void reduceDB(struct solver *S, int k);
void bump(struct solver *S, int lit);
int implied(struct solver *S, int lit);
int __mram_ptr* analyze(struct solver *S, int __mram_ptr* clause);
int propagate(struct solver *S);
int solve(struct solver *S,int stop_it);

void show_solver_info_debug(struct solver S);
void show_solver_stats(struct solver S);
void show_result(struct solver S);
void assign_decision (struct solver* S, int lit);
void unassign_last_decision(struct solver *S);

#endif // DPU 

#endif