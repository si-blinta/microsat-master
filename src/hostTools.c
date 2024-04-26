#include "hostTools.h"
#include "microsat.h"
#include "log.h"
#include <time.h>
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
bool compareClauses(int clause1[], int size1, int clause2[], int size2) {
    if (size1 != size2) {
        return false; // Clauses have different sizes, so they are not equal
    }
    for (int i = 0; i < size1; i++) {
        if (clause1[i] != clause2[i]) {
            return false; // Clauses differ at some position, so they are not equal
        }
    }
    return true; // Clauses are equal
}
void HOST_TOOLS_portfolio_launch(char* filename,struct dpu_set_t set) {
  int learnt_clauses[MAX_LEARNT_CLAUSES][MAX_CLAUSE_SIZE];
  int learnt_clause_sizes[MAX_LEARNT_CLAUSES] = {0}; // Track the size of learned clauses for each DPU
  int learnt_clause_count = 0; // Track the total count of learned clauses
  struct solver dpu_solver;
  int ret = parse(&dpu_solver,filename);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  struct dpu_set_t dpu;
  uint32_t nb_dpus = 1024; 
  int first = 0;
  int dpu_flag = UNSAT;
  int unsat_cpt = 0;
  int nb_boot = 0;
  int sat = 0;
  int offsets[11];int vars[11];
  populate_offsets(offsets,dpu_solver);
  populate_vars(vars,dpu_solver);
  HOST_TOOLS_send_id(set);
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_buffer",0,dpu_solver.DB,MEM_MAX*sizeof(int),DPU_XFER_DEFAULT));
  clock_t start,end;
  double duration;
  start = clock();
  while(unsat_cpt < 1024 && !sat)
  {
    int learnt_clauses_per_launch = 0;
    unsat_cpt = 0;
    log_message(LOG_LEVEL_INFO,"Launching");
    DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));  // If error : halt(), meaning that no memory is left.
    DPU_FOREACH(set,dpu,nb_boot)
    {
      //DPU_ASSERT(dpu_log_read(dpu,stdout));
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_flag",0,&dpu_flag,sizeof(int)));
      if(dpu_flag == SAT)
      {
        DPU_ASSERT(dpu_copy_from(dpu,"dpu_buffer",0,dpu_solver.DB,MEM_MAX*sizeof(int)));
        sat = 1;
        break;
      }
      if(dpu_flag == UNSAT)
      {
        unsat_cpt++;
        if(nb_boot > 0 )
          continue;
      } 
      //Add learned clauses.
      DPU_ASSERT(dpu_copy_from(dpu,"mem_used",0,&dpu_solver.mem_used,sizeof(int)));
      DPU_ASSERT(dpu_copy_from(dpu,"mem_fixed",0,&dpu_solver.mem_fixed,sizeof(int)));
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_buffer",0,dpu_solver.DB,MEM_MAX*sizeof(int)));
      int i = dpu_solver.mem_fixed;
      while (i < dpu_solver.mem_used) 
      {
        i += 2; // Move to the next element after the watch pointers
        int clause_size = 0;
        while (i < dpu_solver.mem_used && dpu_solver.DB[i] != 0)
        {
          clause_size++;
          i++;
        }
        //todo: improve clause filtering
        if(clause_size > 0 &&  dpu_solver.DB[i-clause_size]!= 0 && clause_size <= MAX_CLAUSE_SIZE)
        {
          bool duplicate = false;
          // Check for duplicates
          for (int j = 0; j < learnt_clause_count; j++) {
            if (compareClauses(learnt_clauses[j], learnt_clause_sizes[j], dpu_solver.DB + i - clause_size, clause_size)) 
            {
              duplicate = true;
              break;
            }
          }
          if (!duplicate)
          {
            if (learnt_clause_count < MAX_LEARNT_CLAUSES) 
            {
            // Copy the learned clause to the array
              memcpy(learnt_clauses[learnt_clause_count], dpu_solver.DB + i - clause_size, clause_size * sizeof(int));
              learnt_clause_sizes[learnt_clause_count] = clause_size; // Store the size of the learned clause
              learnt_clause_count++; // Increment the total count of learned clauses
              learnt_clauses_per_launch++;
            }
          }
        }  
        i++;
        }
    }
    //printf("unsat %d\n",unsat_cpt);
    //printf("Newly learned clauses %d\n", learnt_clauses_per_launch);
    if(learnt_clauses_per_launch != 0)
    { 
      DPU_ASSERT(dpu_broadcast_to(set, "learnt_clauses", 0, learnt_clauses + (learnt_clause_count - learnt_clauses_per_launch), learnt_clauses_per_launch * sizeof(int), DPU_XFER_DEFAULT));
      DPU_ASSERT(dpu_broadcast_to(set, "learnt_clause_sizes", 0, learnt_clause_sizes + (learnt_clause_count - learnt_clauses_per_launch), learnt_clauses_per_launch * sizeof(int), DPU_XFER_DEFAULT));
      DPU_ASSERT(dpu_broadcast_to(set, "learnt_clause_count", 0, &learnt_clauses_per_launch, sizeof(int), DPU_XFER_DEFAULT));
    }
  }
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  printf("%lf ms\n",duration);
  if(sat)
  {
    log_message(LOG_LEVEL_INFO,"DPU SAT");
    show_result(dpu_solver);
  }
  else
     log_message(LOG_LEVEL_INFO,"DPU UNSAT");
  dpu_free(set);
}
void HOST_TOOLS_parallel_transfer(char* shared_buffer,size_t buffer_size,uint32_t nb_of_dpus,struct dpu_set_t set)
{
  dpu_xfer_t xfer;
}
void HOST_TOOLS_launch(char* filename, struct dpu_set_t set)
{
  struct solver dpu_solver;
  int ret = parse(&dpu_solver,filename);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  struct dpu_set_t dpu;
  int dpu_flag = UNSAT;
  int offsets[11];int vars[11];
  populate_offsets(offsets,dpu_solver);
  populate_vars(vars,dpu_solver);
  log_message(LOG_LEVEL_INFO,"Broadcasting");
  HOST_TOOLS_send_id(set);
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_buffer",0,dpu_solver.DB,MEM_MAX*sizeof(int),DPU_XFER_DEFAULT));
  log_message(LOG_LEVEL_INFO,"Launching");
  DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
  DPU_FOREACH(set,dpu)
  {
    dpu_log_read(dpu,stdout);
    DPU_ASSERT(dpu_copy_from(dpu,"dpu_flag",0,&dpu_flag,sizeof(int)));
    if(dpu_flag == SAT)
    {
      log_message(LOG_LEVEL_INFO,"DPU SAT");
      dpu_log_read(dpu,stdout);
      break;
    }
    if(dpu_flag == STOPPED)
    {
      log_message(LOG_LEVEL_INFO,"STOPPED");
    }
  }
}