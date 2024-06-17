#ifndef MICRO_SAT_H
#define MICRO_SAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#define DECISION_DEBUG 0
#define RESTART_DEBUG 0 
#define REDUCE_DEBUG 0
#define MAX_CLAUSE_SIZE 100
#define MAX_LEARNT_CLAUSES 100
#define REDUCE_LIMIT 6
#define MAX_LEMMAS 6000


enum
{
  END = -9,
  UNSAT = 0,
  SAT = 1,
  STOPPED=3,
  MARK = 2,
  IMPLIED = 6,
  MEM_MAX = 15000000,
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

enum branching_policy
{
  BR_VSIDS,
  BR_VMTF,
  BR_CHB
};

enum restart_policy {
  REST_DEFAULT,
  REST_ARITH,
  REST_LUBY,
  REST_GEO
};

enum reduce_policy
{
  RED_DEFAULT,
  RED_SIZE,
  RED_LBD
};
typedef struct
{
  enum branching_policy br_p;
  enum restart_policy rest_p;
  enum reduce_policy reduce_p;
  //Variables for restart policy
  int conflicts;
  float geo_factor;
  int geo_max;
  int luby_base;
  int luby_index;
  int arith_reason;
  int arith_max;
  float decay_factor;
  int decay_thresh_hold;
  int clause_size;
  int max_lbd;
  float* Q;
  int* plays;
  int* lastConflict;
  float alpha;
  int numConflicts;

}config_t;
struct solver
{ // The variables in the struct are described in the allocate procedure
  int nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, nConflicts, head, res, fast, slow,decision_counter;
  config_t config;
  int * DB;
  int * buffer;
  int * model;
  int * reason;
  int * falseStack;
  int * falses;
  int * first;
  int * forced;
  int * processed;
  int * assigned;
  int * next;
  int * prev;
  float * scores;
  int * decision_level;


};
void unassign(struct solver *S, int lit,int flag);
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
void set_solver_config(struct solver *S,config_t config);
void set_solver_br(struct solver *S,enum branching_policy br);
void set_solver_rest(struct solver *S,enum restart_policy rest);
void set_solver_red(struct solver *S,enum reduce_policy red);



static void read_until_new_line(FILE *input);
int parse(struct solver *S, char *filename);

void show_solver_info_debug(struct solver S);
void show_solver_stats(struct solver S);
void show_result(struct solver S);
void assign_decision (struct solver* S, int lit);
void unassign_last_decision(struct solver *S);
int get_unassigned(struct solver S);
void picosat_proof(struct solver S);
void unassign_all(struct solver *S);
int* get_unassigned_lits(struct solver S,int *size);
int* get_assigned_lits(struct solver S, int* size);
void reset_solver(struct solver *S);
int* get_reasons(struct solver S);
void decay(struct solver *S,float factor);
void reset_decision_levels(struct solver *S);
void log_decision(int lit, int level) ;
void log_propagation(int lit, int level);
void log_unassign(int lit, int level);
void log_conflict_analysis(int lit, int level);
void print_clause(struct solver *S,int* clause);
void sort_variables(struct solver *S);

#endif