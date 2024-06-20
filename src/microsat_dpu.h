#ifndef MICRO_SAT_H
#define MICRO_SAT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <defs.h>
#include "utils.h"
#include "mram.h"
#include <alloc.h>
// Linear Congruential Generator (LCG) constants
#define LCG_A 1664525
#define LCG_C 1013904223
#define LCG_M ((uint64_t)1 << 32)
#define MAX_CLAUSE_SIZE 100
#define MAX_LEARNT_CLAUSES 100
#define REDUCE_LIMIT 2
#define MAX_LEMMAS 2000
#define DECISION_DEBUG 0
#define RESTART_DEBUG 0
#define REDUCE_DEBUG 0
#define MAX_LBD 10
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define LOG_LEVEL_DEBUG ANSI_COLOR_CYAN "[DEBUG]" ANSI_COLOR_RESET
#define LOG_LEVEL_INFO ANSI_COLOR_GREEN "[INFO]" ANSI_COLOR_RESET
#define LOG_LEVEL_WARNING ANSI_COLOR_YELLOW "[WARNING]" ANSI_COLOR_RESET
#define LOG_LEVEL_ERROR ANSI_COLOR_RED "[ERROR]" ANSI_COLOR_RESET
enum
{
  END = -9,
  UNSAT = 0,
  SAT = 1,
  STOPPED = 3,
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

enum restart_policy
{
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
  // Variables for restart policy
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
  float *Q;
  int *plays;
  int *lastConflict;
  float alpha;

} config_t;
struct solver
{ // The variables in the struct are described in the allocate procedure
  int nVars, nClauses, mem_used, mem_fixed, maxLemmas, nLemmas, nConflicts, head, res, fast, slow, decision_counter;
  config_t config;
  int __mram_ptr *DB;
  int __mram_ptr *buffer;
  int __mram_ptr *model;
  int __mram_ptr *reason;
  int __mram_ptr *falseStack;
  int __mram_ptr *falses;
  int __mram_ptr *first;
  int __mram_ptr *forced;
  int __mram_ptr *processed;
  int __mram_ptr *assigned;
  int __mram_ptr *next;
  int __mram_ptr *prev;
  float __mram_ptr *scores;
  int __mram_ptr *decision_level;
};
typedef int (*branching_function)(struct solver *S, int decision);
extern branching_function branching_funcs[3];

typedef void (*restart_function)(struct solver *);
extern restart_function restart_funcs[4];

typedef int (*reduce_functions)(struct solver *S, int count, int k, float size, int clause_size, float lb);
extern reduce_functions reduce_funcs[3];
/**
 * @brief Sets up the reduce functions.
 * @param None
 * @return None
 */
void setup_reduce_functions();

/**
 * @brief Sets up the branching functions.
 * @param None
 * @return None
 */
void setup_branching_functions();

/**
 * @brief Sets up the restart functions.
 * @param None
 * @return None
 */
void setup_restart_functions();

/**
 * @brief Sets up all functions by calling setup functions for reduce, branching, and restart.
 * @param None
 * @return None
 */
void setup_functions();

/**
 * @brief Default reduce function.
 * @param S Pointer to the solver structure.
 * @param count Integer count of satisfied literals.
 * @param k Integer threshold.
 * @param size Floating point size of the clause.
 * @param clause_size Integer size of the clause.
 * @param lbd Floating point literal block distance.
 * @return Integer indicating whether the clause should be reduced.
 */
int reduce_default(struct solver *S, int count, int k, float size, int clause_size, float lbd);

/**
 * @brief Reduce function based on size.
 * @param S Pointer to the solver structure.
 * @param count Integer count of satisfied literals.
 * @param k Integer threshold.
 * @param size Floating point size of the clause.
 * @param clause_size Integer size of the clause.
 * @param lbd Floating point literal block distance.
 * @return Integer indicating whether the clause should be reduced.
 */
int reduce_size(struct solver *S, int count, int k, float size, int clause_size, float lbd);

/**
 * @brief Reduce function based on LBD (Literal Block Distance).
 * @param S Pointer to the solver structure.
 * @param count Integer count of satisfied literals.
 * @param k Integer threshold.
 * @param size Floating point size of the clause.
 * @param clause_size Integer size of the clause.
 * @param lbd Floating point literal block distance.
 * @return Integer indicating whether the clause should be reduced.
 */
int reduce_lbd(struct solver *S, int count, int k, float size, int clause_size, float lbd);

/**
 * @brief Default restart function.
 * @param S Pointer to the solver structure.
 * @return None
 */
void restart_default(struct solver *S);

/**
 * @brief Luby restart function.
 * @param S Pointer to the solver structure.
 * @return None
 */
void restart_luby(struct solver *S);

/**
 * @brief Geometric restart function.
 * @param S Pointer to the solver structure.
 * @return None
 */
void restart_geo(struct solver *S);

/**
 * @brief Arithmetic restart function.
 * @param S Pointer to the solver structure.
 * @return None
 */
void restart_arith(struct solver *S);

/**
 * @brief VMTF (Variable Move to Front) branching function.
 * @param S Pointer to the solver structure.
 * @param decision Integer decision variable.
 * @return Integer decision variable after applying VMTF.
 */
int branching_vmtf(struct solver *S, int decision);

/**
 * @brief VSIDS (Variable State Independent Decaying Sum) branching function.
 * @param S Pointer to the solver structure.
 * @param decision Integer decision variable.
 * @return Integer decision variable after applying VSIDS.
 */
int branching_vsids(struct solver *S, int decision);

/**
 * @brief CHB (Conflict History Based) branching function.
 * @param S Pointer to the solver structure.
 * @param decision Integer decision variable.
 * @return Integer decision variable after applying CHB.
 */
int branching_chb(struct solver *S, int decision);

/**
 * @brief Updates the CHB (Conflict History Based) score.
 * @param S Pointer to the solver structure.
 * @param var Integer variable.
 * @param multiplier Floating point multiplier.
 * @return None
 */
void update_chb(struct solver *S, int var, float multiplier);

/**
 * @brief Picks the branching variable using CHB (Conflict History Based).
 * @param S Pointer to the solver structure.
 * @return Integer best variable based on CHB score.
 */
int pick_branching_variable_chb(struct solver *S);

/**
 * @brief Logs decision information.
 * @param lit Integer literal.
 * @param level Integer decision level.
 * @return None
 */
void log_decision(int lit, int level);

/**
 * @brief Logs propagation information.
 * @param lit Integer literal.
 * @param level Integer decision level.
 * @return None
 */
void log_propagation(int lit, int level);

/**
 * @brief Logs unassignment information.
 * @param lit Integer literal.
 * @param level Integer decision level.
 * @return None
 */
void log_unassign(int lit, int level);

/**
 * @brief Logs conflict analysis information.
 * @param lit Integer literal.
 * @param level Integer decision level.
 * @return None
 */
void log_conflict_analysis(int lit, int level);

/**
 * @brief Prints a clause.
 * @param S Pointer to the solver structure.
 * @param clause Pointer to the clause.
 * @return None
 */
void print_clause(struct solver *S, int *clause);

/**
 * @brief Computes the power of a number.
 * @param base Integer base.
 * @param exponent Integer exponent.
 * @return Integer result of base raised to the exponent.
 */
int power(int base, int exponent);

/**
 * @brief Computes the Luby sequence value.
 * @param y Integer base.
 * @param x Integer index.
 * @return Integer value of the Luby sequence.
 */
int luby(int y, int x);

/**
 * @brief Checks and restarts if necessary.
 * @param S Pointer to the solver structure.
 * @return None
 */
void check_and_restart(struct solver *S);

/**
 * @brief Decides the next branching variable.
 * @param S Pointer to the solver structure.
 * @param decision Integer decision variable.
 * @return Integer next decision variable.
 */
int decide(struct solver *S, int decision);

/**
 * @brief Unassigns a literal.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @param flag Integer flag indicating whether it is restarting or not.
 * @return None
 */
void unassign(struct solver *S, int lit, int flag);

/**
 * @brief Resets the decision levels.
 * @param S Pointer to the solver structure.
 * @return None
 */
void reset_decision_levels(struct solver *S);

/**
 * @brief Restarts the solver.
 * @param S Pointer to the solver structure.
 * @return None
 */
void restart(struct solver *S);

/**
 * @brief Assigns a literal.
 * @param S Pointer to the solver structure.
 * @param reason Pointer to the reason clause.
 * @param forced Integer indicating whether the assignment is forced.
 * @return None
 */
void assign(struct solver *S, int __mram_ptr *reason, int forced);

/**
 * @brief Adds a watch pointer to a clause containing a literal.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @param mem Integer memory location.
 * @return None
 */
void addWatch(struct solver *S, int lit, int mem);

/**
 * @brief Allocates memory of specified size.
 * @param S Pointer to the solver structure.
 * @param mem_size Integer size of the memory to allocate.
 * @return Pointer to the allocated memory.
 */
int __mram_ptr *getMemory(struct solver *S, int mem_size);

/**
 * @brief Adds a clause to the solver's database.
 * @param S Pointer to the solver structure.
 * @param in Pointer to the input clause.
 * @param size Integer size of the clause.
 * @param irr Integer indicating whether the clause is irreducible.
 * @return Pointer to the added clause in the database.
 */
int __mram_ptr *addClause(struct solver *S, int __mram_ptr *in, int size, int irr);

/**
 * @brief Computes the Literal Block Distance (LBD) of a clause.
 * @param S Pointer to the solver structure.
 * @param clause Pointer to the clause.
 * @return Integer LBD value.
 */
int compute_lbd(struct solver *S, int __mram_ptr *clause);

/**
 * @brief Checks if a clause has an LBD of 2 or less.
 * @param S Pointer to the solver structure.
 * @param clause Pointer to the clause.
 * @return Integer indicating if the LBD is 2 or less.
 */
int is_lbd_2(struct solver *S, int __mram_ptr *clause);

/**
 * @brief Reduces the database by removing less useful lemmas.
 * @param S Pointer to the solver structure.
 * @param k Integer threshold.
 * @return None
 */
void reduceDB(struct solver *S, int k);

/**
 * @brief Increments the score of a literal.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @return None
 */
void increment(struct solver *S, int lit);

/**
 * @brief Bumps a literal to the front of the decision list.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @return None
 */
void bump(struct solver *S, int lit);

/**
 * @brief Decays the scores of variables.
 * @param S Pointer to the solver structure.
 * @param factor Floating point decay factor.
 * @return None
 */
void decay(struct solver *S, float factor);

/**
 * @brief Checks if a literal is implied by marked literals.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @return Integer indicating if the literal is implied.
 */
int implied(struct solver *S, int lit);

/**
 * @brief Analyzes a conflict and computes a resolvent.
 * @param S Pointer to the solver structure.
 * @param clause Pointer to the falsified clause.
 * @return Pointer to the computed resolvent.
 */
int __mram_ptr *analyze(struct solver *S, int __mram_ptr *clause);

/**
 * @brief Determines the satisfiability of the problem.
 * @param S Pointer to the solver structure.
 * @param stop_it Integer stop iteration count.
 * @return Integer result indicating SAT, UNSAT, or STOPPED.
 */
int solve(struct solver *S, int stop_it);

/**
 * @brief Performs unit propagation.
 * @param S Pointer to the solver structure.
 * @return Integer result indicating SAT or UNSAT.
 */
int propagate(struct solver *S);

/**
 * @brief Assigns a decision literal.
 * @param S Pointer to the solver structure.
 * @param lit Integer literal.
 * @return None
 */
void assign_decision(struct solver *S, int lit);

/**
 * @brief Prints the PicoSAT proof.
 * @param S Solver structure.
 * @return None
 */
void picosat_proof(struct solver S);

/**
 * @brief Displays the result of the solver.
 * @param S Solver structure.
 * @return None
 */
void show_result(struct solver S);

/**
 * @brief Shows the solver information for debugging.
 * @param S Solver structure.
 * @return None
 */
void show_solver_info_debug(struct solver S);

/**
 * @brief Shows the solver statistics.
 * @param S Solver structure.
 * @return None
 */
void show_solver_stats(struct solver S);

/**
 * @brief Shows the solver config.
 * @param config : the config.
 * @return None
 */
void log_config(config_t config);

/**
 * @brief Initializes the Conflict History Based (CHB) heuristic for the solver.
 * @param S Pointer to the solver structure.
 * @param num_vars Integer number of variables in the solver.
 * @return None
 */
void initialize_chb(struct solver *S, int num_vars);

/**
 * @brief Populates the solver context with the given data.
 * @param dpu_solver Pointer to the solver structure.
 * @param dpu_vars Pointer to an array of integer variables.
 * @param dpu_DB_offsets Pointer to an array of integer database offsets.
 * @param config Configuration structure.
 * @return None
 */
void populate_solver_context(struct solver *dpu_solver, int *dpu_vars, int* dpu_DB_offsets, config_t config);

/**
 * @brief Randomizes decision list of a solver based on a seed.
 */
void randomize_decision_list(struct solver *S);
void print_decision_list(struct solver *S);
#endif
