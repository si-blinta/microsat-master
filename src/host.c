#include "microsat.h"
#include "hostTools.h"
#define DPU_BINARY "bin/dpu"
char* fileToString(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    // Allocate memory for the string
    char* content = (char*)malloc(size + 1);
    if (content == NULL) {
        fclose(file);
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // Read file content into the string
    size_t bytesRead = fread(content, 1, size, file);
    content[bytesRead] = '\0'; // Null-terminate the string

    fclose(file);
    return content;
}
int main (int argc, char** argv) {			             
  struct solver S,tmp; 
  char* cnf = fileToString(argv[1]);  
  /*
  uint32_t len = strlen(cnf);
  printf("%s\n",cnf);
  uint32_t nb_dpus=1;
  struct dpu_set_t set,dpu;
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  DPU_ASSERT(dpu_load(set,DPU_BINARY,NULL));
  DPU_ASSERT(dpu_launch(set,DPU_SYNCHRONOUS));
  DPU_ASSERT(dpu_broadcast_to(set,"buffer_size",0,&len, sizeof(uint32_t),DPU_XFER_DEFAULT));
  DPU_ASSERT(dpu_broadcast_to(set,"buffer",0,cnf,1024,DPU_XFER_DEFAULT));
  DPU_FOREACH(set,dpu){
    DPU_ASSERT(dpu_log_read(dpu,stdout));
  }
  DPU_ASSERT(dpu_free(set));


  */
  
  
  
  /*
  if (argc < 2) abort ();
  struct solver S,tmp;   

  if(parse (&S, argv[1]) == UNSAT) {
    printf("s UNSATISFIABLE\n");  // Parse the DIMACS file in argv[1]
    return 0;
  } 
  else if (solve (&S)          == UNSAT) printf("s UNSATISFIABLE\n");  // Solve without limit (number of conflicts)
  else                                   printf("s SATISFIABLE\n")  ;  // And print whether the formula has a solution
  printf ("c statistics of %s: mem: %i conflicts: %i max_lemmas: %i\n", argv[1], S.mem_used, S.nConflicts, S.maxLemmas); */
  
  
  
  }
