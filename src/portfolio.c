#include "portfolio.h"
#include "microsat.h"
#include "assert.h"
#include "log.h"
struct solver* portfolio_alloc(int nVars)
{
    assert(nVars > 10);
    if(nVars > MAX_VARS)
      nVars = MAX_VARS;
    struct solver* comb = malloc((1<<nVars)* sizeof(struct solver));
    log_message(LOG_LEVEL_DEBUG,"allocated");
    printf("%d\n",1<<nVars);
    assert(comb != NULL);
    return comb;
}
void portfolio_populate(struct solver* from, struct solver* to)
{
  int nVars = from->nVars;
  if(nVars > MAX_VARS)
    nVars = MAX_VARS;
  log_message(LOG_LEVEL_DEBUG,"populated");
  printf("%d\n",1<<nVars);
  for(int i = 0 ; i < (1<<nVars); i++)
  {
    to[i].DB = (int *)malloc(sizeof(int) * MEM_MAX); // Allocate the initial database
    memset(to[i].DB,0,MEM_MAX*sizeof(int));
    memcpy(to[i].DB, from->DB, MEM_MAX * sizeof(int));
    to[i].nVars = from->nVars;
    to[i].nClauses = from->nClauses;
    to[i].mem_used = from->mem_used;
    to[i].mem_fixed= from->mem_fixed;
    to[i].nLemmas = from->nLemmas;
    to[i].nConflicts = from->nConflicts;
    to[i].maxLemmas = from->maxLemmas;
    to[i].fast = from->fast;
    to[i].slow = from->slow;
    to[i].head = from->head;
    to[i].res = from->res;
    to[i].model = to[i].DB + (from->model - from->DB);
    to[i].next = to[i].DB + (from->next - from->DB);
    to[i].prev = to[i].DB + (from->prev - from->DB);
    to[i].buffer = to[i].DB + (from->buffer - from->DB);
    to[i].reason = to[i].DB + (from->reason - from->DB);
    to[i].falseStack = to[i].DB + (from->falseStack - from->DB);
    to[i].forced = to[i].DB + (from->forced - from->DB);
    to[i].processed = to[i].DB + (from->processed - from->DB);
    to[i].assigned = to[i].DB + (from->assigned - from->DB);
    to[i].falses = to[i].DB + (from->falses - from->DB);
    to[i].first = to[i].DB + (from->first - from->DB);
  }
} 

void portfolio_combination(struct solver* comb,int nVars)
{
    if(nVars > MAX_VARS)
      nVars = MAX_VARS;
    log_message(LOG_LEVEL_DEBUG,"combined");
    printf("%d\n",1<<nVars);
    for(int i  = 0 ; i < 1<<nVars;i++){
        // Loop through each variable
        for (int j = 0; j < nVars; j++) {
            // Check if jth bit is set in i
            assign_decision(&comb[i],(i >> j) & 1 ? j + 1 : -(j + 1));
        }
      }
}
void portfolio_print_all_info(struct solver* comb)
{
  int nVars = comb[0].nVars;
  if(nVars > MAX_VARS)
    nVars = MAX_VARS;
  for(int i = 0; i < (1<<nVars);i++)
  {
    show_solver_info_debug(comb[i]);
    show_solver_stats(comb[i]);
  }
}
struct solver* portfolio_generate(struct solver* from)
{
  struct solver* comb = portfolio_alloc(from->nVars);
  assert(comb != NULL);
  portfolio_populate(from,comb);
  portfolio_combination(comb,comb[0].nVars);
  log_message(LOG_LEVEL_INFO,"Generated portfolio");
  return comb;
}
int portfolio_get_length(struct solver* from)
{
  if(from[0].nVars > MAX_VARS)
    return 1 << MAX_VARS;
  return 1 << from[0].nVars;
}