#include "microsat_dpu.h"
void update_chb(struct solver *S, int var, float multiplier) {
  float reward = multiplier / (S->config.numConflicts - S->config.lastConflict[var] + 1);
  S->config.Q[var] = (1.0 - S->config.alpha) * S->config.Q[var] + S->config.alpha * reward;
}

int pick_branching_variable_chb(struct solver *S) {
  int best_var = 0;
  float best_score = -1.0;
  for (int i = S->nVars+1; i > 0; i--) {
    if (!S->falses[i] && !S->falses[-i] && S->config.Q[i] > best_score) {
      best_score = S->config.Q[i];
      best_var = i;
    }
  }
  return best_var;
}

void log_decision(int lit, int level) {
    printf(LOG_LEVEL_DEBUG"Decide: Literal %d, Decision Level: %d\n", lit, level);
}

void log_propagation(int lit, int level) {
    printf(LOG_LEVEL_DEBUG"Propagate: Literal %d, Decision Level: %d\n", lit, level);
}

void log_unassign(int lit, int level) {
    printf(LOG_LEVEL_DEBUG"Unassign: Literal %d, Decision Level: %d\n", lit, level);
}

void log_conflict_analysis(int lit, int level) {
    printf(LOG_LEVEL_DEBUG"Conflict Analysis: Literal %d, Decision Level: %d\n", lit, level);
}
void print_clause(struct solver *S, int *clause) {
    for (int i = 0; clause[i]; i++) {
        printf("%d@%d ", clause[i], S->decision_level[abs(clause[i])]);
    }
    printf("\n");
}

int power(int base, int exponent)
{
  double result = 1.0;
  for (int i = 0; i < exponent; i++)
  {
    result *= base;
  }
  return result;
}
int luby(int y, int x)
{

  // Find the finite subsequence that contains index 'x', and the
  // size of that subsequence:
  int size, seq;
  for (size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1)
    ;

  while (size - 1 != x)
  {
    size = (size - 1) >> 1;
    seq--;
    x = x % size;
  }

  return power(y, seq);
}
void check_and_restart(struct solver *S)
{
  int reduce_flag = 0;
  if (S->nLemmas > S->maxLemmas)
    reduce_flag = 1;
  switch (S->config.rest_p)
  {
  case REST_DEFAULT:
    if (S->fast > (S->slow / 100) * 125)
    {
#if RESTART_DEBUG    
      printf(LOG_LEVEL_DEBUG"default restart after %d conflicts\n", S->config.conflicts);
#endif
      // Restart logic for default policy
      S->fast = (S->slow / 100) * 125; // 125
      restart(S);                      // Restart and update the averages
      if (reduce_flag)
        reduceDB(S, REDUCE_LIMIT);
      reset_decision_levels(S);
    }
    break;
  case REST_LUBY:
    if (S->config.conflicts >= luby(S->config.luby_base, S->config.luby_index))
    {
#if RESTART_DEBUG    
      printf(LOG_LEVEL_DEBUG"luby restart after %d conflicts\n", S->config.conflicts);
#endif
      restart(S); // Restart and update the averages
      S->config.luby_index++;
      S->config.conflicts = 0;
      if (reduce_flag)
        reduceDB(S, REDUCE_LIMIT);
      reset_decision_levels(S);
    }
    break;
  case REST_GEO:
    if (S->config.conflicts >= S->config.geo_max)
    {
#if RESTART_DEBUG   
      printf(LOG_LEVEL_DEBUG"geo restart after %d conflicts\n", S->config.conflicts);
#endif 
      restart(S); // Restart and update the averages
      S->config.geo_max *= S->config.geo_factor;
      S->config.conflicts = 0;
      if (reduce_flag)
        reduceDB(S, REDUCE_LIMIT);
      reset_decision_levels(S);
    }
    break;
  case REST_ARITH:
    if (S->config.conflicts >= S->config.arith_max)
    {
#if RESTART_DEBUG   
      printf(LOG_LEVEL_DEBUG"arithmetic restart after %d confli\ncts", S->config.conflicts);
#endif
      restart(S); // Restart and update the averages
      S->config.arith_max += S->config.arith_reason;
      S->config.conflicts = 0;
      if (reduce_flag)
        reduceDB(S, REDUCE_LIMIT);
      reset_decision_levels(S);
    }
    break;
  }
}
void unassign(struct solver *S, int lit,int flag) { 
  if(flag == 1)//flag == 0 when restarting
  {
#if DECISION_DEBUG
    log_unassign(lit, S->decision_level[abs(lit)]); // Log unassign
#endif 
    S->decision_level[abs(lit)] = -1;
    if (S->decision_counter > 0 && S->reason[abs(lit)] == 0 )
            S->decision_counter--;
  }     
  S->falses[lit] = 0; 
  } // Unassign the literal
