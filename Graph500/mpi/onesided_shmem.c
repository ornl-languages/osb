/* Copyright (C) 2014  Oak Ridge National Laboratory                       */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Ed D'Azevedo                                                  */

#include "common.h"
#include "onesided.h"

#ifdef USE_OPENSHMEM
#include <shmem.h>
#include "shmem_lib.h"
#else
#include <mpi.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef EMULATE_ONE_SIDED

/* One-sided wrapper to use OpenSHMEM */

#ifndef IS_LONG
#define   IS_LONG(datatype) (                           \
                (datatype == MPI_LONG) ||               \
                (datatype == MPI_UNSIGNED_LONG) ||      \
                (datatype == MPI_INT64_T) ||            \
                (datatype == MPI_UINT64_T)              \
                )
#endif

#ifndef IS_INT
#define   IS_INT(datatype) (                            \
                (datatype == MPI_INT) ||                \
                (datatype == MPI_UNSIGNED) ||           \
                (datatype == MPI_INT32_T) ||            \
                (datatype == MPI_UINT32_T)              \
                )
#endif


#ifndef FALSE
#define FALSE (1 == 0)
#endif

#ifndef TRUE
#define TRUE (1 == 1)
#endif



/* Gather from one array into another. */
struct gather {
  void* input;
  size_t elt_size;
  void* output;
#ifdef USE_OPENSHMEM
  int datatype; 
#else
  MPI_Datatype datatype;
#endif
  int valid;
#ifdef USE_OPENSHMEM
  int win;
#else
  MPI_Win win;
#endif
};

