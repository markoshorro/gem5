/**
 * trmm.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "m5/m5op.h"

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "trmm.h"

int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = 512;

  /* Variable declaration/allocation. */
  double alpha;

  double *A = (double *) malloc(ni*ni*sizeof(double));
  double *B = (double *) malloc(ni*ni*sizeof(double));
  
  /* Run kernel. */
  int i, j, k;
  alpha = 32412;
  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++) {
      A[i*ni + j] = ((double) i*j) / ni;
      B[i*ni + j] = ((double) i*j) / ni;
    }
  m5_reset_stats(0,0);
#pragma scop
  /*  B := alpha*A'*B, A triangular */
  for (i = 1; i <  ni; i++)
    for (j = 0; j < ni; j++)
      for (k = 0; k < i; k++)
        B[i*ni + j] += alpha * A[i*ni + k] * B[j*ni + k];
#pragma endscop
  m5_dump_stats(0,0);

  printf("%f\n", B[11]);
  
  free(A);
  free(B);
  
  return 0;
}