void reset_decision_levels(struct solver *S)
{
  for(int lit = 1 ; lit < S->nVars+1 ; lit++)
    S->decision_level[lit] = -1;
  S->decision_counter = 0;
}
void restart(struct solver *S)
{ // Perform a restart (i.e., unassign all variables)
  while (S->assigned > S->forced)
    unassign(S, *(--S->assigned),0); // Remove all unforced false lits from falseStack
  S->processed = S->forced;
} // Reset the processed pointer
void assign(struct solver *S, int __mram_ptr* reason, int forced)
{                                                  // Make the first literal of the reason true
  int lit = reason[0];                             // Let lit be the first ltieral in the reason
  S->falses[-lit] = forced ? IMPLIED : 1;          // Mark lit as true and IMPLIED if forced
  *(S->assigned++) = -lit;                         // Push it on the assignment stack
  S->reason[abs(lit)] = 1 + (int)((reason)-S->DB); // Set the reason clause of lit
  S->model[abs(lit)] = (lit > 0);
  S->decision_level[abs(lit)] = S->decision_counter;
#if DECISION_DEBUG  
  log_propagation(lit, S->decision_level[abs(lit)]); // Log propagation
#endif
} // Mark the literal as true in the model

void addWatch(struct solver *S, int lit, int mem)
{ // Add a watch pointer to a clause containing lit
  S->DB[mem] = S->first[lit];
  S->first[lit] = mem;
} // By updating the database and the pointers

