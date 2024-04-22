#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#include "portfolio.h"
#define DPU_BINARY "bin/dpu"
static void populate_offsets(uint32_t offsets[11], struct solver S)
{
  //log_message(LOG_LEVEL_INFO,"populating offsets");
  offsets[0] = (int) (S.model - S.DB);
  offsets[1] = (int) (S.next - S.DB);
  offsets[2] = (int) (S.prev - S.DB);
  offsets[3] = (int) (S.buffer - S.DB);
  offsets[4] = (int) (S.reason - S.DB);
  offsets[5] = (int) (S.falseStack - S.DB);
  offsets[6] = (int) (S.forced - S.DB);
  offsets[7] = (int) (S.processed - S.DB);
  offsets[8] = (int) (S.assigned - S.DB);
  offsets[9] = (int) (S.falses - S.DB);
  offsets[10]= (int) (S.first - S.DB);
}
static void populate_vars(uint32_t vars[11], struct solver S)
{
  //log_message(LOG_LEVEL_INFO,"populating vars");
  vars[0] = S.nVars;
  vars[1] = S.nClauses;
  vars[2] = S.mem_used;
  vars[3] = S.mem_fixed;
  vars[4] = S.maxLemmas;
  vars[5] = S.nLemmas;
  vars[6] = S.nConflicts;
  vars[7] = S.fast;
  vars[8] = S.slow;
  vars[9] = S.head;
  vars[10] = S.res;
}
int main(int argc, char **argv)
{
  time_t start,end;
  double duration;
  if (argc < 2)
    abort();
  struct solver master;
  int ret = parse(&master,argv[1]);
  log_message(LOG_LEVEL_INFO,"parsing finished");
  struct dpu_set_t set,dpu;
  uint32_t nb_dpus = 2; 
  int first = 0;
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  int offsets[11];int vars[11];

  populate_offsets(offsets,master);
  populate_vars(vars,master);
  HOST_TOOLS_send_id(set);
  DPU_ASSERT(dpu_broadcast_to(set,"first",0,&first,sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_buffer",0,master.DB,MEM_MAX*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
  
  DPU_FOREACH(set,dpu)
  {
    DPU_ASSERT(dpu_log_read(dpu,stdout));
  }

  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }

  time(&start);
  ret = solve(&master,INT32_MAX);
  time(&end);
  duration = difftime(end,start);

  if(ret == UNSAT)
    log_message(LOG_LEVEL_INFO,"MASTER UNSAT");
  else if(ret == SAT)
    log_message(LOG_LEVEL_INFO,"MASTER SAT");
  else
    log_message(LOG_LEVEL_INFO,"MASTER STOPPED");
  printf("%lf\n",duration);
  show_result(master);
} 