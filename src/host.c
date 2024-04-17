#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include "time.h"
#define DPU_BINARY "bin/dpu"
void populate_offsets(uint32_t offsets[11], struct solver S)
{
  log_info("populating offset");
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
}
void generate_and_send(char* filename,struct dpu_set_t set) {
    struct solver comb;
    struct dpu_set_t dpu;
    parse(&comb,filename);
    int offsets[11];
    int vars[11];
    int i, j;
    // Loop through all numbers from 0 to 2^NUM_VARIABLES - 1
      DPU_FOREACH(set,dpu,i){
        // Loop through each variable
        for (j = 0; j < NUM_VARIABLES; j++) {
            // Check if jth bit is set in i
            assign_decision(&comb,(i >> j) & 1 ? j + 1 : -(j + 1));
        }
        populate_vars(vars, comb);
        populate_offsets(offsets,comb);
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_buffer", 0, comb.DB, MEM_MAX * sizeof(int)));
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_vars", 0, vars, 11 * sizeof(int)));
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_DB_offsets", 0, offsets, 11 * sizeof(int)));
        printf("\n");
        for (j = 0; j < NUM_VARIABLES; j++) {
            // Check if jth bit is set in i
            unassign_decision(&comb,(i >> j) & 1 ? j + 1 : -(j + 1));
        }
      }

}
int main(int argc, char **argv)
{
  if (argc < 2)
    abort();
  /*struct dpu_set_t set, dpu;
  uint32_t nb_dpus = 1<<NUM_VARIABLES;
  HOST_TOOLS_allocate_dpus(&set, &nb_dpus);
  HOST_TOOLS_compile(1);
  DPU_ASSERT(dpu_load(set, DPU_BINARY, NULL));
  generate_and_send(argv[1],set);
  DPU_ASSERT(dpu_launch(set, DPU_SYNCHRONOUS));
  DPU_FOREACH(set, dpu)
  {
    DPU_ASSERT(dpu_log_read(dpu, stdout));
  }
  DPU_ASSERT(dpu_free(set));*/
  struct solver s;
  parse(&s ,argv[1]);
  solve(&s);
  show_result(s);

}
  /*
    clock_t start, end;
  double cpu_time_used;
  start = clock();
  generate_and_send(to_copy,argv[1]);
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  printf("Time taken: %f seconds\n", cpu_time_used);*/