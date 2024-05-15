#include "microsat.h"
#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
int* get_unassigned_lits(struct solver S,int *size)
{
  int* lits = malloc(S.nVars*sizeof(int));
  int j = 0;
  for(int i = 1 ; i < S.nVars; i++)
  {
    if(S.falses[i] == 0 && S.falses[-i] == 0)
    {
      lits[j] = i;
      j++;
    }
  }
  *size = j;
  return lits;
}
int* get_assigned_lits(struct solver S, int* size)
{
  int* lits = malloc(S.nVars*sizeof(int));
  int j = 0;
  for(int i = 1 ; i < S.nVars; i++)
  {
    if(S.falses[i] != 0 || S.falses[-i] != 0)
    {
      lits[j] = i;
      j++;
    }
  }
  *size = j;
  return lits;
}
int main(int argc, char **argv)
{
  if (argc < 3)
    abort();
  struct dpu_set_t set;
  uint32_t nb_dpus = NB_DPU;//atoi(argv[2]); 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  //HOST_TOOLS_pure_portfolio(argv[1],set);
  HOST_TOOLS_divide_and_conquer(argv[1],set);
  
  
  
  /*
  struct solver s;
  int ret = parse(&s,argv[1]);
  ret = solve(&s,100);
  if(ret == STOPPED)
    printf("stopped\n");
  //show_solver_info_debug(s);
  reset_solver(&s);
  int size = 10;
  int *unassigned = get_unassigned_lits(s,&size);
  for(int i = 0 ; i < size; i++ )
    printf("%d ",unassigned[i]);
  printf("\n");

  reset_solver(&s);
  show_solver_info_debug(s);
  ret = solve(&s,INT32_MAX);
  if(ret == UNSAT)
    printf("unsat\n");
  else if(ret == SAT)
    show_result(s);
  */
   

}