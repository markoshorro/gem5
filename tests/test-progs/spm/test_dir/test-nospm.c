/*
 * Universidade da Coruña. 2015
 *
 * Marcos Horro Varela
 *
 * Testing pseudo_instructions
 */

#define KBYTE 1024
#define MBYTE 1024*1024
#define DEBUG 0

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "m5/m5op.h"

int main(int argc, char** argv)
{
  printf("test program start\n");

  int *p = (int *) malloc(sizeof(int)*16*KBYTE);

  int i = 0;

  if (DEBUG) {
    printf("Virtual address pointer vaddr: %lu\n", p);
    exit(0);
  }

  m5_reset_stats(0,0);
  for (i=0; i<16*KBYTE; i++) {
    p[i] = i; 
    //printf("dir: %d\n", p[i]);
  }
  m5_dump_stats(0,0);

  printf("test program finish\n"); 
}
