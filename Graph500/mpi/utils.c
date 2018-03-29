/* Copyright (C) 2010 The Trustees of Indiana University.                  */
/* Copyright (C) 2014  Oak Ridge National Laboratory.                      */
/*                                                                         */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Jeremiah Willcock                                             */
/*           Andrew Lumsdaine                                              */
/*           Ed D'Azevedo                                                  */

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#include "common.h"

#ifdef USE_OPENSHMEM
#include <shmem.h>
#include "shmem_lib.h"

#else

#include <mpi.h>

#endif

int rank, size;
#ifdef SIZE_MUST_BE_A_POWER_OF_TWO
int lgsize;
#endif


#ifdef USE_OPENSHMEM
void *packed_edge_mpi_type = 0;
#else
MPI_Datatype packed_edge_mpi_type;
#endif

void setup_globals() {

#ifdef USE_OPENSHMEM
  rank = shmem_my_pe();
  size = shmem_n_pes();
#else
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
#endif

#ifdef SIZE_MUST_BE_A_POWER_OF_TWO
  if (/* Check for power of 2 */ (size & (size - 1)) != 0) {
    fprintf(stderr, "Number of processes %d is not a power of two, yet SIZE_MUST_BE_A_POWER_OF_TWO is defined in main.cpp.\n", size);

#ifdef USE_OPENSHMEM
   exit(1);
#else
    MPI_Abort(MPI_COMM_WORLD, 1);
#endif
  }
  for (lgsize = 0; lgsize < size; ++lgsize) {
    if ((1 << lgsize) == size) break;
  }
  assert (lgsize < size);
#endif


#ifndef USE_OPENSHMEM
  int blocklengths[] = {1, 1, 1};
  MPI_Aint displs[] = {0, 0, 0};
  packed_edge temp;
  MPI_Aint temp_addr, fld_addr;
  MPI_Get_address(&temp, &temp_addr);
#ifdef GENERATOR_USE_PACKED_EDGE_TYPE
  MPI_Get_address(&temp.v0_low, &fld_addr); displs[0] = fld_addr - temp_addr;
  MPI_Get_address(&temp.v1_low, &fld_addr); displs[1] = fld_addr - temp_addr;
  MPI_Get_address(&temp.high,   &fld_addr); displs[2] = fld_addr - temp_addr;
  MPI_Type_create_hindexed(3, blocklengths, displs, MPI_UINT32_T, &packed_edge_mpi_type);
#else
  MPI_Get_address(&temp.v0, &fld_addr); displs[0] = fld_addr - temp_addr;
  MPI_Get_address(&temp.v1, &fld_addr); displs[1] = fld_addr - temp_addr;
  MPI_Type_create_hindexed(2, blocklengths, displs, MPI_INT64_T, &packed_edge_mpi_type);
#endif
  MPI_Type_commit(&packed_edge_mpi_type);


#endif
}

void cleanup_globals(void) {
#ifndef USE_OPENSHMEM
  MPI_Type_free(&packed_edge_mpi_type);
#endif
}

int lg_int64_t(int64_t x) { /* Round up */
  assert (x > 0);
  --x;
  int result = 0;
  while ((x >> result) != 0) ++result;
  return result;
}

/* These are in the graph generator. */
#if 0
void* xmalloc(size_t nbytes) {
  void* p = malloc(nbytes);
  if (!p) {
    fprintf(stderr, "malloc() failed for size %zu\n", nbytes);
    abort();
  }
  return p;
}

void* xcalloc(size_t n, size_t unit) {
  void* p = calloc(n, unit);
  if (!p) {
    fprintf(stderr, "calloc() failed for size %zu * %zu\n", n, unit);
    abort();
  }
  return p;
}
#endif

void* xrealloc(void* p, size_t nbytes) {
  p = realloc(p, nbytes);
  if (!p && nbytes != 0) {
    fprintf(stderr, "realloc() failed for size %zu\n", nbytes);
    abort();
  }
  return p;
}

void xMPI_Free_mem( void *p)
{
  if (p != NULL) {
#ifdef USE_OPENSHMEM
     shmem_barrier_all();
     shfree(p);
#else
     MPI_Free_mem(p);
#endif
     };
}

void* xMPI_Alloc_mem(size_t nbytes) {
  void* p;
#ifdef USE_OPENSHMEM
  {
  long lvalue = (long) nbytes;
  long gvalue = shmem_long_max_all(lvalue);
  assert( gvalue >= lvalue );
  p = (void *) shmalloc( (size_t) gvalue );
  }
#else
  MPI_Alloc_mem(nbytes, MPI_INFO_NULL, &p);
#endif
  if (nbytes != 0 && !p) {
    fprintf(stderr, "MPI_Alloc_mem failed for size %zu\n", nbytes);
    abort();
  }
  return p;
}