int __mram_ptr* getMemory(struct solver *S, int mem_size)
{ // Allocate memory of size mem_size
  if (S->mem_used > MEM_MAX - mem_size)
  { // In case the code is used within a code base    
    printf("c out of memory | mem used %d|mem wanted to allocate %d\n", S->mem_used, mem_size);
    halt();
  }
  int __mram_ptr* store = (S->DB + S->mem_used); // Compute a pointer to the new memory location
  S->mem_used += mem_size;            // Update the size of the used memory
  return store;
} // Return the pointer
int __mram_ptr* addClause(struct solver *S, int __mram_ptr* in, int size, int irr)
{                                           // Adds a clause stored in *in of size size
  int i, used = S->mem_used;                // Store a pointer to the beginning of the clause
  int __mram_ptr* clause = getMemory(S, size + 3) + 2; // Allocate memory for the clause in the database
  if (size > 1)
  {
    addWatch(S, in[0], used); // If the clause is not unit, then add
    addWatch(S, in[1], used + 1);
  } // Two watch pointers to the datastructure
  for (i = 0; i < size; i++)
    clause[i] = in[i];
  clause[i] = 0; // Copy the clause from the buffer to the database
  if (irr)
    S->mem_fixed = S->mem_used;
  else
    S->nLemmas++; // Update the statistics
  return clause;
} // Return the pointer to the clause is the database
int compute_lbd(struct solver *S, int __mram_ptr *clause) {
    int levels[MAX_LBD];
    memset(levels, 0, sizeof(levels)); 
    int lbd = 0;
    
    for (int i = 0; clause[i] && lbd < MAX_LBD; i++) {
        int var = abs(clause[i]);
        int level = S->decision_level[var];
        if (level > 0 && level < MAX_LBD && !levels[level]) {
            levels[level] = 1;
            lbd++;
        }
    }
    return lbd;
}
int is_lbd_2(struct solver *S, int __mram_ptr *clause) {
    int levels[3] = {0, 0, 0}; 
    int lbd = 0;

    for (int i = 0; clause[i]; i++) {
        int var = abs(clause[i]);
        int level = S->decision_level[var];
        
        if (level > 0) {
            int j;
            for (j = 0; j < lbd; j++) {
                if (levels[j] == level) {
                    break;
                }
            }
            if (j == lbd) {
                if (lbd < 2) {
                    levels[lbd++] = level;
                } else {
                    return 0; // More than 2 unique decision levels
                }
            }
        }
    }

    return 1; // LBD is less than or equal to 2
}
void reduceDB(struct solver *S, int k)
{ // Removes "less useful" lemmas from DB
  while (S->nLemmas > S->maxLemmas)
    S->maxLemmas += 300; // Allow more lemmas in the future
  S->nLemmas = 0;        // Reset the number of lemmas

  int i;
  for (i = -S->nVars; i <= S->nVars; i++)
  { // Loop over the variables
    if (i == 0)
      continue;
    int __mram_ptr* watch = &S->first[i]; // Get the pointer to the first watched clause
    while (*watch != END)      // As long as there are watched clauses
      if (*watch < S->mem_fixed)
        watch = (S->DB + *watch); // Remove the watch if it points to a lemma
      else
        *watch = S->DB[*watch];
  } // Otherwise (meaning an input clause) go to next watch

  int old_used = S->mem_used;
  S->mem_used = S->mem_fixed; // Virtually remove all lemmas
  for (i = S->mem_fixed + 2; i < old_used; i += 3)
  {                          // While the old memory contains lemmas
    int count = 0, head = i; // Get the lemma to which the head is pointing
    float size = 0;
    float score = 0;
    int __mram_ptr* clause = &S->DB[head];
    int lbd_2 = is_lbd_2(S, clause); // Calculate the LBD of the clause
    int lbd = 0;
    if(S->config.reduce_p == RED_LBD)
      lbd = compute_lbd(S,clause);
    while (S->DB[i])
    {
      int lit = S->DB[i++]; // Count the number of literals
      if ((lit > 0) == S->model[abs(lit)])
        count++;
      size++;
      score+= S->scores[abs(lit)];
      
    } // That are satisfied by the current model
    if ( S->config.reduce_p == RED_DEFAULT && count < k)
    {
#if REDUCE_DEBUG 
      printf(LOG_LEVEL_DEBUG"ADDED a clause based on number of assigned literals\n");
      //print_clause(S,clause);
#endif
      addClause(S, S->DB + head, i - head, 0);
    }
    else if(S->config.reduce_p == RED_SIZE && size <= S->config.clause_size)
    {
#if REDUCE_DEBUG 
      printf(LOG_LEVEL_DEBUG"ADDED a clause based on size\n");
      //print_clause(S,clause);
#endif
      addClause(S, S->DB + head, i - head, 0);
    }
    // Clauses of lbd  = 2 are importants, no matter the policy of reducing , we will conserve them.
    else if(S->config.reduce_p == RED_LBD && lbd <= S->config.max_lbd)
    {
#if REDUCE_DEBUG 
      printf(LOG_LEVEL_DEBUG"ADDED clause based on LBD (%d)\n",lbd);
      //print_clause(S,clause);
#endif
      addClause(S, S->DB + head, i - head, 0);
    }
    else if(lbd_2)
    {
#if REDUCE_DEBUG 
      printf(LOG_LEVEL_DEBUG"ADDED LBD-2 clause\n");
      //printclause(S,clause);
#endif
      addClause(S, S->DB + head, i - head, 0);
    }
    

  }
} // If the latter is smaller than k, add it back
void increment(struct solver *S, int lit)
{
  if (S->falses[lit] != IMPLIED)
  {
    S->falses[lit] = MARK; // MARK the literal as involved if not a top-level unit
    S->scores[abs(lit)]++;
  }
}
void bump(struct solver *S, int lit)
{ // Move the variable to the front of the decision list
  if (S->falses[lit] != IMPLIED)
  {
    S->falses[lit] = MARK; // MARK the literal as involved if not a top-level unit
    int var = abs(lit);
    if (var != S->head)
    {                                       // In case var is not already the head of the list
      S->prev[S->next[var]] = S->prev[var]; // Update the prev link, and
      S->next[S->prev[var]] = S->next[var]; // Update the next link, and
      S->next[S->head] = var;               // Add a next link to the head, and
      S->prev[var] = S->head;
      S->head = var;
    }
  }
} // Make var the new head
void decay(struct solver *S, float factor)
{
  for (int j = 1; j < S->nVars + 1; j++)
  {
    S->scores[j] = S->scores[j] * factor;
  }
}
int implied(struct solver *S, int lit)
{ // Check if lit(eral) is implied by MARK literals
  if (S->falses[lit] > MARK)
    return (S->falses[lit] & MARK); // If checked before return old result
  if (!S->reason[abs(lit)])
    return 0;                                 // In case lit is a decision, it is not implied
  int __mram_ptr* p = (S->DB + S->reason[abs(lit)] - 1); // Get the reason of lit(eral)
  while (*(++p))
  {                              // While there are literals in the reason
    if ((S->falses[*p] ^ MARK) && !implied(S, *p))
    { // Recursively check if non-MARK literals are implied
      S->falses[lit] = IMPLIED - 1;
      return 0;
    } // Mark and return not implied (denoted by IMPLIED - 1)
  }
  S->falses[lit] = IMPLIED;
  return 1;
} // Mark and return that the literal is implied



