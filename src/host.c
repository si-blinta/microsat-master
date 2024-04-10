#include "microsat.h"
#include "hostTools.h"
#define DPU_BINARY "bin/dpu"
int main (int argc, char** argv) {			             
  if (argc < 2) abort ();
  struct solver S;	                                                   // Create the solver datastructure
  if(parse (&S, argv[1]) == UNSAT) {
    printf("s UNSATISFIABLE\n");  // Parse the DIMACS file in argv[1]
    return 0;
  }
  uint32_t nb_dpus=2;
  struct dpu_set_t set,dpu;
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(11);
  DPU_ASSERT(dpu_load(set,DPU_BINARY,NULL));
  DPU_ASSERT(dpu_broadcast_to(set,"x",0,&nb_dpus,8,DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
  DPU_FOREACH(set,dpu){
    DPU_ASSERT(dpu_log_read(dpu,stdout));

  }
  DPU_ASSERT(dpu_free(set));


  
  
  
  
  /*
   
  else if (solve (&S)          == UNSAT) printf("s UNSATISFIABLE\n");  // Solve without limit (number of conflicts)
  else                                   printf("s SATISFIABLE\n")  ;  // And print whether the formula has a solution
  printf ("c statistics of %s: mem: %i conflicts: %i max_lemmas: %i\n", argv[1], S.mem_used, S.nConflicts, S.maxLemmas); */
  
  
  
  }
