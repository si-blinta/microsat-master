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
    dpu_solver.config.br_p  = BR_CHB;
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
        dpu_log_read(dpu,stdout);
        finish = 1;
        break;
      }
      else if(dpu_ret == UNSAT)
      {
        log_message(LOG_LEVEL_INFO,"DPU UNSAT");
        finish = 1;
        dpu_log_read(dpu,stdout);
        break;
      }
    }
  }
  end = clock();
  duration = (double)(end-start)/CLOCKS_PER_SEC *1000.0;
  log_message(LOG_LEVEL_INFO,"DPU %lf ms",duration);
}