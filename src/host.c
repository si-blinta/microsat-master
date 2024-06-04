#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
int power(int base, int exponent) {
    double result = 1.0;
    for(int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}
int luby(int y, int x) {

    // Find the finite subsequence that contains index 'x', and the
    // size of that subsequence:
    int size, seq;
    for(size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1);

    while(size - 1 != x) {
        size = (size - 1) >> 1;
        seq--;
        x = x % size;
    }

    return power(y, seq);
}

void print_decision_list(struct solver* S) {
    while (S->head != 0) { // traverse the list until the end of the list is reached
        printf("%d ", S->head); // print the index of the current node
        S->head = S->prev[S->head]; // move to the next node
    }
    printf("\n"); // print a newline character at the end
}
int solve_luby(struct solver* S,int stop_it,int luby_param)
{
  int conflicts = 0;
  int luby_index = 0;
  int decision = S->head;
  S->res = 0;
  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;
    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      for(int j = 1 ; j < S->nVars+1 ; j++)
      {
          S->scores[j] = S->scores[j]*0;
      }
      if ( conflicts >= luby(luby_param,luby_index) ) {
        //printf("restarted after %d conflicts\n",conflicts);
        S->res = 0;
        luby_index++;
        conflicts = 0;
        restart(S);
        if (S->nLemmas > S->maxLemmas) {
          reduceDB(S, REDUCE_LIMIT);
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
  /*if (argc < 3)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = atoi(argv[2]); 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  HOST_TOOLS_pure_portfolio(argv[1],set);
  //HOST_TOOLS_divide_and_conquer(argv[1],set);*/
  struct solver dpu_solver;
  int ret = parse(&dpu_solver,argv[1]);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  ret = solve_luby(&dpu_solver,INT32_MAX,2);
  if(ret == SAT)
    log_message(LOG_LEVEL_INFO,"SAT");
  for(int i = 1 ; i < dpu_solver.nVars+1;i++)
  {
    printf("score %d = %f\n",i,dpu_solver.scores[i]);
  }
}