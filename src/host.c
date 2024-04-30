#include "microsat.h"
//#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
#define MIN(a, b) ((a) < (b) ? (a) : (b))
int main(int argc, char **argv)
{
   if (argc < 2)
    abort();
  clock_t start,end;
  double duration;
  srand(time(NULL));
  /*struct dpu_set_t set;
  uint32_t nb_dpus = NB_DPU; 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  HOST_TOOLS_divide_and_conquer(argv[1],set);*/
  struct solver master;
  int ret = STOPPED;
  parse(&master,argv[1]);
  start = clock();
  while(ret == STOPPED )
  {
    ret = solve_portfolio(&master,RANDOM,rand()%10000+1000);
  }
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  printf("%lf\n",duration);
  switch (ret)
  {
  case UNSAT:
    log_message(LOG_LEVEL_INFO,"UNSAT");
    break;
  case SAT:
    log_message(LOG_LEVEL_INFO,"SAT");
    picosat_proof(master);
    show_result(master);
    show_solver_stats(master);
  default:
    break;
  }
}