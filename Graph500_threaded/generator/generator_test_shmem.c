/* Copyright (C) 2009-2010 The Trustees of Indiana University.             */
/* Copyright (c) 2017-2018  UT-Battelle, LLC. All rights reserved.         */
/*                                                                         */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Jeremiah Willcock                                             */
/*           Andrew Lumsdaine                                              */

#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stdio.h>
#include <assert.h>

#include <shmem.h>
#include "shmem_lib.h"
#include "make_graph.h"

int main(int argc, char* argv[]) {
  int log_numverts;
  int size, rank;
  unsigned long my_edges;
  unsigned long global_edges;
  double start, stop;
  size_t i;

#ifdef USE_OPENSHMEM
  start_pes(0);
#else
  MPI_Init(&argc, &argv);
#endif

  log_numverts = 16; /* In base 2 */
  if (argc >= 2) log_numverts = atoi(argv[1]);

#ifdef USE_OPENSHMEM
  size = shmem_n_pes();
  rank = shmem_my_pe();
#else
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif

  if (rank == 0) fprintf(stderr, "Graph size is %" PRId64 " vertices and %" PRId64 " edges\n", INT64_C(1) << log_numverts, INT64_C(16) << log_numverts);

  /* Start of graph generation timing */
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Barrier(MPI_COMM_WORLD);
#endif


#ifdef USE_OPENSHMEM
  start = shmem_wtime();
#else
  start = MPI_Wtime();
#endif
  int64_t nedges;
  packed_edge* result;
  make_graph(log_numverts, INT64_C(16) << log_numverts, 1, 2, &nedges, &result);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Barrier(MPI_COMM_WORLD);
#endif


#ifdef USE_OPENSHMEM
  stop = shmem_wtime();
#else
  stop = MPI_Wtime();
#endif
  /* End of graph generation timing */

  my_edges = nedges;

  for (i = 0; i < my_edges; ++i) {
    assert ((get_v0_from_edge(&result[i]) >> log_numverts) == 0);
    assert ((get_v1_from_edge(&result[i]) >> log_numverts) == 0);
  }
  
  free(result);

#ifdef USE_OPENSHMEM
  {
    /*
     * ------------------------
     * long integer type should be sufficient to hold maximum  number of edges
     * ------------------------
     */
  long lvalue = (long) my_edges;
  long gvalue = shmem_long_sum_all( lvalue );
  assert( my_edges >= 0);
  assert( lvalue <= gvalue );

  global_edges = (unsigned long) gvalue;
  }
#else
  MPI_Reduce(&my_edges, &global_edges, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
#endif
  if (rank == 0) {
    fprintf(stderr, "%lu edge%s generated in %fs (%f Medges/s on %d processor%s)\n", global_edges, (global_edges == 1 ? "" : "s"), (stop - start), global_edges / (stop - start) * 1.e-6, size, (size == 1 ? "" : "s"));
  }
#ifdef USE_OPENSHMEM
#else
  MPI_Finalize();
#endif
  return 0;
}
