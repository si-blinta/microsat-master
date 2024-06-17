#include "hostTools.h"
#include "microsat_host.h"
#include "log.h"
#include <time.h>
#include "utils.h"
#define MAX_RANDOM_THRESHHOLD 100

#define GEOMETRIC_FACTOR 1.2

#define ADAPTIVE_FACTOR 1.5
#define PROGRESS_THRESHOLD 0.95
#define MIN_THRESHOLD 2000

#define FIXED_THRESHOLD 1000
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
static void populate_offsets(int offsets[13], struct solver S)
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
  offsets[11] = (int)((int*)(S.scores) - (S.DB));
  offsets[12]= (int) (S.decision_level - S.DB);
}
static void populate_vars(int vars[12], struct solver S)
{
    log_message(LOG_LEVEL_INFO, "Populating vars");
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
    vars[11] = S.decision_counter;
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
  int dpu_ret= UNSAT;
  int offsets[13];int vars[12];
  populate_offsets(offsets,dpu_solver);
  populate_vars(vars,dpu_solver);
  log_message(LOG_LEVEL_INFO,"Broadcasting");

  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,11*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,DPU_MRAM_HEAP_POINTER_NAME,0,dpu_solver.DB,MEM_MAX*sizeof(int),DPU_XFER_DEFAULT));
  log_message(LOG_LEVEL_INFO,"Launching");
  DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
  log_message(LOG_LEVEL_DEBUG,"AFTER LAUNCHING");
  DPU_FOREACH(set,dpu)
  {
    DPU_ASSERT(dpu_copy_from(dpu,"dpu_ret",0,&dpu_ret,sizeof(int)));
    if(dpu_ret== SAT)
    {
      log_message(LOG_LEVEL_INFO,"DPU SAT");
      dpu_log_read(dpu,stdout);
      break;
    }
    if(dpu_ret== STOPPED)
    {
      log_message(LOG_LEVEL_INFO,"STOPPED");
    }
  }
}
void HOST_TOOLS_divide_and_conquer_old(char* filename, struct dpu_set_t set)
{ 
  dpu_cntx states[NB_DPU];
  int id = 0;
  struct dpu_set_t dpu;
  DPU_FOREACH(set,dpu,id)
  {
    states[id].state = BUSY;
    states[id].dpu   = dpu;
  }

  struct solver dpu_solver,model;
  int ret = parse(&dpu_solver,filename);
  initCDCL(&model,dpu_solver.nVars,dpu_solver.nClauses);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  int dpu_ret = UNSAT;
  int unsat_cpt = 0;
  int sat = 0;
  uint32_t offsets[13];uint32_t vars[12];
  populate_offsets(offsets,dpu_solver);
  populate_vars(vars,dpu_solver);
  log_message(LOG_LEVEL_INFO,"Broadcasting");
  HOST_TOOLS_send_id(set);
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,12*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,13*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,DPU_MRAM_HEAP_POINTER_NAME,0,dpu_solver.DB,roundup(dpu_solver.mem_used,8)*sizeof(int),DPU_XFER_DEFAULT));
  clock_t start,end;
  double duration;
  start = clock();
  while(unsat_cpt < NB_DPU && !sat)
  {
    id = 0;
    unsat_cpt = 0;
    sat = 0;
    log_message(LOG_LEVEL_INFO,"Launching");
    DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
    
    DPU_FOREACH(set,dpu,id)
    { 
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_ret",0,&dpu_ret,sizeof(int)));
      if(dpu_ret == SAT)
      {
        log_message(LOG_LEVEL_INFO,"DPU SAT");
        dpu_log_read(dpu,stdout);
        sat = 1;
        break;
      }
      if(dpu_ret== STOPPED)
      {
        //log_message(LOG_LEVEL_INFO,"STOPPED");

      }
      if(dpu_ret == UNSAT)
      {
        states[id].state = IDLE;
        unsat_cpt++;
      }
    }
  int busy = 0;
  for(int i = 0 ; i < NB_DPU;i++)
  {
    if(states[i].state == busy)
      busy++;
  }
  printf("BUSY %d | IDLE %d\n",busy,NB_DPU-busy);
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  printf("DPU %lf ms\n",duration);
  if(!sat)
    log_message(LOG_LEVEL_INFO,"DPU UNSAT");
}
} 
void HOST_TOOLS_pure_portfolio(char* filename, struct dpu_set_t set)
{
  struct dpu_set_t dpu;
  struct solver dpu_solver;
  int ret = parse(&dpu_solver,filename);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  log_message(LOG_LEVEL_INFO,"parsing finished");
  int dpu_ret;
  int finish = 0;
  int offsets[13];int vars[12];
  populate_offsets(offsets,dpu_solver);
  populate_vars(vars,dpu_solver);
  log_message(LOG_LEVEL_INFO,"Broadcasting");
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_vars",0,vars,12*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"dpu_DB_offsets",0,offsets,13*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,DPU_MRAM_HEAP_POINTER_NAME,0,dpu_solver.DB,MEM_MAX*sizeof(int),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"config",0,&dpu_solver.config,sizeof(config_t)-sizeof(float*)-2*sizeof(int*),DPU_XFER_DEFAULT));
  DPU_FOREACH(set,dpu)
  { //todo get good combinations
    dpu_solver.config.br_p  = rand() % 2;
    dpu_solver.config.rest_p  = rand() % 4;
    dpu_solver.config.reduce_p  = rand() % 3;
    
    DPU_ASSERT(dpu_copy_to(dpu,"config",0,&dpu_solver.config,sizeof(config_t)-sizeof(float*)-2*sizeof(int*)));
  }
  clock_t start,end;
  double duration;
  start = clock();
  while(!finish)
  {
    int iterations = rand()%1000 + 500;   // luby
    DPU_ASSERT(dpu_broadcast_to(set,"dpu_iterations",0,&iterations,sizeof(int),DPU_XFER_DEFAULT));
    log_message(LOG_LEVEL_INFO,"Launching with %d iterations",iterations);
    DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
    DPU_FOREACH(set,dpu)
    { 
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_ret",0,&dpu_ret,sizeof(int)));
      if(dpu_ret == SAT)
      {
        DPU_ASSERT(dpu_copy_from(dpu,"config",0,&dpu_solver.config,sizeof(int)));
        log_message(LOG_LEVEL_INFO,"DPU SAT");
        log_config(dpu_solver.config);
        finish = 1;
        break;
      }
      else if(dpu_ret == UNSAT)
      {
        log_message(LOG_LEVEL_INFO,"DPU UNSAT");
        finish = 1;
        log_config(dpu_solver.config);
        break;
      }
    }
  }
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  log_message(LOG_LEVEL_INFO,"DPU %lf ms",duration);
}
/**
 * #if SHARING
       //Add learned clauses.
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_mem_used",0,&dpu_solver.mem_used,sizeof(int)));
      DPU_ASSERT(dpu_copy_from(dpu,"dpu_last_mem_used",0,&dpu_solver.mem_fixed,sizeof(int)));
      if(dpu_solver.mem_used - dpu_solver.mem_fixed > 0)
      {
        //DPU_ASSERT(dpu_copy_from(dpu,DPU_MRAM_HEAP_POINTER_NAME,rounddown(dpu_solver.mem_fixed,8)*sizeof(int),dpu_solver.DB+rounddown(dpu_solver.mem_fixed,8),roundup((dpu_solver.mem_used-dpu_solver.mem_fixed),8)*sizeof(int)));
        //TODO SEND ONLY CLAUSES : I HAVE A WEIRD BUG (SOME CLAUSE MEMBERS ARE SWAPPED)
        DPU_ASSERT(dpu_copy_from(dpu,DPU_MRAM_HEAP_POINTER_NAME,0,dpu_solver.DB,roundup(dpu_solver.mem_used,8)*sizeof(int)));
      }
      else
        continue;
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
        if (learnt_clause_count < MAX_LEARNT_CLAUSES && clause_size < MAX_CLAUSE_SIZE) 
        {
        // Copy the learned clause to the array
        memcpy(learnt_clauses[learnt_clause_count], dpu_solver.DB + i - clause_size, clause_size * sizeof(int));
        learnt_clause_sizes[learnt_clause_count] = clause_size; // Store the size of the learned clause
        printf("learned a clause : size %d \n",clause_size);
        for(int i = 0 ; i < learnt_clause_sizes[learnt_clause_count] ; i++)
          printf("%d ",learnt_clauses[learnt_clause_count][i]);
        printf("\n");
        learnt_clause_count++; // Increment the total count of learned clauses
        learnt_clauses_per_launch++;
        }
        i++;
        }
    }
     if(learnt_clauses_per_launch != 0)
    { 
    //SEND LEARNED CLAUSES :
    DPU_ASSERT(dpu_broadcast_to(set, "dpu_lc", 0, learnt_clauses + (learnt_clause_count - learnt_clauses_per_launch), learnt_clauses_per_launch * sizeof(int), DPU_XFER_DEFAULT));
    DPU_ASSERT(dpu_broadcast_to(set, "dpu_lc_sizes", 0, learnt_clause_sizes + (learnt_clause_count - learnt_clauses_per_launch), learnt_clauses_per_launch * sizeof(int), DPU_XFER_DEFAULT));
    DPU_ASSERT(dpu_broadcast_to(set, "dpu_lc_count", 0, &learnt_clauses_per_launch, sizeof(int), DPU_XFER_DEFAULT));
#endif //SHARING
*/

