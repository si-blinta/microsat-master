#include <stdio.h>
#include "microsat.h"   
#include "utils.h"
#include "mram.h"
/**
 * SOLVER DATA TRANSFER
*/
__host int dpu_DB_offsets[11];
__host int dpu_vars[11];
/**
 * RETURN VALUE
*/
__host int dpu_ret;
__host int dpu_id;

/**
 * PORTFOLIO ARGS
*/
__host portfolio_args dpu_args;
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
}
/**
 * Initialization flag
*/
int first;
/**
 * Iterations
*/
__host int dpu_iterations;
struct solver dpu_solver;
int main()
{

  if(first == 0)
  {
    populate_solver_context(&dpu_solver);
    first = 1;
  }
  dpu_ret = solve_portfolio(&dpu_solver,dpu_args.restart_policy,10,dpu_args.factor,dpu_args.min_thresh_hold);
  if(dpu_ret == SAT)
  {
    printf("SOLVED using %d\n",dpu_args.restart_policy);
    show_result(dpu_solver);
  }

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
/**
 * DIVIDE CONQUER 
 *  struct solver dpu_solver;
  
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
*/