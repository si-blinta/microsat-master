#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include "time.h"
#define DPU_BINARY "bin/dpu"

int main(int argc, char **argv)
{
  if (argc < 2)
    abort();
  clock_t start, end;
  double cpu_time_used;
  struct dpu_set_t set,dpu;
  uint32_t nb_dpus = 1<<NUM_VARIABLES;
  HOST_TOOLS_allocate_dpus(&set, &nb_dpus);
  HOST_TOOLS_compile(1);
  DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
  HOST_TOOLS_portfolio_launch(argv[1],set);
  log_message(LOG_LEVEL_INFO,"Finished transfer");
  DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
  //start = clock();
  //end = clock();
  //cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  
  DPU_FOREACH(set, dpu)
  {
    DPU_ASSERT(dpu_log_read(dpu, stdout));
  }
  DPU_ASSERT(dpu_free(set));
  //printf("DPU Time taken: %f seconds\n", cpu_time_used);;

}
