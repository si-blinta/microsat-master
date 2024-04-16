#include "microsat.h"
//#include "hostTools.h"
#include "log.h"
#define DPU_BINARY "bin/dpu"
/*void populate_offsets(uint32_t offsets[11], struct solver S)
{
  log_info("populating offset");
  offsets[0] = S.model - S.DB;
  offsets[1] = S.next - S.DB;
  offsets[2] = S.prev - S.DB;
  offsets[3] = S.buffer - S.DB;
  offsets[4] = S.reason - S.DB;
  offsets[5] = S.falseStack - S.DB;
  offsets[6] = S.forced - S.DB;
  offsets[7] = S.processed - S.DB;
  offsets[8] = S.assigned - S.DB;
  offsets[9] = S.falses - S.DB;
  offsets[10] = S.first - S.DB;
}
void populate_vars(uint32_t vars[11], struct solver S)
{
  log_info("populating vars");
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
}*/
int main(int argc, char **argv)
{
  if (argc < 2)
    abort();
  struct solver S, tmp;
  parse(&S, argv[1]);
  show_solver_info_debug(S);
  //show_solver_stats(S);
  solve(&S);
  //show_solver_info_debug(S);
  //show_solver_stats(S);
  //propagate(&S);
  show_solver_info_debug(S);
  //show_solver_info_debug(S);
  /*uint32_t offsets[11];
  int vars[11];
  populate_offsets(offsets, S);
  populate_vars(vars, S);
  uint32_t nb_dpus = 1;
  struct dpu_set_t set, dpu;
  HOST_TOOLS_allocate_dpus(&set, &nb_dpus);
  HOST_TOOLS_compile(1);
  DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
  // Send data base, the size must be 4 bytes aligned.
  DPU_ASSERT(dpu_broadcast_to(set, "dpu_buffer", 0, S.DB, MEM_MAX * sizeof(int), DPU_XFER_DEFAULT));
  // Send the variables
  DPU_ASSERT(dpu_broadcast_to(set, "dpu_vars", 0, vars, 11 * sizeof(int), DPU_XFER_DEFAULT));
  // Send the offsets of pointers in DB
  DPU_ASSERT(dpu_broadcast_to(set, "dpu_DB_offsets", 0, offsets, 11 * sizeof(uint32_t), DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));

  DPU_FOREACH(set, dpu)
  {
    DPU_ASSERT(dpu_log_read(dpu, stdout));
  }
  DPU_ASSERT(dpu_free(set));
  if (solve(&S) == SAT)
  {
    log_result(H"SAT");
   
  }
  else
    log_result(H"UNSAT");
  show_solver_stats(S);*/
}
