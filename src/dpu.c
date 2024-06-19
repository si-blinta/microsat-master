#include <stdio.h>
#include "microsat_dpu.h"   
#include "utils.h"
#include "mram.h"
#include <alloc.h>

/**
 * SOLVER DATA TRANSFER
*/
__host int dpu_DB_offsets[13];
__host int dpu_vars[12];
__host config_t config ;

/**
 * RETURN VALUE
*/
__host int dpu_ret = STOPPED;

void initialize_chb(struct solver *S, int num_vars) {
  S->config.lastConflict = (int *)mem_alloc((num_vars+1) * sizeof(int));
  S->config.Q = (float *)mem_alloc((num_vars+1) * sizeof(float));
  S->config.plays = (int *)mem_alloc((num_vars+1) * sizeof(int));
  for (int i = 0; i < num_vars+1; i++) {
    S->config.lastConflict[i] = 0;
    S->config.Q[i] = 0.0;
  }
  
  S->config.alpha = 0.4;
}
void populate_solver_context(struct solver *dpu_solver)
{                     
  dpu_solver->DB = DPU_MRAM_HEAP_POINTER;
  dpu_solver->nVars = dpu_vars[0];
  dpu_solver->nClauses = dpu_vars[1];
  dpu_solver->mem_used = dpu_vars[2];
  dpu_solver->mem_fixed = dpu_vars[3];
  dpu_solver->maxLemmas = dpu_vars[4];
  dpu_solver->nLemmas = dpu_vars[5];
  dpu_solver->nConflicts = dpu_vars[6];
  dpu_solver->fast = dpu_vars[7];
  dpu_solver->slow = dpu_vars[8];
  dpu_solver->head = dpu_vars[9];
  dpu_solver->res = dpu_vars[10];
  dpu_solver->decision_counter = dpu_vars[11];
  dpu_solver->model = dpu_solver->DB + dpu_DB_offsets[0];
  dpu_solver->next = dpu_solver->DB + dpu_DB_offsets[1];
  dpu_solver->prev = dpu_solver->DB + dpu_DB_offsets[2];
  dpu_solver->buffer = dpu_solver->DB + dpu_DB_offsets[3];
  dpu_solver->reason = dpu_solver->DB + dpu_DB_offsets[4];
  dpu_solver->falseStack = dpu_solver->DB + dpu_DB_offsets[5];
  dpu_solver->forced = dpu_solver->DB + dpu_DB_offsets[6];
  dpu_solver->processed = dpu_solver->DB + dpu_DB_offsets[7];
  dpu_solver->assigned = dpu_solver->DB + dpu_DB_offsets[8];
  dpu_solver->falses = dpu_solver->DB + dpu_DB_offsets[9];
  dpu_solver->first = dpu_solver->DB + dpu_DB_offsets[10];
  dpu_solver->scores= ( float __mram_ptr*)dpu_solver->DB + dpu_DB_offsets[11];
  dpu_solver->decision_level= dpu_solver->DB + dpu_DB_offsets[12];
  dpu_solver->config = config;  
  initialize_chb(dpu_solver,dpu_solver->nVars);
  setup_functions();

}
/**
 * Initialization flag
*/
int first ;
/**
 * Iterations
*/
__host int dpu_iterations;
__host int dpu_id; 

struct solver dpu_solver;
int main()
{
  if(first == 0)
  {
    populate_solver_context(&dpu_solver);
    first = 1;
  }
  dpu_ret = solve(&dpu_solver,dpu_iterations);
  log_config(dpu_solver.config);
}