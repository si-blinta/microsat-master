#include "microsat_host.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
void print_decision_list(struct solver* S) {
    int current = S->head;
    while (current != 0) { // traverse the list until the end of the list is reached
        if(!S->falses[current] && !S->falses[-current] )
        printf("%d (%f) ", current,S->scores[current]); // print the index of the current node
        current = S->prev[current]; // move to the next node
    }
    printf("\n"); // print a newline character at the end
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
  dpu_solver.config.decay_thresh_hold = 1;
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
  printf("%d\n",dpu_solver.nConflicts);

}