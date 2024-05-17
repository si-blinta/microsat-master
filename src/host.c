#include "microsat.h"
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
  uint32_t nb_dpus = NB_DPU;//atoi(argv[2]); 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  //HOST_TOOLS_pure_portfolio(argv[1],set);
  HOST_TOOLS_divide_and_conquer(argv[1],set);


}