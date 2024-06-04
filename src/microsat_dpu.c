#include "microsat_dpu.h"
extern int conflicts;
void unassign(struct solver *S, int lit) { S->falses[lit] = 0; } // Unassign the literal

void restart(struct solver *S)
{ // Perform a restart (i.e., unassign all variables)
  while (S->assigned > S->forced)
    unassign(S, *(--S->assigned)); // Remove all unforced false lits from falseStack
  S->processed = S->forced;
} // Reset the processed pointer

void assign(struct solver *S, int __mram_ptr* reason, int forced)
{                                                  // Make the first literal of the reason true
  int lit = reason[0];                             // Let lit be the first ltieral in the reason
  S->falses[-lit] = forced ? IMPLIED : 1;          // Mark lit as true and IMPLIED if forced
  *(S->assigned++) = -lit;                         // Push it on the assignment stack
  S->reason[abs(lit)] = 1 + (int)((reason)-S->DB); // Set the reason clause of lit
  S->model[abs(lit)] = (lit > 0);
//printf("c model[%d]=%d\n",abs(lit),S->model[abs(lit)]);
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

void reduceDB(struct solver *S, int k)
{ // Removes "less useful" lemmas from DB
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
    while (S->DB[i])
    {
      int lit = S->DB[i++]; // Count the number of literals
      if ((lit > 0) == S->model[abs(lit)])
        count++;
    } // That are satisfied by the current model
    if (count < k)
      addClause(S, S->DB + head, i - head, 0);
  }
  //printf("%d\n", S->nLemmas);
} // If the latter is smaller than k, add it back

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

int implied(struct solver *S, int lit)
{ // Check if lit(eral) is implied by MARK literals
  if (S->falses[lit] > MARK)
    return (S->falses[lit] & MARK); // If checked before return old result
  if (!S->reason[abs(lit)])
    return 0;                                 // In case lit is a decision, it is not implied
  int __mram_ptr* p = (S->DB + S->reason[abs(lit)] - 1); // Get the reason of lit(eral)
  while (*(++p))                              // While there are literals in the reason
    if ((S->falses[*p] ^ MARK) && !implied(S, *p))
    { // Recursively check if non-MARK literals are implied
      S->falses[lit] = IMPLIED - 1;
      return 0;
    } // Mark and return not implied (denoted by IMPLIED - 1)
  S->falses[lit] = IMPLIED;
  return 1;
} // Mark and return that the literal is implied

