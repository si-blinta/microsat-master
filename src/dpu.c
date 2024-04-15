#include <stdio.h>
#include "microsat.h"
#include <mram.h>
#include <alloc.h>
#include "log.h"
__mram_noinit int dpu_buffer[MEM_MAX];
__host int dpu_DB_offsets[11];
__host int dpu_vars[11];
__dma_aligned int clone[MEM_MAX];
void populate_solver_context(struct solver *dpu_solver)
{
  log_info("populating solver context");
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
  mram_read(dpu_buffer, clone, 2048);
  dpu_solver->DB = clone;
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
  struct solver dpu_solver, test;
  populate_solver_context(&dpu_solver);
  show_solver_info_debug(dpu_solver);
  if (solve(&dpu_solver) == SAT)
  {
    log_result(D"SAT");
  }
  else
    log_result(D"UNSAT");
  show_solver_stats(dpu_solver);
  return 0;
}