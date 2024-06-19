#ifndef MICRO_SAT_H
#define MICRO_SAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <defs.h>
#define MAX_CLAUSE_SIZE 100
#define MAX_LEARNT_CLAUSES 100
#define REDUCE_LIMIT 2
#define MAX_LEMMAS 2000
#define DECISION_DEBUG 0
#define RESTART_DEBUG 0
#define REDUCE_DEBUG 0
#define MAX_LBD   10
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define LOG_LEVEL_DEBUG ANSI_COLOR_CYAN"[DEBUG]"ANSI_COLOR_RESET
#define LOG_LEVEL_INFO ANSI_COLOR_GREEN"[INFO]"ANSI_COLOR_RESET
#define LOG_LEVEL_WARNING ANSI_COLOR_YELLOW "[WARNING]" ANSI_COLOR_RESET
#define LOG_LEVEL_ERROR ANSI_COLOR_RED "[ERROR]" ANSI_COLOR_RESET 
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

}config_t;
struct solver
{ // The variables in the struct are described in the allocate procedure
  int nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, nConflicts, head, res, fast, slow,decision_counter;
  config_t config;
  int __mram_ptr* DB;
  int __mram_ptr* buffer;
  int __mram_ptr* model;
  int __mram_ptr* reason;
  int __mram_ptr* falseStack;
  int __mram_ptr* falses;
  int __mram_ptr* first;
  int __mram_ptr* forced;
  int __mram_ptr* processed;
  int __mram_ptr* assigned;
  int __mram_ptr* next;
  int __mram_ptr* prev;
  float __mram_ptr* scores;
  int __mram_ptr* decision_level;
   
};
typedef int (*branching_function)(struct solver *S,int decision);
extern branching_function branching_funcs[3];

typedef void (*restart_function)(struct solver *);
extern restart_function restart_funcs[4];

typedef int (*reduce_functions)(struct solver *S, int count, int k, float size, int clause_size, float lb);
extern reduce_functions reduce_funcs[3];
void setup_reduce_functions();
void setup_functions();
void restart_default(struct solver *S);
void restart_luby(struct solver *S);
void restart_geo(struct solver *S);
void restart_arith(struct solver *S);
int branching_vmtf(struct solver* S,int decision);
int branching_vsids(struct solver *S,int decision);
int branching_chb(struct solver *S,int decision);
int reduce_default(struct solver *S, int count, int k, float size, int clause_size, float lbd);
int reduce_size(struct solver *S, int count, int k, float size, int clause_size, float lbd);
int reduce_lbd(struct solver *S, int count, int k, float size, int clause_size, float lbd);
int luby(int y, int x);

void reset_solver(struct solver *S);
void unassign(struct solver *S, int lit,int flag);
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
int solve_geometric(struct solver* S,int stop_it,float geometric_factor,int min_thresh_hold);
int solve_fixed(struct solver* S, int stop_it, int fixed_thresh_hold);
void picosat_proof(struct solver S);
int solve_portfolio(struct solver *S,int restart_p,int stop_it,int thresh_hold);
int solve_random(struct solver* S, int stop_it);
int solve_luby(struct solver* S,int stop_it,int luby_param);
void reset_solver(struct solver *S);
int* get_reasons(struct solver S);
void reset_decision_levels(struct solver *S);
#endif 
