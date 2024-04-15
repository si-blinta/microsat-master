#include "hostTools.h"
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
