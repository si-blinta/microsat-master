#include <stdio.h>
#include "microsat.h"
#include <mram.h>
#include <alloc.h>
#include "log.h"
#include <barrier.h>       
#include <defs.h>                                                                                 
__mram_noinit int dpu_buffer[MEM_MAX];
__host int first;
__host int dpu_DB_offsets[11];
__host int dpu_vars[11];
__dma_aligned int DB[MEM_MAX];    // Very important : transfers from Wram <> mram need to be dma aligned else it would produce unexpected results
__host int dpu_flag;
void DB_populate(){
  int n64 = (MEM_MAX*sizeof(int))/64;
  int mem_needed = 0;
  if(MEM_MAX % 64 !=0)
  {
    n64+=1;
  }
  mem_needed = 64*n64;
  //printf(D"mem fixed = %d BYTES| we need %d BYTES| %d * 64 BYTES\n",mem_fixed*sizeof(int),mem_needed,n64);
  if(mem_needed > 2048)
  {
    int n2048 = mem_needed/2048;
    for(int i = 0; i < (mem_needed/2048);i++)
    {
      //printf(D"index %d | mram read %d\n",(i*2048/sizeof(int)),2048);  
      mram_read(dpu_buffer+(i*2048/sizeof(int)),DB+(i*2048/sizeof(int)),2048);
    }
    if(mem_needed % 2048 !=0)
    {
      //printf(D"index %d | mram read %d\n",n2048*2048/sizeof(int),mem_needed%2048);
      mram_read(dpu_buffer+n2048*2048/sizeof(int),DB+n2048*2048/sizeof(int),mem_needed%2048);
    }
  }
  else 
  {
    //printf(D"mram read %d\n",mem_needed);
    mram_read(dpu_buffer,DB,mem_needed);
  }
}

void populate_solver_context(struct solver *dpu_solver)
{ 
  //log_message(LOG_LEVEL_INFO,"populating solver context");
  DB_populate();                        // Dont forget that this function takes the number of INTS in the array.
  dpu_solver->DB = DB;
  dpu_solver->nVars = dpu_vars[0];
  dpu_solver->nClauses = dpu_vars[1];
  dpu_solver->mem_used = dpu_vars[2];
  dpu_solver->mem_fixed = dpu_vars[3];
  dpu_solver->maxLemmas = dpu_vars[4];
  dpu_solver->nLemmas = dpu_vars[5];
  dpu_solver->nConflicts = dpu_vars[6];
  dpu_solver->fast = dpu_vars[7];
  dpu_solver->slow = dpu_vars[8];
  dpu_solver->head = dpu_vars[9];
  dpu_solver->res = dpu_vars[10];
  dpu_solver->model = dpu_solver->DB + dpu_DB_offsets[0];
  dpu_solver->next = dpu_solver->DB + dpu_DB_offsets[1];
  dpu_solver->prev = dpu_solver->DB + dpu_DB_offsets[2];
  dpu_solver->buffer = dpu_solver->DB + dpu_DB_offsets[3];
  dpu_solver->reason = dpu_solver->DB + dpu_DB_offsets[4];
  dpu_solver->falseStack = dpu_solver->DB + dpu_DB_offsets[5];
  dpu_solver->forced = dpu_solver->DB + dpu_DB_offsets[6];
  dpu_solver->processed = dpu_solver->DB + dpu_DB_offsets[7];
  dpu_solver->assigned = dpu_solver->DB + dpu_DB_offsets[8];
  dpu_solver->falses = dpu_solver->DB + dpu_DB_offsets[9];
  dpu_solver->first = dpu_solver->DB + dpu_DB_offsets[10];
}
int main()
{
  struct solver dpu_solver;
  if(first == 0)
    populate_solver_context(&dpu_solver);
  //if(dpu_flag == STOPPED)
  dpu_flag = solve(&dpu_solver,1);
  //dpu_solver.DB[0]=0;
  if(dpu_flag == SAT )
    log_message(LOG_LEVEL_INFO,"SAT");
  if(dpu_flag == UNSAT )
    log_message(LOG_LEVEL_INFO,"UNSAT");
  else
    log_message(LOG_LEVEL_INFO,"STOPPED");
  show_result(dpu_solver);
  first++;
  return 0;
}