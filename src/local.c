#include "microsat_host.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define SIZE_OF_CONFIG_T(nVars) \
    (4 * 14 + 8 * 3 + ((nVars + 1) * 4) * 2 + (nVars + 1) * 4)

void print_decision_list(struct solver* S) {
    int current = S->head;
    while (current != 0) { // traverse the list until the end of the list is reached
        if(!S->falses[current] && !S->falses[-current] )
        printf("%d (%f) ", current,S->scores[current]); // print the index of the current node
        current = S->prev[current]; // move to the next node
    }
    printf("\n"); // print a newline character at the end
}
void log_config(config_t config)
{
    printf("Branching heurstic used ");
    switch (config.br_p)
    {
    case BR_VMTF:
        printf("VMTF : Variable move to the front\n");
        break;
    case BR_VSIDS:
        printf("VSIDS : Variable state decaying sum : decay factor ->%f |decay thresh hold -> %d conflicts\n",config.decay_factor,config.decay_thresh_hold);
        break;
    default:
        break;
    }
    printf("Restart policy used ");
    switch (config.rest_p)
    {
    case REST_DEFAULT:
        printf("DEFAULT : Exponential moving averages ( slow - fast )\n");
        break;
    case REST_GEO:
        printf("GEOMETRIC : reason ->%f | thresh hold -> %d conflicts\n",config.geo_factor,config.geo_max);
        break;
    case REST_LUBY:
        printf("LUBY: Luby's Series : base ->%d\n",config.luby_base);
        break;
    default:
        break;
    }
    printf("Clause suppression mecanism used ");
    switch (config.rest_p)
    {
    case RED_DEFAULT:
        printf("DEFAULT : Clauses that have literals unassigned < %d\n",REDUCE_LIMIT);
        break;
    case RED_SIZE:
        printf("SIZE : Clauses that have size > %d \n",config.clause_size);
        break;
    case RED_LBD:
        printf("LBD: Clauses with LBD > %d\n",config.max_lbd);
        break;
    default:
        break;
    }
    printf("Learnt clauses with LBD <= 2 and clauses with size = 2 are always conserved\n");


}
int main(int argc, char **argv)
{
  clock_t start,end;
  double duration;
  struct solver dpu_solver;
  int ret = parse(&dpu_solver,argv[1]);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  /*set_solver_rest(&dpu_solver,REST_LUBY);
  dpu_solver.config.luby_base = 4;*/
  //set_solver_rest(&dpu_solver,REST_ARITH);
  //dpu_solver.config.geo_factor = 1.1;
  dpu_solver.config.br_p = BR_VSIDS;
  //dpu_solver.config.decay_thresh_hold = 1;
  //dpu_solver.config.reduce_p = RED_VSIDS;
  //dpu_solver.config.clause_size = 20;
  //dpu_solver.config.clause_score_ratio = 600;
  //dpu_solver.config.reduce_p = RED_LBD;
  //dpu_solver.config.max_lbd   = 10;
  start = clock();
  ret = solve(&dpu_solver,INT32_MAX);
  end = clock();

  if(ret == SAT)
  {
    log_message(LOG_LEVEL_INFO,"SAT");
    show_result(dpu_solver);
  }
   if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"UNSAT");

  }
  
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  printf("DPU %lf ms\n",duration);
  log_config(dpu_solver.config);


}