int __mram_ptr* analyze(struct solver *S, int __mram_ptr *clause)
{ // Compute a resolvent from falsified clause
  S->res++;
  S->nConflicts++; // Bump restarts and update the statistic
  while (*clause)
  {
    if (S->config.br_p == BR_VMTF || S->config.br_p == BR_CHB)
      bump(S, *(clause++)); // MARK all literals in the falsified clause
    if (S->config.br_p == BR_VSIDS)
      increment(S, *(clause++));
    // todo lrb
  }
  while (S->reason[abs(*(--S->assigned))])
  { // Loop on variables on falseStack until the last decision
    if (S->falses[*S->assigned] == MARK)
    {                                       // If the tail of the stack is MARK
      int __mram_ptr* check = S->assigned;             // Pointer to check if first-UIP is reached
      while (S->falses[*(--check)] != MARK) // Check for a MARK literal before decision
        if (!S->reason[abs(*check)])
          goto build;                                // Otherwise it is the first-UIP so break
      clause = S->DB + S->reason[abs(*S->assigned)]; // Get the reason and ignore first literal
      while (*clause)
      { // is it also good for VSIDS : incrementing all the variables that are in the clause that caused a conflict ? ask sami
        if (S->config.br_p == BR_VMTF || S->config.br_p == BR_CHB)
          bump(S, *(clause++)); // MARK all literals in the falsified clause
        if (S->config.br_p == BR_VSIDS)
          increment(S, *(clause++));
        // todo lrb
      }
    }
    unassign(S, *S->assigned,1);
  } // Unassign the tail of the stack
build:;
  int size = 0, lbd = 0, flag = 0;     // Build conflict clause; Empty the clause buffer
  int __mram_ptr* p = S->processed = S->assigned; // Loop from tail to front
  while (p >= S->forced)
  { // Only literals on the stack can be MARKed
    if ((S->falses[*p] == MARK) && !implied(S, *p))
    { // If MARKed and not implied
      S->buffer[size++] = *p;
      flag = 1;
    } // Add literal to conflict clause buffer
    if (!S->reason[abs(*p)])
    {
      lbd += flag;
      flag = 0; // Increase LBD for a decision with a true flag
      if (size == 1)
        S->processed = p;
    } // And update the processed pointer
    S->falses[*(p--)] = 1;
  } // Reset the MARK flag for all variables on the stack

  S->fast -= S->fast >> 5;
  S->fast += lbd << 15; // Update the fast moving average
  S->slow -= S->slow >> 15;
  S->slow += lbd << 5; // Update the slow moving average

  while (S->assigned > S->processed) // Loop over all unprocessed literals
    unassign(S, *(S->assigned--),1);   // Unassign all lits between tail & head
  unassign(S, *S->assigned,1);         // Assigned now equal to processed
  S->buffer[size] = 0;               // Terminate the buffer (and potentially print clause)
  return addClause(S, S->buffer, size, 0);
} // Add new conflict clause to redundant DB
int decide(struct solver* S,int decision)
{
  if(S->config.br_p == BR_VMTF)
    {
      while (S->falses[decision] || S->falses[-decision])
      { // As long as the temporay decision is assigned
        decision = S->prev[decision];
      }
      return decision;
    }
  else if(S->config.br_p == BR_VSIDS)
    {
      int highest_score_var = 0;  // Variable with the highest score
      float highest_score = -1.0f;  // Initialize to a low value

      // Loop through all variables to find the unassigned one with the highest score
      for (int var = 1; var <= S->nVars; var++) {
          if (S->falses[var] == 0 && S->falses[-var] == 0) {  // Check if the variable is unassigned
              if (S->scores[var] > highest_score) {  // Check if current variable has a higher score than the current highest
                  highest_score = S->scores[var];
                  highest_score_var = var;
              }
          }
      }

      if (highest_score_var == 0) {
          // No unassigned variables found, return 0 or handle it as per your solver's design
          return 0;
      }
      return highest_score_var;
    }
    return 0;
  /*else if (S->config.br_p == BR_CHB)
  {
    return pick_branching_variable_chb(S);
  }*/
}

