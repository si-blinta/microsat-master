#include "microsat_host.h"
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
    int current = S->head;
    while (current != 0) { // traverse the list until the end of the list is reached
        if(!S->falses[current] && !S->falses[-current] )
        printf("%d (%f) ", current,S->scores[current]); // print the index of the current node
        current = S->prev[current]; // move to the next node
    }
    printf("\n"); // print a newline character at the end
}
void decay(struct solver *S,float factor);
int solve_luby(struct solver* S,int stop_it,int luby_param)
{
  int conflicts = 0;
  int luby_index = 0;
  int decision = S->head;
  int decay_thresh_hold = 2;
  S->res = 0;
  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;
    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if(conflicts % decay_thresh_hold == 0)
          decay(S,0.99);
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
    /*printf("VSIDS %d score %f\n",decision,S->scores[decision]); 
    print_decision_list(S);
    printf("#####################\n");*/
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
  clock_t start,end;
  double duration;
  struct solver dpu_solver;
  start = clock();
  int ret = parse(&dpu_solver,argv[1]);
  end = clock();
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  ret = solve_luby(&dpu_solver,INT32_MAX,2);
  //ret = solve(&dpu_solver,INT32_MAX);
  if(ret == SAT)
  {
    log_message(LOG_LEVEL_INFO,"SAT");
    //show_result(dpu_solver);
  }
  
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  printf("DPU %lf ms\n",duration);

}