int __mram_ptr* analyze(struct solver *S, int __mram_ptr* clause)
{ // Compute a resolvent from falsified clause
  S->res++;
  S->nConflicts++; // Bump restarts and update the statistic
  while (*clause)
    bump(S, *(clause++)); // MARK all literals in the falsified clause
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
        bump(S, *(clause++));
    } // MARK all literals in reason
    unassign(S, *S->assigned);
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
    unassign(S, *(S->assigned--));   // Unassign all lits between tail & head
  unassign(S, *S->assigned);         // Assigned now equal to processed
  S->buffer[size] = 0;               // Terminate the buffer (and potentially print clause)
  return addClause(S, S->buffer, size, 0);
} // Add new conflict clause to redundant DB
int solve(struct solver *S,int stop_it)
{ // Determine satisfiability
  int decision = S->head;
  S->res = 0; // Initialize the solver
  for (int i = 0; i < stop_it ; i++)
  {                              // Main solve loop
    int old_nLemmas = S->nLemmas; // Store nLemmas to see whether propagate adds lemmas
    //printf("propagating\n");
    if (propagate(S) == UNSAT)
    {
      return UNSAT; // Propagation returns UNSAT for a root level conflict
    
    }
    if (S->nLemmas > old_nLemmas)
    { // If the last decision caused a conflict
      // printf("new learned clause\n");
      decision = S->head; // Reset the decision heuristic to head
      if (S->fast > (S->slow / 100) * 125) 
      { // If fast average is substantially larger than slow average
        S->res = 0;
        S->fast = (S->slow / 100) * 125; // 125
        restart(S); // Restart and update the averages
        if (S->nLemmas > S->maxLemmas)
          reduceDB(S, REDUCE_LIMIT); 
      }
    } // Reduce the DB when it contains too many lemmas

    while (S->falses[decision] || S->falses[-decision])
    { // As long as the temporay decision is assigned
      decision = S->prev[decision];
      // printf("decision = %d\n",S->prev[decision]);                // Replace it with the next variable in the decision list
    }
    if (decision == 0)
    {
      return SAT;                                         // If the end of the list is reached, then a solution is found
    }
    decision = S->model[decision] ? decision : -decision; // Otherwise, assign the decision variable based on the model
    S->falses[-decision] = 1;                             // Assign the decision literal to true (change to IMPLIED-1?)
    *(S->assigned++) = -decision;                         // And push it on the assigned stack
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
} // Decisions have no reason clauses
int propagate(struct solver *S)
{                                             // Performs unit propagation
  int forced = S->reason[abs(*S->processed)]; // Initialize forced flag
  while (S->processed < S->assigned)
  {                              // While unprocessed false literals
    int lit = *(S->processed++); // Get first unprocessed literal
    int __mram_ptr* watch = &S->first[lit]; // Obtain the first watch pointer
    while (*watch != END && *watch > 0)
    {                                     // While there are watched clauses (watched by lit)
      int i, unit = 1;                    // Let's assume that the clause is unit
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
          int store = *watch;
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
        } // A unit clause is found, and the reason is set
        else
        {
          if (forced)
            return UNSAT;                  // Found a root level conflict -> UNSAT
          int __mram_ptr* lemma = analyze(S, clause); // Analyze the conflict return a conflict clause
          if (!lemma[1])
            forced = 1; // In case a unit clause is found, set forced flag
          assign(S, lemma, forced);
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
{ // Determine satisfiability
  switch (restart_p)
  {
  case DEFAULT:
    return solve(S,stop_it);
    break;
  case FIXED:
    return solve_fixed(S,stop_it,thresh_hold);
    break;
  case RANDOM:
    return solve_random(S,stop_it);
    break;
  case LUBY:
    return solve_luby(S,stop_it,thresh_hold);
  default:
    return solve(S,stop_it);
    break;
  }
}
int power(int base, int exponent) {
    double result = 1.0;
    for(int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}
int luby(int y, int x) {

    // Find the finite subsequence that contains index 'x', and the
    // size of that subsequence:
    int size, seq;
    for(size = 1, seq = 0; size < x + 1; seq++, size = 2 * size + 1);

    while(size - 1 != x) {
        size = (size - 1) >> 1;
        seq--;
        x = x % size;
    }

    return power(y, seq);
}

int solve_luby(struct solver* S,int stop_it,int luby_param)
{
  int luby_index = 0;
  int decision = S->head;
  S->res = 0;
  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;

    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if ( conflicts >= luby(luby_param,luby_index) ) {
        S->res = 0;
        luby_index++;
        conflicts = 0;
        restart(S);
        if (S->nLemmas > S->maxLemmas) {
          reduceDB(S, REDUCE_LIMIT);
        }
      }
    }

    while (S->falses[decision] || S->falses[-decision]) {
      decision = S->prev[decision];
    }

    if (decision == 0) {
      return SAT;
    }

    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
}
int solve_geometric(struct solver* S,int stop_it,float geometric_factor,int min_thresh_hold)
{
  float restart_threshold = (float)min_thresh_hold;
  int decision = S->head;

  S->res = 0;

  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;

    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if (conflicts >= restart_threshold) {
        S->res = 0;
        conflicts = 0;
        restart_threshold *= geometric_factor;
        restart(S);
        if (S->nLemmas > S->maxLemmas)
        {
            reduceDB(S, REDUCE_LIMIT);
        }
      }
    }

    while (S->falses[decision] || S->falses[-decision]) {
      decision = S->prev[decision];
    }

    if (decision == 0) {
      return SAT;
    }

    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
}
int solve_fixed(struct solver* S, int stop_it, int fixed_thresh_hold) {
  int restart_threshold = fixed_thresh_hold;
  int decision = S->head;

  S->res = 0;

  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;

    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if (conflicts >= restart_threshold) {
        S->res = 0;
        conflicts = 0;
        restart(S);
        if (S->nLemmas > S->maxLemmas) {
          reduceDB(S, REDUCE_LIMIT);
        }
      }
    }

    while (S->falses[decision] || S->falses[-decision]) {
      decision = S->prev[decision];
    }

    if (decision == 0) {
      return SAT;
    }

    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
}
#define A 1103515245
#define C 12345
#define M 2147483647
extern uint32_t seed;
// Function to generate a pseudorandom number using LCG
unsigned int lcg() {
  seed = (A * seed + C) % M;
  return seed;
}

int solve_random(struct solver* S, int stop_it) {
  int restart_threshold = lcg() % 2000 + 10; 
  int decision = S->head;
  S->res = 0;

  for (int i = 0; i < stop_it ; i++) {
    int old_nLemmas = S->nLemmas;

    if (propagate(S) == UNSAT) {
      return UNSAT;
    }

    if (S->nLemmas > old_nLemmas) {
      decision = S->head;
      conflicts++;
      if (conflicts >= restart_threshold) {
        S->res = 0;
        conflicts = 0;
        restart(S);
        if (S->nLemmas > S->maxLemmas) {
          reduceDB(S, REDUCE_LIMIT);
        }
        // Update the restart threshold to a random number between 1 and 100
        restart_threshold = lcg() % 2000 + 10;
      }
    }

    while (S->falses[decision] || S->falses[-decision]) {
      decision = S->prev[decision];
    }

    if (decision == 0) {
      return SAT;
    }

    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
  }
  return STOPPED;
}

void unassign_last_decision(struct solver *S) 
{
  int lit = *(--S->assigned);
  if(S->falses[lit] != IMPLIED)
    unassign(S,lit);
}
void unassign_all(struct solver *S)
{
  while(S->assigned != S->falseStack)
  {
    unassign_last_decision(S);
  }
} 

void reset_solver(struct solver *S)
{
  unassign_all(S);
  //restart(S);
}

int get_unassigned(struct solver S)
{
  for(int i = 1 ; i < S.nVars; i++)
  {
    if(!(S.falses[i] != 0 || S.falses[-i] != 0))
    {
      return i;
    }
  }
  return 0;
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
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"
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
