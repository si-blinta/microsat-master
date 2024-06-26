#include "microsat_host.h"
#include "log.h"
reduce_functions reduce_funcs[3];
restart_function restart_funcs[3];
branching_function branching_funcs[3];
void setup_reduce_functions()
{
    reduce_funcs[RED_DEFAULT] = reduce_default;
    reduce_funcs[RED_SIZE] = reduce_size;
    reduce_funcs[RED_LBD] = reduce_lbd;
}
void setup_branching_functions()
{
    branching_funcs[BR_VMTF] = branching_vmtf;
    branching_funcs[BR_VSIDS] = branching_vsids;
    branching_funcs[BR_CHB] = branching_chb;
}
void setup_restart_functions()
{
    restart_funcs[REST_DEFAULT] = restart_default;
    restart_funcs[REST_ARITH] = restart_arith;
    restart_funcs[REST_GEO] = restart_geo;
    restart_funcs[REST_LUBY] = restart_luby;
}
void setup_functions()
{
    setup_restart_functions();
    setup_reduce_functions();
    setup_branching_functions();
}
int reduce_default(struct solver *S, int count, int k, float size, int clause_size, float lbd)
{
    return (count < k);
}

int reduce_size(struct solver *S, int count, int k, float size, int clause_size, float lbd)
{
    return (size <= clause_size);
}

int reduce_lbd(struct solver *S, int count, int k, float size, int clause_size, float lbd)
{
    return (lbd <= S->config.max_lbd);
}
void restart_default(struct solver *S)
{
    if (S->fast > (S->slow / 100) * 125)
    {
        S->fast = (S->slow / 100) * 125;
        restart(S);
        if (S->nLemmas > S->maxLemmas)
        {
            reduceDB(S, REDUCE_LIMIT);
        }
        reset_decision_levels(S);
    }
}

void restart_luby(struct solver *S)
{
    if (S->config.conflicts >= luby(S->config.luby_base, S->config.luby_index))
    {
        restart(S);
        S->config.luby_index++;
        S->config.conflicts = 0;
        if (S->nLemmas > S->maxLemmas)
        {
            reduceDB(S, REDUCE_LIMIT);
        }
        reset_decision_levels(S);
    }
}

void restart_geo(struct solver *S)
{
    if (S->config.conflicts >= S->config.geo_max)
    {
        restart(S);
        S->config.geo_max *= S->config.geo_factor;
        S->config.conflicts = 0;
        if (S->nLemmas > S->maxLemmas)
        {
            reduceDB(S, REDUCE_LIMIT);
        }
        reset_decision_levels(S);
    }
}

void restart_arith(struct solver *S)
{
    if (S->config.conflicts >= S->config.arith_max)
    {
        restart(S);
        S->config.arith_max += S->config.arith_reason;
        S->config.conflicts = 0;
        if (S->nLemmas > S->maxLemmas)
        {
            reduceDB(S, REDUCE_LIMIT);
        }
        reset_decision_levels(S);
    }
}

int branching_vmtf(struct solver* S,int decision)
{
    while (S->falses[decision] || S->falses[-decision])
    {
        decision = S->prev[decision];
    }
    return decision;
}

