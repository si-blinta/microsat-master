#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
int solve_exp(struct solver* S, int stop_it) {
  int restart_threshold = 2;
  int decision = S->head;
  int conflicts = 0;

  S->res = 0;

  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;

    if (propagate(S) == UNSAT) {
      return UNSAT;
    }
    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if (conflicts >= restart_threshold) {
        printf("RESTARTED AFTER %d CONFLICTS\n", conflicts);
        S->res = 0;
        conflicts = 0;
        restart(S);
        restart_threshold<<=1;
      if (S->nLemmas > S->maxLemmas) {
        reduceDB(S, 2);
      }
      }
    }

    while (S->falses[decision] || S->falses[-decision]) {
      decision = S->prev[decision];
    }

    if (decision == 0) {
      return SAT;
    }

    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
}
int main(int argc, char **argv)
{
  if (argc < 2)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = 1024  ; 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  HOST_TOOLS_pure_portfolio(argv[1],set);
}