int solve(struct solver *S, int stop_it)
{ // Determine satisfiability
  int decision = decide(S,S->head);
  S->res = 0; // Initialize the solver
  for (int i = 0; i < stop_it; i++)
  {                               // Main solve loop;
    int old_nLemmas = S->nLemmas; // Store nLemmas to see whether propagate adds lemmas
    // printf("propagating\n");
    if (propagate(S) == UNSAT)
    {
      return UNSAT; // Propagation returns UNSAT for a root level conflict
    }
    if (S->nLemmas > old_nLemmas)
    {
      if(S->config.br_p == BR_VMTF)
        decision = S->head;
      S->config.conflicts++;
      S->res = 0;
      if (S->config.br_p == BR_VSIDS)
      {
        if (S->config.conflicts % S->config.decay_thresh_hold == 0)
          decay(S, 0.90);
      }
      /*else if(S->config.br_p == BR_CHB)
        if (S->config.alpha > 0.06) {
          S->config.alpha -= 0.000001;
      }*/
      check_and_restart(S);
    }
    decision = decide(S,decision);
    if (decision == 0)
    {
      return SAT; // If the end of the list is reached, then a solution is found
    }
    decision = S->model[decision] ? decision : -decision; // Otherwise, assign the decision variable based on the model
    S->falses[-decision] = 1;                             // Assign the decision literal to true (change to IMPLIED-1?)
    *(S->assigned++) = -decision;                         // And push it on the assigned stack
    decision = abs(decision);
    S->reason[decision] = 0;
    S->decision_counter++;
    S->decision_level[decision] = S->decision_counter;
#if DECISION_DEBUG
    log_decision(decision, S->decision_level[decision]); // Log decision
#endif
  }
  return STOPPED;
} // Decisions have no reason clauses

