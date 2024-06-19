#include <stdio.h>
#include "microsat_dpu.h"   
#include "utils.h"
#include "mram.h"
#include <alloc.h>
void log_config(config_t config)
{
    printf("c Branching heurstic used ");
    switch (config.br_p)
    {
    case BR_VMTF:
        printf("c VMTF : Variable move to the front\n");
        break;
    case BR_VSIDS:
        printf("c VSIDS : Variable state decaying sum : decay factor ->%f |decay thresh hold -> %d conflicts\n",config.decay_factor,config.decay_thresh_hold);
        break;
    case BR_CHB:
        printf("c CHB : Conflict history based branching \n");
        break;
    default:
        break;
    }
    printf("c Restart policy used ");
    switch (config.rest_p)
    {
    case REST_DEFAULT:
        printf("c DEFAULT : Exponential moving averages ( slow - fast )\n");
        break;
    case REST_GEO:
        printf("c GEOMETRIC : reason ->%f | thresh hold -> %d conflicts\n",config.geo_factor,config.geo_max);
        break;
    case REST_LUBY:
        printf("c LUBY: Luby's Series : base ->%d\n",config.luby_base);
        break;
    case REST_ARITH:
        printf("c ARITHMETIC: Arithmetic Series : reason ->%d\n",config.arith_reason);
        break;
    default:
        break;
    }
    printf("c Clause suppression mecanism used ");
    switch (config.rest_p)
    {
    case RED_DEFAULT:
        printf("c DEFAULT : Clauses that have literals unassigned < %d\n",REDUCE_LIMIT);
        break;
    case RED_SIZE:
        printf("c SIZE : Clauses that have size > %d \n",config.clause_size);
        break;
    case RED_LBD:
        printf("c LBD: Clauses with LBD > %d\n",config.max_lbd);
        break;
    default:
        break;
    }
    printf("c Learnt clauses with LBD <= 2 and clauses with size = 2 are always conserved\n");


}
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