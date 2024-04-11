#include <stdio.h>
#include "microsat.h"
#include <mram.h>
__host uint32_t buffer_size;
__mram_noinit char buffer[1024];
int main(){
    __dma_aligned char cache[1024];
    mram_read(buffer,cache,1024);
    cache[1023]=0;
    printf("%s\n",cache);
    return 0;

}