#ifdef USE_OPENSHMEM
gather* init_gather(void* input, size_t input_count, size_t elt_size, void* output, size_t output_count, size_t nrequests_max, int dt) {
#else
gather* init_gather(void* input, size_t input_count, size_t elt_size, void* output, size_t output_count, size_t nrequests_max, MPI_Datatype dt) {
#endif

  gather* g = (gather*)xmalloc(sizeof(gather));
  
  g->input = input;
  g->elt_size = elt_size;
  g->output = output;
  g->datatype = dt;
  g->valid = 0;

#ifdef USE_OPENSHMEM
  assert( shmem_addr_accessible_all( input ) );
  shmem_barrier_all();
#else
  MPI_Win_create(input, input_count * elt_size, elt_size, MPI_INFO_NULL, MPI_COMM_WORLD, &g->win);
#endif
  return g;
}

void destroy_gather(gather* g) {
  assert( g != NULL );
  assert(!g->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_free(&g->win);
#endif
  free( g );
 
}

void begin_gather(gather* g) {
  assert( g != NULL );
  assert(!g->valid);
  g->valid = 1;
#ifdef USE_OPENSHMEM
  shmem_quiet();
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOPRECEDE | MPI_MODE_NOPUT, g->win);
#endif
}
/* 
 * -------------------------------------------------
 * Get data from a memory window on a remote process
 * Synopsis
 * MPI_Get copies data from the target memory to the  origin
 * 
 * int MPI_Get(void *origin_addr, int origin_count, MPI_Datatype
 *             origin_datatype, int target_rank, MPI_Aint target_disp,
 *             int target_count, MPI_Datatype target_datatype, MPI_Win
 *             win)
 * -------------------------------------------------
 */

void add_gather_request(gather* g, size_t local_idx, int remote_rank, size_t remote_idx, size_t req_id) {
  assert(g != NULL );
  assert(g->valid);
#pragma omp critical
  {
#ifndef USE_OPENSHMEM
 {
  MPI_Get(g->output + local_idx * g->elt_size, 1, g->datatype, remote_rank, remote_idx, 1, g->datatype, g->win);
 }
#else
  {
  size_t len = 1;
  int pe = remote_rank;
  /*
   * --------------------------------------------------------------
   * non-standard to perform pointer arithmetic on (void *) pointer
   * --------------------------------------------------------------
   */
  char *g_output = (char *) g->output;
  char *g_input = (char *) g->input;

  /* efd july 27, note 1st argument in MPI_Get is "origin_addr" */

  void *origin_addr = (void *) (g_output + local_idx * g->elt_size);
  void *target_addr =  (void *) (g_input + remote_idx * g->elt_size);

  /* MPI_Get, gets data from target memory to the origin */
  void *src = target_addr;
  void *dest = origin_addr;



#ifdef USE_DEBUG
  assert( (0 <= pe) && (pe < shmem_n_pes()) );
  assert( g->elt_size > 0);

  assert( shmem_pe_accessible(pe) );
  assert( shmem_addr_accessible( src, pe ) );
  
#endif

  shmem_getmem( (void *) dest, (void *) src, len * g->elt_size, pe );

#ifdef USE_DEBUG
  shmem_quiet();
#endif

  }
#endif
 
  }
}

void end_gather(gather* g) {
  assert( g != NULL );
  assert(g->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOSUCCEED, g->win);
#endif
  g->valid = 0;
}

/* Scatter a constant to various locations in an array. */
struct scatter_constant {
  void* array;
  size_t elt_size;
  void* constant;
#ifdef USE_OPENSHMEM
  int datatype;
#else
  MPI_Datatype datatype;
#endif
  int valid;
#ifdef USE_OPENSHMEM
  int   win;
#else
  MPI_Win win;
#endif
};

#ifdef USE_OPENSHMEM
scatter_constant* init_scatter_constant(void* array, size_t array_count, size_t elt_size, void* constant, size_t nrequests_max, int dt) {
#else
scatter_constant* init_scatter_constant(void* array, size_t array_count, size_t elt_size, void* constant, size_t nrequests_max, MPI_Datatype dt) {
#endif

  scatter_constant* sc = (scatter_constant*)xmalloc(sizeof(scatter_constant));

  sc->array = array;
  sc->elt_size = elt_size;
  sc->constant = constant;
  sc->datatype = dt;
  sc->valid = 0;
#ifdef USE_OPENSHMEM
  assert( shmem_addr_accessible_all( array ) );
  shmem_barrier_all();
#else
  MPI_Win_create(array, array_count * elt_size, elt_size, MPI_INFO_NULL, MPI_COMM_WORLD, &sc->win);
#endif

  return sc;
}

void destroy_scatter_constant(scatter_constant* sc) {
  assert( sc != NULL );
  assert(!sc->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_free(&sc->win);
#endif
  free(sc);
}

void begin_scatter_constant(scatter_constant* sc) {
  assert( sc != NULL );
  assert(!sc->valid);
  sc->valid = 1;
#ifdef USE_OPENSHMEM
  shmem_quiet();
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOPRECEDE, sc->win);
#endif
}

/*
 * ------------------------------------------------
 * Put data into a memory window on a remote process
 * Synopsis
 * 
 * int MPI_Put(const void *origin_addr, int origin_count, MPI_Datatype
 *             origin_datatype, int target_rank, MPI_Aint target_disp,
 *             int target_count, MPI_Datatype target_datatype, MPI_Win
 *             win)
 * ------------------------------------------------
 */


void add_scatter_constant_request(scatter_constant* sc, int remote_rank, size_t remote_idx, size_t req_id) {
  assert( sc != NULL );
  assert(sc->valid);
#pragma omp critical
  {

#ifndef USE_OPENSHMEM
  {
  MPI_Put(sc->constant, 1, sc->datatype, remote_rank, remote_idx, 1, sc->datatype, sc->win);
  }
#else
  {
  size_t len = 1;
  int pe = remote_rank;
  /*
   * --------------------------------------------------------------
   * non-standard to perform pointer arithmetic on (void *) pointer
   * --------------------------------------------------------------
   */
  
  /* efd july 27, note 1st argument to MPI_Put is origin_addr */
  char *sc_array = (char *) sc->array;
  void *target_addr = (void *) (sc_array  + remote_idx*sc->elt_size);
  void *origin_addr =  (void *) (sc->constant);

  void *src = origin_addr;
  void *dest = target_addr;



#ifdef USE_DEBUG
  assert( (0 <= pe) && (pe < shmem_n_pes()) );
  assert( sc->elt_size > 0 );

  assert( shmem_pe_accessible(pe) );
  assert( shmem_addr_accessible( dest, pe ) );
#endif

  shmem_putmem( (void *) dest, (void *) src, len * sc->elt_size, pe );

#ifdef USE_DEBUG
  shmem_quiet();
#endif

  }
#endif

 };
}

void end_scatter_constant(scatter_constant* sc) {
  assert( sc != NULL );
  assert(sc->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOSUCCEED | MPI_MODE_NOSTORE, sc->win);
#endif
  sc->valid = 0;
}

/* Scatter values to various locations in an array using MPI_REPLACE. */
struct scatter {
  void* array;
  size_t elt_size;
  size_t request_count;
  size_t nrequests_max;
  char* send_data;
#ifdef USE_OPENSHMEM
  int datatype;
#else
  MPI_Datatype datatype;
#endif
  int valid;

#ifdef USE_OPENSHMEM
  int win;
#else
   MPI_Win win;
#endif

};

#ifdef USE_OPENSHMEM
scatter* init_scatter(void* array, size_t array_count, size_t elt_size, size_t nrequests_max, int dt) {
#else
scatter* init_scatter(void* array, size_t array_count, size_t elt_size, size_t nrequests_max, MPI_Datatype dt) {
#endif
  scatter* sc = (scatter*)xmalloc(sizeof(scatter));


  sc->array = array;
  sc->elt_size = elt_size;
  sc->request_count = 0;
  sc->nrequests_max = nrequests_max;
  sc->send_data = (char *) xmalloc(nrequests_max * elt_size);

  sc->datatype = dt;
  sc->valid = 0;
#ifdef USE_OPENSHMEM
  assert( sc->elt_size > 0);
  assert( sc-> send_data != NULL );
  assert( shmem_addr_accessible_all(array) );
  shmem_barrier_all();
#else
  MPI_Win_create(array, array_count * elt_size, elt_size, MPI_INFO_NULL, MPI_COMM_WORLD, &sc->win);
#endif

  return sc;
}

void destroy_scatter(scatter* sc) {
  assert( sc != NULL );
  assert(!sc->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_free(&sc->win);
#endif

  free(sc->send_data);
  free(sc);
}

void begin_scatter(scatter* sc) {
  assert( sc != NULL );
  assert(!sc->valid);
  sc->valid = 1;
  sc->request_count = 0;
#ifdef USE_OPENSHMEM
  shmem_quiet();
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOPRECEDE, sc->win);
#endif
}

void add_scatter_request(scatter* sc, const char* local_data, int remote_rank, size_t remote_idx, size_t req_id) {
  assert( sc != NULL );
  assert(sc->valid);
  assert(sc->request_count < sc->nrequests_max);
  memcpy(sc->send_data + sc->request_count * sc->elt_size, local_data, sc->elt_size);
#pragma omp critical
  {
#ifndef USE_OPENSHMEM
  {
  MPI_Put(sc->send_data + sc->request_count * sc->elt_size, 1, sc->datatype, remote_rank, remote_idx, 1, sc->datatype, sc->win);
  }
#else

  {
   /* 
    * --------------------------------------------------------------
      MPI_Put(const void *origin_addr, int origin_count, MPI_Datatype
              origin_datatype, int target_rank, MPI_Aint target_disp,
              int target_count, MPI_Datatype target_datatype, MPI_Win win)
     
    MPI_Put transfers origin_count successive entries of the type
    specified by origin_datatype, starting at address origin_addr on the
    origin node to the target node specified by the win, target_rank pair.
    * --------------------------------------------------------------
    */

  size_t len = 1;
  int pe = remote_rank;
  /*
   * --------------------------------------------------------------
   * non-standard to perform pointer arithmetic on (void *) pointer
   * --------------------------------------------------------------
   */
  char *sc_array = (char *) sc->array;
  char *sc_send_data = (char *) sc->send_data;

  void *origin_addr =  (void *) (sc_send_data + sc->request_count * sc->elt_size);
  void *target_addr = (void *) (sc_array  + remote_idx * sc->elt_size);

  void *src = origin_addr;
  void *dest = target_addr;

#ifdef USE_DEBUG
  assert( (0 <= pe) && (pe < shmem_n_pes()) );
  assert( shmem_pe_accessible(pe) );
  assert( shmem_addr_accessible( dest, pe ));
  assert( sc->elt_size > 0);
#endif

  shmem_putmem( (void *) dest, (void *) src, len * sc->elt_size, pe );
#ifdef USE_OPENSHMEM
  shmem_quiet();
#endif

  }
#endif
  }
  ++sc->request_count;
}

void end_scatter(scatter* sc) {
  assert( sc != NULL );
  assert(sc->valid);
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Win_fence(MPI_MODE_NOSUCCEED | MPI_MODE_NOSTORE, sc->win);
#endif
  sc->valid = 0;
}

#endif /* !EMULATE_ONE_SIDED */