int branching_vsids(struct solver *S,int decision)
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
int branching_chb(struct solver *S,int decision) {
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

void initialize_chb(struct solver *S, int num_vars) {
  log_message(LOG_LEVEL_DEBUG, "Initializing CHB with %d variables", num_vars);
  S->config.lastConflict = (int *)malloc((num_vars+1) * sizeof(int));
  S->config.Q = (float *)malloc((num_vars+1) * sizeof(float));
  S->config.plays = (int *)malloc((num_vars+1) * sizeof(int));
  for (int i = 0; i < num_vars+1; i++) {
    S->config.lastConflict[i] = 0;
    S->config.Q[i] = 0.0;
  }
  
  S->config.alpha = 0.4;

  log_message(LOG_LEVEL_DEBUG, "CHB initialization completed");
}

void update_chb(struct solver *S, int var, float multiplier) {
  float reward = multiplier / (S->nConflicts- S->config.lastConflict[var] + 1);
  S->config.Q[var] = (1.0 - S->config.alpha) * S->config.Q[var] + S->config.alpha * reward;

}


int pick_branching_variable_chb(struct solver *S) {
  int best_var = 0;
  float best_score = -1.0;
  for (int i = 1; i < S->nVars+1; i++) {
    if (!S->falses[i] && !S->falses[-i] && S->config.Q[i] > best_score) {
      best_score = S->config.Q[i];
      best_var = i;
    }
  }
  return best_var;
}



void handle_conflict(struct solver *S, int *conflict_clause) {
  log_message(LOG_LEVEL_DEBUG, "Handling conflict with clause: ");
  print_clause(S, conflict_clause);

  for (int i = 0; conflict_clause[i] != 0; i++) {
    int var = abs(conflict_clause[i]);
    update_chb(S, var, 1.0);
    S->config.lastConflict[var] = S->nConflicts;
  }
  if (S->config.alpha > 0.06) {
    S->config.alpha -= 0.000001;
  }
  log_message(LOG_LEVEL_DEBUG, "Conflict handled. numConflicts: %d, alpha: %.6f", S->nConflicts, S->config.alpha);
}


int power(int base, int exponent)
{
  float result = 1.0;
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
  restart_funcs[S->config.rest_p](S);
}
int decide(struct solver* S,int decision)
{
  return branching_funcs[S->config.br_p](S,decision);
}
int solve(struct solver *S, int stop_it) {
  int decision = decide(S,S->head);
  S->res = 0;
  for (int i = 0; i < stop_it; i++) {
    int old_nLemmas = S->nLemmas;
    if (propagate(S) == UNSAT) {
      return UNSAT;
    }
    if (S->nLemmas > old_nLemmas) {
      if (S->config.alpha > 0.06) {
        S->config.alpha -= 0.000001;
      }
      S->config.conflicts++;
      S->res = 0;
      if(S->config.br_p == BR_VMTF)
        decision = S->head;
      check_and_restart(S);
      
    }
    decision = decide(S,decision);
    if (decision == 0) {
      return SAT;
    }
    decision = S->model[decision] ? decision : -decision;
    S->falses[-decision] = 1;
    *(S->assigned++) = -decision;
    decision = abs(decision);
    S->reason[decision] = 0;
    S->decision_counter++;
    S->decision_level[decision] = S->decision_counter;
  }
  return STOPPED;
}


void assign_decision(struct solver *S, int lit)
{
  S->falses[-lit] = IMPLIED; // Mark lit as true and IMPLIED if forced
  *(S->assigned++) = -lit;   // Push it on the assignment stack
  S->reason[abs(lit)] = END; // Set the reason as undefined ( different from 0 ).
  S->model[abs(lit)] = (lit > 0);
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

void assign(struct solver *S, int *reason, int forced)
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

int *getMemory(struct solver *S, int mem_size)
{ // Allocate memory of size mem_size
  if (S->mem_used > MEM_MAX - mem_size)
  { // In case the code is used within a code base
    exit(1);
  }
  int *store = (S->DB + S->mem_used); // Compute a pointer to the new memory location
  S->mem_used += mem_size;            // Update the size of the used memory
  return store;
} // Return the pointer
int *addClause(struct solver *S, int *in, int size, int irr)
{                                           // Adds a clause stored in *in of size size
  int i, used = S->mem_used;                // Store a pointer to the beginning of the clause
  int *clause = getMemory(S, size + 3) + 2; // Allocate memory for the clause in the database
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
//Myabe we will use it later.
int compute_lbd(struct solver *S, int *clause) {
    int levels[S->nVars + 1];
    memset(levels, 0, sizeof(levels)); // Initialize levels array to 0
    int lbd = 0;
    
    for (int i = 0; clause[i]; i++) {
        int var = abs(clause[i]);
        int level = S->decision_level[var];
        if (level > 0 && level <= S->nVars && !levels[level]) {
            levels[level] = 1;
            lbd++;
        }
    }
    return lbd;
}
int is_lbd_2(struct solver *S, int *clause) {
    int levels[3] = {0, 0, 0}; // Array to store up to 3 unique decision levels
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
    int *watch = &S->first[i]; // Get the pointer to the first watched clause
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
    int *clause = &S->DB[head];
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
      
    }
    int add_clause = reduce_funcs[S->config.reduce_p](S, count, k, size, S->config.clause_size, lbd);
    if (add_clause || lbd_2)
    {
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
// Utility function to split the nodes of the given list into two halves.
// Function to find the middle of the linked list
int getMiddle(struct solver *S, int head)
{
  int slow = head;
  int fast = S->prev[head];

  // Move fast by 2 and slow by 1
  // Finally slow will point to middle node
  while (fast != 0)
  {
    fast = S->prev[fast];
    if (fast != 0)
    {
      slow = S->prev[slow];
      fast = S->prev[fast];
    }
  }
  return slow;
}

// Function to merge two halves
int sortedMerge(struct solver *S, int left, int right)
{
  if (left == 0)
    return right;
  if (right == 0)
    return left;

  // Pick either a or b, and recur
  if (S->scores[left] >= S->scores[right])
  {
    S->prev[left] = sortedMerge(S, S->prev[left], right);
    S->next[S->prev[left]] = left;
    S->next[left] = 0; // Might be the last element now
    return left;
  }
  else
  {
    S->prev[right] = sortedMerge(S, left, S->prev[right]);
    S->next[S->prev[right]] = right;
    S->next[right] = 0; // Might be the last element now
    return right;
  }
}

// Function to do merge sort
void mergeSort(struct solver *S, int *headRef)
{
  int head = *headRef;
  if ((head == 0) || (S->prev[head] == 0))
    return;

  // Get the middle of the list
  int middle = getMiddle(S, head);

  // Split the list into two halves
  int left = head;
  int right = S->prev[middle];
  S->prev[middle] = 0;

  // Recursively sort the halves
  mergeSort(S, &left);
  mergeSort(S, &right);

  // Merge the sorted halves
  *headRef = sortedMerge(S, left, right);
}

// Caller function for sorting
void sort_variables(struct solver *S)
{
  mergeSort(S, &S->head);

  // Correctly assign next pointers from prev
  int current = S->head;
  int last = 0;
  while (current != 0)
  {
    S->next[current] = last;
    last = current;
    current = S->prev[current];
  }
}
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
  int *p = (S->DB + S->reason[abs(lit)] - 1); // Get the reason of lit(eral)
  while (*(++p))                              // While there are literals in the reason
    if ((S->falses[*p] ^ MARK) && !implied(S, *p))
    { // Recursively check if non-MARK literals are implied
      S->falses[lit] = IMPLIED - 1;
      return 0;
    } // Mark and return not implied (denoted by IMPLIED - 1)
  S->falses[lit] = IMPLIED;
  return 1;
} // Mark and return that the literal is implied

int *analyze(struct solver *S, int *clause)
{ // Compute a resolvent from falsified clause
  S->res++;
  S->nConflicts++; // Bump restarts and update the statistic
  while (*clause)
  {
    if (S->config.br_p == BR_VMTF || S->config.br_p == BR_CHB )
      bump(S, *(clause++)); // MARK all literals in the falsified clause
    if (S->config.br_p == BR_VSIDS)
      increment(S, *(clause++));
    // todo lrb
  }
  while (S->reason[abs(*(--S->assigned))])
  { // Loop on variables on falseStack until the last decision
    if (S->falses[*S->assigned] == MARK)
    {                                       // If the tail of the stack is MARK
      int *check = S->assigned;             // Pointer to check if first-UIP is reached
      while (S->falses[*(--check)] != MARK) // Check for a MARK literal before decision
        if (!S->reason[abs(*check)])
          goto build;                                // Otherwise it is the first-UIP so break
      clause = S->DB + S->reason[abs(*S->assigned)]; // Get the reason and ignore first literal
      while (*clause)
      { // is it also good for VSIDS : incrementing all the variables that are in the clause that caused a conflict ? ask sami
        if (S->config.br_p == BR_VMTF || S->config.br_p == BR_CHB )
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
  int *p = S->processed = S->assigned; // Loop from tail to front
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

int propagate(struct solver *S)
{                                             // Performs unit propagation
  int forced = S->reason[abs(*S->processed)]; // Initialize forced flag
  while (S->processed < S->assigned)
  {                              // While unprocessed false literals
    int lit = *(S->processed++); // Get first unprocessed literal
    int *watch = &S->first[lit]; // Obtain the first watch pointer
    while (*watch != END)
    {                                     // While there are watched clauses (watched by lit)
      int i, unit = 1;                    // Let's assume that the clause is unit
      int *clause = (S->DB + *watch + 1); // Get the clause from DB
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
          update_chb(S,abs(clause[0]),0.9);
        } // A unit clause is found, and the reason is set
        
        else
        {
          if (forced)
            return UNSAT;                  // Found a root level conflict -> UNSAT
          int *lemma = analyze(S, clause); // Analyze the conflict return a conflict clause
          if (!lemma[1])
            forced = 1; // In case a unit clause is found, set forced flag
          assign(S, lemma, forced);
          for (int i = 0; lemma[i] != 0; i++) 
          {
            int var = abs(lemma[i]);
            S->config.lastConflict[var] = S->nConflicts;
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

void initCDCL(struct solver *S, int n, int m)
{
  initialize_chb(S,n);
  setup_functions();
  if (n < 1)
    n = 1;                     // The code assumes that there is at least one variable
  S->nVars = n;                // Set the number of variables
  S->nClauses = m;             // Set the number of clauases
  S->mem_used = 0;             // The number of integers allocated in the DB
  S->nLemmas = 0;              // The number of learned clauses -- redundant means learned
  S->nConflicts = 0;           // Under of conflicts which is used to updates scores
  S->maxLemmas = MAX_LEMMAS;   // Initial maximum number of learnt clauses  //2000 default
  S->fast = S->slow = 1 << 24; // Initialize the fast and slow moving averages
  S->decision_counter = -1;
  S->config.br_p = BR_VSIDS;
  S->config.reduce_p = RED_DEFAULT;
  S->config.rest_p = REST_DEFAULT;
  S->config.conflicts = 0;
  S->config.luby_base = 64;
  S->config.luby_index = 0;
  S->config.geo_factor = 1.1;
  S->config.geo_max = 100;
  S->config.arith_max = 100;
  S->config.arith_reason = 500;
  S->config.decay_factor = 0.99;
  S->config.decay_thresh_hold = 1; // like in minisat
  S->config.clause_size = 2;
  S->config.max_lbd     = 6;
  S->res = 0;
  S->DB = (int *)malloc(sizeof(int) * MEM_MAX); // Allocate the initial database
  memset(S->DB, 0, MEM_MAX * sizeof(int));
  S->model = getMemory(S, n + 1); // Full assignment of the (Boolean) variables (initially set to false)
  S->scores = (float *)getMemory(S, n + 1);
  S->decision_level = getMemory(S, n + 1); //ADDED
  for (int i = 0; i <= n; i++) S->decision_level[i] = -1;
  S->next = getMemory(S, n + 1);       // Next variable in the heuristic order
  S->prev = getMemory(S, n + 1);       // Previous variable in the heuristic order
  S->buffer = getMemory(S, n);         // A buffer to store a temporary clause
  S->reason = getMemory(S, n + 1);     // Array of clauses
  S->falseStack = getMemory(S, n + 1); // Stack of falsified literals -- this pointer is never changed
  S->forced = S->falseStack;           // Points inside *falseStack at first decision (unforced literal)
  S->processed = S->falseStack;        // Points inside *falseStack at first unprocessed literal
  S->assigned = S->falseStack;         // Points inside *falseStack at last unprocessed literal
  S->falses = getMemory(S, 2 * n + 1);
  S->falses += n; // Labels for variables, non-zero means false
  S->first = getMemory(S, 2 * n + 1);
  S->first += n;            // Offset of the first watched clause
  S->DB[S->mem_used++] = 0; // Make sure there is a 0 before the clauses are loaded.
  
  int i;
  for (i = 1; i <= n; i++)
  { // Initialize the main datastructes:
    S->prev[i] = i - 1;
    S->next[i - 1] = i;                             // the float-linked list for variable-move-to-front,
    S->model[i] = S->falses[-i] = S->falses[i] = 0; // the model (phase-saving), the false array,
    S->first[i] = S->first[-i] = END;
    S->scores[i] = 0;
  } // and first (watch pointers).
  S->head = n;
} // Initialize the head of the float-linked list

void read_until_new_line(FILE *input)
{
  int ch;
  while ((ch = getc(input)) != '\n')
    if (ch == EOF)
    {
      printf("parse error: unexpected EOF");
      exit(1);
    }
}

int parse(struct solver *S, char *filename)
{ // Parse the formula and initialize
  int tmp;
  FILE *input;
  int close = 1;
  if (strcmp(filename + strlen(filename) - 3, ".xz"))
    input = fopen(filename, "r"); // Open file
  else
  {
    char *cmd = malloc(strlen(filename) + 20);
    sprintf(cmd, "xz -c -d %s", filename);
    input = popen(cmd, "r");
    close = 2;
    free(cmd);
  } // Open pipe
  while ((tmp = getc(input)) == 'c')
    read_until_new_line(input);
  ungetc(tmp, input);
  do
  {
    tmp = fscanf(input, "p cnf %d %d", &S->nVars, &S->nClauses); // Find the first non-comment line
    if (tmp > 0 && tmp != EOF)
      break;
    tmp = fscanf(input, "%*s\n");
  } // In case a commment line was found
  while (tmp != 2 && tmp != EOF); // Skip it and read next line

  initCDCL(S, S->nVars, S->nClauses); // Allocate the main datastructures
  int nZeros = S->nClauses, size = 0; // Initialize the number of clauses to read
  while (nZeros > 0)
  { // While there are clauses in the file
    int ch = getc(input);
    if (ch == ' ' || ch == '\n')
      continue;
    if (ch == 'c')
    {
      read_until_new_line(input);
      continue;
    }
    ungetc(ch, input);
    int lit = 0;
    tmp = fscanf(input, " %i ", &lit);

    if (!lit)
    {                                                     // If reaching the end of the clause
      int *clause = addClause(S, S->buffer, size, 1);     // Then add the clause to data_base
      if (!size || ((size == 1) && S->falses[clause[0]])) // Check for empty clause or conflicting unit
        return UNSAT;                                     // If either is found return UNSAT
      if ((size == 1) && !S->falses[-clause[0]])
      { // Check for a new unit
        assign(S, clause, 1);
      } // Directly assign new units (forced = 1)
      size = 0;
      --nZeros;
    } // Reset buffer
    else
      S->buffer[size++] = lit;
  } // Add literal to buffer
  if (close == 1)
    fclose(input); // Close the formula file
  if (close == 2)
    pclose(input); // Close the formula pipe
  return SAT;
} // Return that no conflict was observed
int *get_unassigned_lits(struct solver S, int *size)
{
  int *lits = malloc(S.nVars * sizeof(int));
  int j = 0;
  for (int i = 1; i < S.nVars; i++)
  {
    if (S.falses[i] == 0 && S.falses[-i] == 0)
    {
      lits[j] = i;
      j++;
    }
  }
  *size = j;
  return lits;
}
int *get_assigned_lits(struct solver S, int *size)
{
  int *lits = malloc(S.nVars * sizeof(int));
  int j = 0;
  for (int i = 1; i < S.nVars; i++)
  {
    if (S.falses[i] != 0 || S.falses[-i] != 0)
    {
      if (S.falses[i] != 0)
        lits[j] = i;
      else
        lits[j] = -i;
      j++;
    }
  }
  *size = j;
  return lits;
}
int *get_reasons(struct solver S)
{
  int *reasons = malloc((S.nVars + 1) * sizeof(int));
  memcpy(reasons, S.reason, (S.nVars + 1) * sizeof(int));
  return reasons;
}

void unassign_last_decision(struct solver *S)
{
  int lit = *(--S->assigned);
  if (S->falses[lit] != IMPLIED)
    unassign(S, lit,1);
}
void unassign_all(struct solver *S)
{
  while (S->assigned != S->falseStack)
  {
    unassign_last_decision(S);
  }
}

void reset_solver(struct solver *S)
{
  unassign_all(S);
  // restart(S);
}

int get_unassigned(struct solver S)
{
  for (int i = 1; i < S.nVars; i++)
  {
    if (!(S.falses[i] != 0 || S.falses[-i] != 0))
    {
      return i;
    }
  }
  return 0;
}

void picosat_proof(struct solver S)
{
  for (int i = 1; i < S.nVars + 1; i++)
  {
    int m = 1;
    if (S.model[i] == 0)
      m = -1;
    printf("-a %d ", i * m);
  }
  printf("\n\n");
}
void show_result(struct solver S)
{
  log_message(LOG_LEVEL_INFO, "Results :");
  for (int i = 0; i < S.nVars + 1; i++)
  {
    printf("[%d,%d] ", i, (int)S.model[i]);
  }
  printf("\n\n");
  log_message(LOG_LEVEL_INFO, "Pico Sat proof:");
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
  log_message(LOG_LEVEL_INFO, "Solver data base");
  int partition[9] = {S.nVars + 1, S.nVars + 1, S.nVars + 1, S.nVars, S.nVars + 1, S.nVars + 1, 2 * S.nVars + 1, 2 * S.nVars + 1, __INT_MAX__};
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
  log_message(LOG_LEVEL_INFO, "Solver stats");
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
void set_solver_config(struct solver *S, config_t config)
{
  S->config = config;
}
void set_solver_br(struct solver *S, enum branching_policy br)
{
  S->config.br_p = br;
}
void set_solver_rest(struct solver *S, enum restart_policy rest)
{
  S->config.rest_p = rest;
}
void set_solver_red(struct solver *S, enum reduce_policy red)
{
  S->config.reduce_p = red;
}
void log_decision(int lit, int level) {
    log_message(LOG_LEVEL_DEBUG,"Decide: Literal %d, Decision Level: %d", lit, level);
}

void log_propagation(int lit, int level) {
    log_message(LOG_LEVEL_DEBUG,"Propagate: Literal %d, Decision Level: %d", lit, level);
}

void log_unassign(int lit, int level) {
    log_message(LOG_LEVEL_DEBUG,"Unassign: Literal %d, Decision Level: %d", lit, level);
}

void log_conflict_analysis(int lit, int level) {
    log_message(LOG_LEVEL_DEBUG,"Conflict Analysis: Literal %d, Decision Level: %d", lit, level);
}
void print_clause(struct solver *S, int *clause) {
    for (int i = 0; clause[i]; i++) {
        printf("%d@%d ", clause[i], S->decision_level[abs(clause[i])]);
    }
    printf("\n");
}