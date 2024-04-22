#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#include "portfolio.h"
#define DPU_BINARY "bin/dpu"
int main(int argc, char **argv)
{
  clock_t start,end;
  double duration;
  if (argc < 2)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = 1024; 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  HOST_TOOLS_portfolio_launch(argv[1],set);
  struct solver master;
  int ret = parse(&master,argv[1]);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"HOST parsing UNSAT");
    exit(0);
  }
  start = clock();
  ret = solve(&master,INT_MAX);
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;

  if(ret == UNSAT)
    log_message(LOG_LEVEL_INFO,"HOST UNSAT");
  else if(ret == SAT)
    log_message(LOG_LEVEL_INFO,"HOST SAT");
  else
    log_message(LOG_LEVEL_INFO,"HOST STOPPED");
  printf("%lf ms\n",duration);
  show_result(master);
} 