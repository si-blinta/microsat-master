#include <stdio.h>
#include "microsat.h"
#include <mram.h>
#include <alloc.h>
#include "log.h"
#include <barrier.h>       
#include <defs.h>   
__host int dpu_DB_offsets[11];
__host int dpu_vars[11];
__host int dpu_ret;
__host uint32_t dpu_id;
int first;
int relaunch;
void populate_solver_context(struct solver *dpu_solver)
{ 
  log_message(LOG_LEVEL_INFO,"populating solver context");                      
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
  for (int j = 0; j < 10; j++) 
  {
    assign_decision(dpu_solver,(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
    //printf("%d ",(dpu_id >> j) & 1 ? j + 1 : -(j + 1));
  }
}
int main()
{
  //printf("relaunch flag %d\n",relaunch);
  if(relaunch == NO_RELAUNCH)
  {
    return 0;
  }
  struct solver dpu_solver;
  if(first == 0)
  {
    populate_solver_context(&dpu_solver);
  }
  dpu_ret = solve(&dpu_solver,100);
  if(dpu_ret == SAT )
  {
    log_message(LOG_LEVEL_INFO,"SAT");
    show_result(dpu_solver);
  }
  if(dpu_ret == UNSAT )
  {
    log_message(LOG_LEVEL_INFO,"UNSAT");
    relaunch = NO_RELAUNCH;
  }
  else
  {
    log_message(LOG_LEVEL_INFO,"STOPPED");
  }

  first++;
}
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