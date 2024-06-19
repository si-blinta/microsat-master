#include <stdio.h>
#include "microsat_dpu.h"   

/**
 * Initialization Data
 */
__host int dpu_DB_offsets[13];
__host int dpu_vars[12];
__host config_t config ;
/**
 * Initialization flag
*/
int first ;
/**
 * Launch Data
*/
__host int dpu_iterations;
/**
 * RETURN VALUE
*/
__host int dpu_ret = STOPPED;

struct solver dpu_solver;
int main()
{
  if(first == 0)
  {
    populate_solver_context(&dpu_solver,dpu_vars,dpu_DB_offsets,config);
    first = 1;
  }
  dpu_ret = solve(&dpu_solver,dpu_iterations);
  log_config(dpu_solver.config);
}