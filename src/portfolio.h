#ifndef PORTFOLIO_H
#define PORTFOLIO_H
#include "microsat.h"
#define MAX_VARS 10
struct solver* portfolio_alloc(int nVars);
void portfolio_populate(struct solver* from, struct solver* to);
void portfolio_combination(struct solver* comb,int nVars);
void portfolio_print_all_info(struct solver* comb);
struct solver* portfolio_generate(struct solver* from);
int portfolio_get_length(struct solver* from);
#endif