void log_config(config_t config)
{
    printf("Branching heurstic used ");
    switch (config.br_p)
    {
    case BR_VMTF:
        printf("VMTF : Variable move to the front\n");
        break;
    case BR_VSIDS:
        printf("VSIDS : Variable state decaying sum : decay factor ->%f |decay thresh hold -> %d conflicts\n",config.decay_factor,config.decay_thresh_hold);
        break;
    default:
        break;
    }
    printf("Restart policy used ");
    switch (config.rest_p)
    {
    case REST_DEFAULT:
        printf("DEFAULT : Exponential moving averages ( slow - fast )\n");
        break;
    case REST_GEO:
        printf("GEOMETRIC : reason ->%f | thresh hold -> %d conflicts\n",config.geo_factor,config.geo_max);
        break;
    case REST_LUBY:
        printf("LUBY: Luby's Series : base ->%d\n",config.luby_base);
        break;
    default:
        break;
    }
    printf("Clause suppression mecanism used ");
    switch (config.rest_p)
    {
    case RED_DEFAULT:
        printf("DEFAULT : Clauses that have literals unassigned < %d\n",REDUCE_LIMIT);
        break;
    case RED_SIZE:
        printf("SIZE : Clauses that have size > %d \n",config.clause_size);
        break;
    case RED_LBD:
        printf("LBD: Clauses with LBD > %d\n",config.max_lbd);
        break;
    default:
        break;
    }
    printf("Learnt clauses with LBD <= 2 and clauses with size = 2 are always conserved\n");


}