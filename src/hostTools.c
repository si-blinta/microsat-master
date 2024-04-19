#include "hostTools.h"
#include "microsat.h"
#include "log.h"
void HOST_TOOLS_parse_args(int argc, char **argv, uint32_t *nb_dpus, uint8_t *nb_tasklets, uint32_t *nb_boots, uint32_t *nb_blocks_to_mine)
{
  if (argc < 5)
  {
    fprintf(stderr, "Usage : %s [nb_dpus] [nb_tasklets] [nb_boots] [nb_blocks_to_mine]\n", argv[0]);
    fprintf(stdout, "0 if you want to allocate all available DPUs\n");
    exit(EXIT_FAILURE);
  }
  if (atoi(argv[2]) <= 0 || atoi(argv[2]) > 24 || atoi(argv[1]) < 0 || atoi(argv[1]) > 1280 || atoi(argv[3]) < 0 || atoi(argv[4]) < 0)
  {
    fprintf(stderr, "1 <= nb_tasklets <= 24 | 0 <= nb_dpus <= 1280 | nb_boots > 1 \n");
    exit(EXIT_FAILURE);
  }
  *nb_dpus = (uint32_t)atoi(argv[1]);
  *nb_tasklets = (uint32_t)atoi(argv[2]);
  *nb_boots = (uint32_t)atoi(argv[3]);
  *nb_blocks_to_mine = (uint32_t)atoi(argv[4]);
  if (*nb_dpus == 0)
    *nb_dpus = DPU_ALLOCATE_ALL;
}

void HOST_TOOLS_allocate_dpus(struct dpu_set_t *set, uint32_t *nb_dpus)
{
  DPU_ASSERT(dpu_alloc(*nb_dpus, NULL, set));
  DPU_ASSERT(dpu_get_nr_dpus(*set, nb_dpus));
  printf("ALLOCATED : %u DPUs \n", *nb_dpus);
}

void HOST_TOOLS_send_id(struct dpu_set_t set)
{
  struct dpu_set_t dpu;
  uint32_t id = 0;
  DPU_FOREACH(set, dpu, id)
  {
    DPU_ASSERT(dpu_copy_to(dpu, "dpu_id", 0, &id, sizeof(uint32_t)));
  }
}

void HOST_TOOLS_compile(uint8_t nb_tasklets)
{
  char command[100];
  sprintf(command, "make dpu NB_TASKLETS=%d", nb_tasklets);
  system(command);
}
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
void HOST_TOOLS_portfolio_launch(char* filename,struct dpu_set_t set) {
    struct solver comb;
    struct dpu_set_t dpu;
    parse(&comb,filename);
    int offsets[11];
    int vars[11];
    int first = 0;
    int flag  = 1;
    int i, j;
    // Loop through all numbers from 0 to 2^NUM_VARIABLES - 1
      DPU_FOREACH(set,dpu,i){
        // Loop through each variable
        for (j = 0; j < NUM_VARIABLES; j++) {
            printf("assign %d ",(i >> j) & 1 ? j + 1 : -(j + 1));
            // Check if jth bit is set in i
            //assign_decision(&comb,(i >> j) & 1 ? j + 1 : -(j + 1));
        }
        /*populate_vars(vars, comb);
        populate_offsets(offsets,comb); 
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_buffer", 0, comb.DB, MEM_MAX * sizeof(int)));
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_vars", 0, vars, 11 * sizeof(int)));
        DPU_ASSERT(dpu_copy_to(dpu, "dpu_DB_offsets", 0, offsets, 11 * sizeof(int)));
        DPU_ASSERT(dpu_copy_to(dpu, "first", 0, &first,sizeof(int)));
        //DPU_ASSERT(dpu_copy_to(dpu, "dpu_flag", 0, &flag,sizeof(int)));
        //printf("\n");
        for (j = 0; j < NUM_VARIABLES; j++) {
            // Check if jth bit is set in i
            unassign_decision(&comb,(i >> j) & 1 ? j + 1 : -(j + 1));
        }*/
      }

}
void HOST_TOOLS_parallel_transfer(char* shared_buffer,size_t buffer_size,uint32_t nb_of_dpus,struct dpu_set_t set)
{
  dpu_xfer_t xfer;



}