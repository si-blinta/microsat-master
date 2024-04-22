#include "microsat.h"
//#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#include "portfolio.h"
#define DPU_BINARY "bin/dpu"
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
int main(int argc, char **argv)
{
  time_t start,end;
  double duration;
  if (argc < 2)
    abort();
  struct solver master;
  int ret = parse(&master,argv[1]);
  //printf("%d\n",solve(&master,100));
  //struct solver* S = portfolio_generate(&master);
  //show_solver_info_debug(S[0]);
  //portfolio_print_all_info(S);
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"parsing UNSAT");
    exit(0);
  }
  time(&start);
  ret = solve(&master,INT32_MAX);
  time(&end);
  duration = difftime(end,start);

  if(ret == UNSAT)
    log_message(LOG_LEVEL_INFO,"MASTER UNSAT");
  else if(ret == SAT)
    log_message(LOG_LEVEL_INFO,"MASTER SAT");
  else
    log_message(LOG_LEVEL_INFO,"MASTER STOPPED");
  //show_result(master);
  printf("%lf\n",duration);
  /*for(int i = 0; i < portfolio_get_length(S);i++)
  {
    ret = solve(&S[i],INT32_MAX);
    if(ret == SAT)
    {
      log_message(LOG_LEVEL_INFO,"PORTFOLIO SAT");
      show_result(S[i]);
      break;
    }
  }
  if(ret == UNSAT)
  {
    log_message(LOG_LEVEL_INFO,"PORTFOLIO UNSAT");
  }*/

} 
/**
 * 
 * READ MEEEEEEEEEEEEEEEEE
 * I NEED TO FIX THE PROBLEM USING THE CNF PRIME4, THIS CNF MUST BE SAT ALTHOUGH I CANT GET IT SAT WITH PORTFOLIO APPROACH FOR SOME REASON . 
 * 
 * READ MEEE , YOU WILL NEED TO MODIFY PORTFOLIO GENERATE, TO JUST SEND DATA AND NOT ALLOCATE EVERY THING
 * 
*/