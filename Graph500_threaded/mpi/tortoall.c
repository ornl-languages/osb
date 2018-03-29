/* Copyright (c) 2017-2018  UT-Battelle, LLC. All rights reserved.         */

/*
 * test shmem_long_or_to_all
 */
#include <stdio.h>
#include <stdlib.h>
#include <shmem.h>

int 
main( int argc, char *argv[]) 
{

  start_pes(0);
  int n_pes = shmem_n_pes();
  int my_pe = shmem_my_pe();

  if (my_pe == 0) {
    printf("n_pes = %d \n", n_pes );
    };

  static long pSync[ _SHMEM_REDUCE_SYNC_SIZE ];
  static long pWrk[ 2+1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE ];

  {
   int i = 0;
   for(i=0; i < _SHMEM_REDUCE_SYNC_SIZE; i++) {
        pSync[i] = _SHMEM_SYNC_VALUE;
        };
   for(i=0; i < sizeof(pWrk)/sizeof(long); i++) {
        pWrk[i] = 0;
        };
   }

  static long lvalue; 
  static long gvalue;
  {
  int pe_start = 0;
  int logpe_stride = 0;
  int pe_size = n_pes;
  int nreduce = 1;

  lvalue = my_pe;
  gvalue = 0;

  shmem_long_or_to_all( &gvalue, &lvalue, nreduce, 
         pe_start, logpe_stride, pe_size, pWrk, pSync );
  }


  if (my_pe == 0) {
    printf("n_pes = %d gvalue = %ld \n", n_pes, gvalue );
    };

  exit(0);
}