int propagate(struct solver *S)
{                                             // Performs unit propagation
  __dma_aligned int forced = S->reason[abs(*S->processed)]; // Initialize forced flag
  while (S->processed < S->assigned)
  {                              // While unprocessed false literals
    __dma_aligned int lit = *(S->processed++); // Get first unprocessed literal
    int __mram_ptr* watch = &S->first[lit]; // Obtain the first watch pointer
    while (*watch != END && *watch > 0)
    {                                     // While there are watched clauses (watched by lit)
      __dma_aligned int i, unit = 1;                    // Let's assume that the clause is unit
      int __mram_ptr* clause = (S->DB + *watch + 1); // Get the clause from DB
      if (clause[-2] == 0)
        clause++; // Set the pointer to the first literal in the clause
      if (clause[0] == lit)
        clause[0] = clause[1];            // Ensure that the other watched literal is in front
      for (i = 2; unit && clause[i]; i++) // Scan the non-watched literals
        if (!S->falses[clause[i]])
        { // When clause[i] is not false, it is either true or unset
          clause[1] = clause[i];
          clause[i] = lit; // Swap literals
          __dma_aligned int store = *watch;
          unit = 0;               // Store the old watch
          *watch = S->DB[*watch]; // Remove the watch from the list of lit
          addWatch(S, clause[1], store);
        } // Add the watch to the list of clause[1]
      if (unit)
      { // If the clause is indeed unit
        clause[1] = lit;
        watch = (S->DB + *watch); // Place lit at clause[1] and update next watch
        if (S->falses[-clause[0]])
          continue; // If the other watched literal is satisfied continue
        if (!S->falses[clause[0]])
        { // If the other watched literal is falsified,
          assign(S, clause, forced);
          //update_chb(S,abs(clause[0]),0.9); // todo : divide by 0.9 propagated units if a conflict occured 
        } // A unit clause is found, and the reason is set
        else
        {
          if (forced)
            return UNSAT;                  // Found a root level conflict -> UNSAT
          int __mram_ptr* lemma = analyze(S, clause); // Analyze the conflict return a conflict clause
          if (!lemma[1])
            forced = 1; // In case a unit clause is found, set forced flag
          assign(S, lemma, forced);
          S->config.numConflicts++;
          for (int i = 0; lemma[i] != 0; i++) 
          {
            int var = abs(lemma[i]);
            S->config.lastConflict[var] = S->config.numConflicts;
          }
          break;
        }
      }
    }
  } // Assign the conflict clause as a unit
  if (forced)
    S->forced = S->processed; // Set S->forced if applicable
  return SAT;
} // Finally, no conflict was found
void assign_decision(struct solver *S, int lit)
{
  S->falses[-lit] = IMPLIED;          // Mark lit as true and IMPLIED if forced
  *(S->assigned++) = -lit;            // Push it on the assignment stack
  S->reason[abs(lit)] = END;          // Set the reason as undefined ( different from 0 ).
  S->model[abs(lit)] = (lit > 0);
}
int solve_portfolio(struct solver *S,int restart_p,int stop_it,int thresh_hold)
{ 
  return solve(S,stop_it);
}
void picosat_proof(struct solver S)
{
  for(int i = 1 ; i < S.nVars+1 ; i++)
  {
    int m = 1;
    if(S.model[i] == 0)
      m = -1;
    printf("-a %d ",i *m);
  }
  printf("\n\n");
}
void show_result(struct solver S)
{
  printf("[DPU] Results:\n");  
  for (int i = 0; i < S.nVars + 1; i++)
  {
    printf("[%d,%d] ",i,(int)S.model[i]);
  }
  printf("\n\n");
  printf("[DPU] Pico Sat proof:\n");  
  picosat_proof(S);
}
void show_solver_info_debug(struct solver S)
{
  printf("[DPU] Solver data base:\n");
  int partition[9] = {S.nVars + 1, S.nVars + 1, S.nVars + 1, S.nVars, S.nVars + 1, S.nVars + 1, 2 * S.nVars + 1, 2 * S.nVars + 1,__INT_MAX__};
  int current = partition[0];
  int cumul = 0;
  int j = 0;
  for (int i = 0; i < S.mem_used; i++)
  {
    if (cumul == current || i == 0)
    {
      switch (j)
      {
      case MODEL:
        printf(ANSI_COLOR_YELLOW "\n\nmodel:");
        break;
      case NEXT:
        printf(ANSI_COLOR_YELLOW "\n\nnext:");
        break;
      case PREV:
        printf(ANSI_COLOR_YELLOW "\n\nprev:");
        break;
      case BUFFER:
        printf(ANSI_COLOR_YELLOW "\n\nbuffer:");
        break;
      case REASON:
        printf(ANSI_COLOR_YELLOW "\n\nreason:");
        break;
      case FALSESTACK:
        printf(ANSI_COLOR_YELLOW "\n\nfalseStack:");
        break;
      case FALSES:
        printf(ANSI_COLOR_YELLOW "\n\nfalses:");
        break;
      case FIRST:
        printf(ANSI_COLOR_YELLOW "\n\nfirst:");
        break;
      case CLAUSES:
        printf(ANSI_COLOR_YELLOW "\n\nclauses:");
        break;
      }
      cumul = 0;
      current = partition[j++];
    }
    printf(ANSI_COLOR_RESET "[%d,%d] ", i, S.DB[i]);
    cumul++;
  }
  printf("\n");
}
void show_solver_stats(struct solver S)
{
  printf("[DPU] Solver stats:\n");
  printf("nVars     = %d\n", S.nVars);
  printf("nClauses  = %d\n", S.nClauses);
  printf("mem_used  = %d\n", S.mem_used);
  printf("mem fixed = %d\n", S.mem_fixed);
  printf("maxLemmas = %d\n", S.maxLemmas);
  printf("nLemmas   = %d\n", S.nLemmas);
  printf("nConflicts= %d\n", S.nConflicts);
  printf("fast      = %d\n", S.fast);
  printf("slow      = %d\n", S.slow);
  printf("head      = %d\n", S.head);
  printf("res       = %d\n", S.res);
  printf("prev      = %d\n", S.prev[0]);
  printf("next      = %d\n", S.next[0]);
  printf("first     = %d\n", S.first[0]);
  printf("falses    = %d\n", S.falses[0]);
  printf("falseStack= %d\n", S.falseStack[0]);
  printf("assigned  = %d\n", S.assigned[0]);
  printf("processed = %d\n", S.processed[0]);
  printf("reason    = %d\n", S.reason[0]);
  printf("forced    = %d\n", S.forced[0]);
  printf("buffer    = %d\n", S.buffer[0]);
  printf("model     = %d\n", S.model[0]);
}
