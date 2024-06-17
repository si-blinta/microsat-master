#include "microsat_host.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
int main(int argc, char **argv)
{
  if (argc < 3)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = atoi(argv[2]); 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  HOST_TOOLS_pure_portfolio(argv[1],set);
  //HOST_TOOLS_divide_and_conquer(argv[1],set);
  /*clock_t start,end;
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
  printf("DPU %lf ms\n",duration);*/
  //print_decision_list(&dpu_solver);
  //sort_variables(&dpu_solver);
  //print_decision_list(&dpu_solver);

}