#include <stdio.h>
#include "microsat_dpu.h"   
#include "utils.h"
#include "mram.h"
#include <alloc.h>
void print_config(const config_t* config) {
    printf("Branching Policy: %d\n", config->br_p);
    printf("Restart Policy: %d\n", config->rest_p);
    printf("Reduce Policy: %d\n", config->reduce_p);
    printf("Conflicts: %d\n", config->conflicts);
    printf("Geo Factor: %f\n", config->geo_factor);
    printf("Geo Max: %d\n", config->geo_max);
    printf("Luby Base: %d\n", config->luby_base);
    printf("Luby Index: %d\n", config->luby_index);
    printf("Arith Reason: %d\n", config->arith_reason);
    printf("Arith Max: %d\n", config->arith_max);
    printf("Decay Factor: %f\n", config->decay_factor);
    printf("Decay Threshold: %d\n", config->decay_thresh_hold);
    printf("Clause Size: %d\n", config->clause_size);
    printf("Max LBD: %d\n", config->max_lbd);
    printf("Alpha: %f\n", config->alpha);
}
/**
 * SOLVER DATA TRANSFER
*/
__host int dpu_DB_offsets[13];
__host int dpu_vars[12];
__host config_t config ;
/*__host int learnt_clauses[MAX_LEARNT_CLAUSES+1][MAX_CLAUSE_SIZE];
__host int dpu_mem_used;
__host int dpu_old_mem_used;*/

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
  print_config(&dpu_solver.config);
  printf("nConflicts %d\n",dpu_solver.nConflicts);
}
/**
 * DIVIDE AND CONQUER

int first;
int relaunch;
void divide_and_conquer_kernel()
{
  struct solver dpu_solver;
  
  //printf("relaunch flag %d\n",relaunch);
  if(relaunch == NO_RELAUNCH)
  {
    return 0;
  }
  if(first == 0)
  {
    populate_solver_context(&dpu_solver);
      for (int j = 0; j < 10; j++) 
  {
    assign_decision(&dpu_solver,(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
    //printf("%d ",(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
  }
  }
  dpu_ret = solve(&dpu_solver,10);
  if(dpu_ret == SAT )
  {
    show_result(dpu_solver);
  }
  if(dpu_ret == UNSAT )
  {
    relaunch = NO_RELAUNCH;
    //printf("%d ",(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
  }
  first++;
}*/
/*__host int dpu_last_mem_used;
__host int dpu_mem_used;
__host int dpu_lc[MAX_LEARNT_CLAUSES][MAX_CLAUSE_SIZE];
__host int dpu_lc_sizes[MAX_LEARNT_CLAUSES];
__host int dpu_lc_count;
__mram_noinit int tmp[MAX_CLAUSE_SIZE];*/
/**
 * #if SHARING
  else
  {
    // Update database with new learned clauses.
    for(int i = 0 ; i < dpu_lc_count;i++)
    {
      mram_write(dpu_lc[i],tmp,roundup(dpu_lc_sizes[i],8)*sizeof(int));
      addClause(&dpu_solver,tmp,dpu_lc_sizes[i],0);
    } 
  }
#endif
*/
  
  

/** PURE PORTFOLIO
 * seed = dpu_args.seed;
  if(first == 0)
  {
    populate_solver_context(&dpu_solver);
    first = 1;
  }
  dpu_ret = solve_portfolio(&dpu_solver,dpu_args.restart_policy,dpu_iterations,dpu_args.min_thresh_hold);
  if(dpu_ret == SAT)
  {
    printf("[DPU] SOLVED using ");
    switch (dpu_args.restart_policy)
    {
    case FIXED:
       printf("FIXED RESTART\n");
      break;
    case DEFAULT:
       printf("DEFAULT RESTART\n");
      break; 
    case RANDOM:
       printf("RANDOM RESTART\n");
      break; 
    default:
      break;
    }
    show_result(dpu_solver);
    return SAT;
  }
  if(dpu_ret == UNSAT)
  { 
    printf("[DPU] SOLVED using ");
    switch (dpu_args.restart_policy)
    {
    case FIXED:
       printf("FIXED RESTART\n");
      break;
    case DEFAULT:
       printf("DEFAULT RESTART\n");
      break;
    case RANDOM:
       printf("RANDOM RESTART\n");
      break;  
    default:
      break;
    }
    return UNSAT;
  }
*/
 //printf("relaunch flag %d\n",relaunch);
  //populate_solver_context(&dpu_solver);
  //if(first == 0)
  //{
   
    //for (int j = 0; j < 10; j++) 
    //{
      //assign_decision(&dpu_solver,(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
      //printf("%d ",(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
    //}
    //first = 1;
  //}
  //else
  //{
    //load another solver :
    // assign decisions
    
    
    //reset
    //assign new starting point
    //assign new 
    /*reset_solver(&dpu_solver);
    for (int j = 0; j < dpu_assign_size; j++) 
    {
      assign_decision(&dpu_solver,dpu_assigns[j]);
      printf("%d ",dpu_assigns[j]);
    }
    printf("\n");
    printf("%d\n",dpu_assign_size);*/
    //if(dpu_to_assign_size > 10)
      //dpu_to_assign_size = 10;
    //for(int j = 0 ; j < dpu_to_assign_size; j++)
    //{
      //assign_decision(&dpu_solver,(dpu_id >> j) & 1 ? dpu_to_assign[j] + 1 : -(dpu_to_assign[j] + 1));
      //printf("%d ",(dpu_id >> j) & 1 ? dpu_to_assign[j] + 1 : -(dpu_to_assign[j] + 1));
    //}
    //printf("\n");
    //show_solver_info_debug(dpu_solver);*/
  //}
  //dpu_ret = solve(&dpu_solver,10);
  //populate_offsets(dpu_DB_offsets,dpu_solver);
  //populate_vars(dpu_vars,dpu_solver);
  //if(dpu_ret == SAT )
  //{
    //show_result(dpu_solver);
  //}
  /*if(dpu_ret == UNSAT)
    relaunch = NO_RELAUNCH;
   * Here else 
   * get assigned lits;
   * store it in assignement_t 
   * get unassigned lits
   * store it in another assignement_t 
  */
