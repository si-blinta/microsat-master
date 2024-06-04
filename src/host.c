#include "microsat.h"
//#include "hostTools.h"
#include "log.h"
#include <stdint.h>
#include "time.h"
#define DPU_BINARY "bin/dpu"
double power(double base, int exponent) {
    double result = 1.0;
    for(int i = 0; i < exponent; i++) {
        result *= base;
    }
    return result;
}
double luby(double y, int x) {

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

int main(int argc, char **argv)
{
  for(int i = 0 ; i < 100 ; i++)
    printf("%f ",luby((double)100,i));
  //if (argc < 3)
    //abort();
  /*struct dpu_set_t set;
  uint32_t nb_dpus = NB_DPU;//atoi(argv[2]); 
  HOST_TOOLS_allocate_dpus(&set,&nb_dpus);
  HOST_TOOLS_compile(1);
  dpu_load(set,DPU_BINARY,NULL);
  srand(time(NULL));
  //HOST_TOOLS_pure_portfolio(argv[1],set);
  HOST_TOOLS_divide_and_conquer(argv[1],set);*/


}