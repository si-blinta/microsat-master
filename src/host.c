#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
int main(int argc, char **argv)
{
   if (argc < 2)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = 10; 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  HOST_TOOLS_pure_portfolio(argv[1],set);
}