/*
   This file is part of SHOMS.

   Copyright (C) 2014-2018, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SHOMS developers at:
   bakermb@ornl.gov

*/

#include <string.h>
#include <shoms.h>
#include <stdio.h>

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

/* Handy general utilities */

uint64_t print_count=0;

#ifdef DEBUG
#define DEBUG_ALLOCATIONS
#endif

#ifdef DEBUG_ALLOCATIONS
static uint64_t shmalloc_count = 0;
static uint64_t shfree_count = 0;

void print_mem_management_stats(parsed_options_t *input_options){
  if(MY_PE == 0){
    fprintf(input_options->output_file, "\nshmem_malloc calls: %lu\nshmem_free calls: %lu\n", shmalloc_count, shfree_count );
  }
}

#endif

static void touch_memory(void *mem, index_t size) {
  index_t page_size = sysconf(_SC_PAGESIZE);
  index_t *this_memory = (index_t *)mem;
  index_t size_increment = page_size / sizeof(index_t);
  index_t size_max = size / sizeof(index_t);
  for(index_t idx=0; idx < size_max; idx+=size_increment) {
    this_memory[idx] = 0;
  }
}

static inline index_t compute_indexes_per_pe(index_t test_size, index_t alignment){
  return ((test_size + alignment)-1)/alignment;
}

static void *allocate_distributed_buffer(index_t size, index_t alignment){
  index_t alloc_per_pe = compute_indexes_per_pe(size, alignment);
  void *allocation = shmem_malloc(alloc_per_pe*alignment);
  assert(allocation != NULL);
  touch_memory(allocation, size);
#ifdef DEBUG_ALLOCATIONS
  shmalloc_count++;
#endif
  return allocation;
}

void free_distributed_buffer(void *buffer){
  shmem_free(buffer);
#ifdef DEBUG_ALLOCATIONS
  shfree_count++;
#endif
}

static void *allocate_local_buffer(index_t size, index_t alignment){
  index_t alloc_per_pe = compute_indexes_per_pe(size, alignment);
  void *allocation = malloc(alloc_per_pe*alignment);
  assert(allocation != NULL);
  touch_memory(allocation, size);
  return allocation;
}

void init_distributed_16bit_bufffer(void **buffers,index_t size){
  *buffers = allocate_distributed_buffer(size, sizeof(int16_t));
}

void init_distributed_32bit_bufffer(void **buffers,index_t size){
  *buffers = allocate_distributed_buffer(size, sizeof(int32_t));
}

void init_distributed_64bit_bufffer(void **buffers,index_t size){
  *buffers = allocate_distributed_buffer(size, sizeof(int64_t));
}

#define declare_type_alloc(type) \
  void init_distributed_ ## type ## _buffer(void **buffers, index_t size){ \
  *buffers = allocate_distributed_buffer(size, sizeof(type));\
}

#define declare_type_alloc2(type_a, type_b)                                \
  void init_distributed_ ## type_a ## type_b ## _buffer(void **buffers, index_t size){ \
  *buffers = allocate_distributed_buffer(size, sizeof(type_a type_b));\
}

declare_type_alloc(char)
declare_with_common_types(declare_type_alloc)

static void *allocate_zeroed_buffer(index_t size, index_t alignment){
  void *new_buffer = allocate_distributed_buffer(size, alignment);
  memset(new_buffer, '\0', size);
  return new_buffer;
}

#define declare_zero_alloc(type) \
  void init_distributed_ ## type ## _buffer_zeroed(void **buffers, index_t size){ \
  *buffers = allocate_zeroed_buffer(size, sizeof(type));\
  }

#define declare_zero_alloc2(type_a, type_b)                                \
  void init_distributed_ ## type_a ## type_b ## _buffer_zeroed(void **buffers, index_t size){ \
  *buffers = allocate_zeroed_buffer(size, sizeof(type_a type_b));\
  }

declare_zero_alloc(char)
declare_with_common_types(declare_zero_alloc)

static void init_sym_and_local_t(void **buffers, index_t size, size_t alignment){
  sym_and_local_t *test_buffers = malloc(sizeof(sym_and_local_t));
  test_buffers->symetric = allocate_distributed_buffer(size, alignment);
  test_buffers->local = allocate_local_buffer(size, alignment);
  *buffers = (void *)test_buffers;
}

#define declare_sym_and_local_alloc(type) \
  void init_sym_and_local_ ## type(void **buffers, index_t size){\
    init_sym_and_local_t(buffers, size, sizeof(type));           \
  }

#define declare_sym_and_local_alloc2(type_a, type_b)\
  void init_sym_and_local_ ## type_a ## type_b(void **buffers, index_t size){\
    init_sym_and_local_t(buffers, size, sizeof(type_a type_b));         \
  }

void init_sym_and_local_32bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size, sizeof(int32_t));
}

void init_sym_and_local_64bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size, sizeof(int64_t));
}

void init_sym_and_local_128bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size, sizeof(int128_t));
}

declare_with_common_types(declare_sym_and_local_alloc)

static void init_collective_t(void **buffers, size_t test_size, size_t alignment){
  collective_buffers_t *test_buffers = malloc(sizeof(collective_buffers_t));
  test_buffers->target = allocate_distributed_buffer(test_size, alignment);
  test_buffers->source = allocate_distributed_buffer(test_size, alignment);
  index_t work_size = compute_indexes_per_pe(test_size, alignment)/2 + 1;
  if(work_size < _SHMEM_REDUCE_MIN_WRKDATA_SIZE){
    work_size = _SHMEM_REDUCE_MIN_WRKDATA_SIZE;
  }
  test_buffers->pWrk = allocate_distributed_buffer(work_size * alignment, alignment);
  test_buffers->pSync = allocate_distributed_buffer(_SHMEM_REDUCE_SYNC_SIZE * sizeof(long), sizeof(long));
  assert(test_buffers->target != NULL);
  assert(test_buffers->source != NULL);
  assert(test_buffers->pWrk != NULL);
  assert(test_buffers->pSync != NULL);
  for(int idx=0; idx < _SHMEM_REDUCE_SYNC_SIZE; idx++){
    test_buffers->pSync[idx] = _SHMEM_SYNC_VALUE;
  }

  (*buffers) = (void *)test_buffers;
}

void free_collective(void *doomed){
  collective_buffers_t *test_buffers = (collective_buffers_t *)doomed;
  free_distributed_buffer(test_buffers->source);
  free_distributed_buffer(test_buffers->target);
  free_distributed_buffer(test_buffers->pWrk);
  free_distributed_buffer(test_buffers->pSync);
  free(test_buffers);
}

#define declare_shmem_collective_alloc(type) \
  void init_collective_ ## type(void **buffers, index_t size){\
  init_collective_t(buffers, size, sizeof(type));\
  }

#define declare_shmem_collective_alloc2(type_a, type_b)                \
  void init_collective_ ## type_a ## type_b(void **buffers, index_t size){\
  init_collective_t(buffers, size, sizeof(type_a type_b));\
  }

#define declare_collective_types(alloc_function)\
  alloc_function(short)\
  alloc_function(int)\
  alloc_function(long)\
  alloc_function##2(long, long)


declare_collective_types(declare_shmem_collective_alloc)

//Strided functions need twice as much (or more!) memory to use the stride parameter

#define declare_strided_sym_and_local_alloc(type) \
  void init_strided_sym_and_local_ ## type(void **buffers, index_t size){\
  init_sym_and_local_t(buffers, size*2, sizeof(type));           \
  }

#define declare_strided_sym_and_local_alloc2(type_a, type_b)\
  void init_strided_sym_and_local_ ## type_a ## type_b(void **buffers, index_t size){\
  init_sym_and_local_t(buffers, size*2, sizeof(type_a type_b));         \
  }

void init_strided_sym_and_local_32bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size*2, sizeof(int32_t));
}

void init_strided_sym_and_local_64bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size*2, sizeof(int64_t));
}

void init_strided_sym_and_local_128bit(void **buffers, index_t size){
  init_sym_and_local_t(buffers, size*2, sizeof(int128_t));
}

declare_with_common_types(declare_strided_sym_and_local_alloc)

void free_sym_and_local_t(void *doomed){
  sym_and_local_t *doomed_ptr = (sym_and_local_t *)doomed;
  free_distributed_buffer(doomed_ptr->symetric);
  free(doomed_ptr->local);
  free(doomed_ptr);
}

void init_shmem_broadcast(void **buffers, index_t size){
  broadcast_buffers_t *broadcast_buffers = malloc(sizeof(broadcast_buffers_t));
  void *local_target = allocate_distributed_buffer(size, sizeof(int32_t));
  void *local_source = allocate_distributed_buffer(size, sizeof(int32_t));
  long *local_psync = allocate_distributed_buffer(_SHMEM_BCAST_SYNC_SIZE * sizeof(long), sizeof(long));
  assert(broadcast_buffers != NULL);
  assert(local_target != NULL);
  assert(local_source != NULL);
  assert(local_psync != NULL);
  for(int idx=0; idx < _SHMEM_BCAST_SYNC_SIZE; idx++){
    local_psync[idx] = _SHMEM_SYNC_VALUE;
  }
  broadcast_buffers->target=local_target;
  broadcast_buffers->source=local_source;
  broadcast_buffers->pSync=local_psync;
  *buffers = (void *)broadcast_buffers;
}

void init_shmem_collect32(void **buffers, index_t size){
  broadcast_buffers_t *broadcast_buffers = malloc(sizeof(broadcast_buffers_t));
  index_t indexes = compute_indexes_per_pe(size, N_PES);
  void *local_target = allocate_distributed_buffer(indexes*sizeof(int32_t)*N_PES, sizeof(int32_t));
  void *local_source = allocate_distributed_buffer(indexes*sizeof(int32_t), sizeof(int32_t));
  long *local_psync = allocate_distributed_buffer(_SHMEM_BCAST_SYNC_SIZE * sizeof(long), sizeof(long));
  assert(broadcast_buffers != NULL);
  assert(local_target != NULL);
  assert(local_source != NULL);
  assert(local_psync != NULL);
  for(int idx=0; idx < _SHMEM_BCAST_SYNC_SIZE; idx++){
    local_psync[idx] = _SHMEM_SYNC_VALUE;
  }
  broadcast_buffers->target=local_target;
  broadcast_buffers->source=local_source;
  broadcast_buffers->pSync=local_psync;
  *buffers = (void *)broadcast_buffers;
}

void init_shmem_collect64(void **buffers, index_t size){
  broadcast_buffers_t *broadcast_buffers = malloc(sizeof(broadcast_buffers_t));
  index_t indexes = compute_indexes_per_pe(size, N_PES);
  void *local_target = allocate_distributed_buffer(indexes*sizeof(int64_t)*N_PES, sizeof(int64_t));
  void *local_source = allocate_distributed_buffer(indexes*sizeof(int64_t), sizeof(int64_t));
  long *local_psync = allocate_distributed_buffer(_SHMEM_BCAST_SYNC_SIZE * sizeof(long), sizeof(int64_t));
  assert(broadcast_buffers != NULL);
  assert(local_target != NULL);
  assert(local_source != NULL);
  assert(local_psync != NULL);
  for(int idx=0; idx < _SHMEM_BCAST_SYNC_SIZE; idx++){
    local_psync[idx] = _SHMEM_SYNC_VALUE;
  }
  broadcast_buffers->target=local_target;
  broadcast_buffers->source=local_source;
  broadcast_buffers->pSync=local_psync;
  *buffers = (void *)broadcast_buffers;
}

void free_shmem_broadcast(void *buffers){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;

  free_distributed_buffer(target);
  free_distributed_buffer(source);
  free_distributed_buffer(pSync);
  free(broadcast_buffers);
}

void calculate_local_performance(test_results_t *test_results){
  test_results->avg_time_tick = (double)test_results->accumulated_time_tick / (double)(test_results->iterations);
  test_results->bandwidth_bytes_per_tick = (double)(test_results->transfered_data_bytes) / (double)(test_results->accumulated_time_tick);
}

long long pWork_min[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
long long pWork_max[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
long long pWork_sum[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
long pSync_min[_SHMEM_REDUCE_SYNC_SIZE];
long pSync_max[_SHMEM_REDUCE_SYNC_SIZE];
long pSync_sum[_SHMEM_REDUCE_SYNC_SIZE];
int performance_data_initalized=0;

long long max;
long long min;
long long bytes;
long long accumulated_time;

void reduce_common_results(test_results_t *test_results){
  if(performance_data_initalized==0){
    for(int idx=0; idx < _SHMEM_REDUCE_SYNC_SIZE; idx++){
      pSync_min[idx]=_SHMEM_SYNC_VALUE;
      pSync_max[idx]=_SHMEM_SYNC_VALUE;
      pSync_sum[idx]=_SHMEM_SYNC_VALUE;
    }
    shmem_barrier_all();
  }

  int npes = N_PES;
  
  max = (long long)test_results->max_time_tick;
  min = (long long)test_results->min_time_tick;
  accumulated_time = (long long)test_results->accumulated_time_tick;

  shmem_longlong_max_to_all(&max, &max, 1, 0, 0, npes, pWork_max, pSync_max);
  shmem_longlong_sum_to_all(&accumulated_time, &accumulated_time, 1, 0, 0, npes, pWork_sum, pSync_sum);
  shmem_longlong_min_to_all(&min, &min, 1, 0, 0, npes, pWork_min, pSync_min);

  test_results->max_time_tick = max;
  test_results->min_time_tick = min;
  test_results->accumulated_time_tick = accumulated_time;

  test_results->avg_time_tick = (double)test_results->accumulated_time_tick / (double)(test_results->iterations * npes);
}

void calculate_global_performance(test_results_t *test_results){
  bytes = (long long)test_results->transfered_data_bytes;
  reduce_common_results(test_results);
  shmem_longlong_sum_to_all(&bytes, &bytes, 1, 0, 0, N_PES, pWork_sum, pSync_sum);
  test_results->transfered_data_bytes = bytes;
  test_results->bandwidth_bytes_per_tick = (double)(test_results->transfered_data_bytes) / (double)(test_results->accumulated_time_tick);
}

void calculate_global_no_bw_performance(test_results_t *test_results){
  reduce_common_results(test_results);
  test_results->bandwidth_bytes_per_tick = (double)0.0;
}


#define elemental_put_type(type) \
index_t test_shmem_ ## type ## _p(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){ \
  if(MY_PE != 0) return 1; \
  type *test_buffer = (type *)buffers; \
  type value = 0; \
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type)); \
  for(index_t idx=0; idx < num_indexes; idx++){ \
    for(int jdx=0; jdx < REMOTE_PES; jdx++){\
      shmem_ ## type ## _p(&(test_buffer[idx]), value, jdx+1); \
    }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;  \
  shmem_quiet();\
  return 0;\
}

#define elemental_get_type(type) \
  index_t test_shmem_ ## type ## _g(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){ \
  if(MY_PE != 0) return 1; \
  type *test_buffer = (type *)buffers; \
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type)); \
  type value = 0;\
  for(index_t idx=0; idx < num_indexes; idx++){ \
    for(int jdx=0; jdx < REMOTE_PES; jdx++){\
      value += shmem_ ## type ## _g(&(test_buffer[idx]), jdx+1); \
    }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES; \
  return value;\
}

#define elemental_put_type2(type_a, type_b)                                    \
index_t test_shmem_ ## type_a ## type_b ## _p(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){ \
  if(MY_PE != 0) return 1; \
  type_a type_b *test_buffer = (type_a type_b *)buffers; \
  type_a type_b value = 0; \
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b)); \
  for(index_t idx=0; idx < num_indexes; idx++){ \
    for(int jdx=0; jdx < REMOTE_PES; jdx++){\
      shmem_ ## type_a ## type_b ## _p(&(test_buffer[idx]), value, jdx+1); \
    }\
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  shmem_quiet();\
  return 0;\
}

#define elemental_get_type2(type_a, type_b)                                    \
  index_t test_shmem_ ## type_a ## type_b ## _g(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){ \
  if(MY_PE != 0) return 1; \
  type_a type_b *test_buffer = (type_a type_b *)buffers; \
  type_a type_b value = 0; \
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b)); \
  for(index_t idx=0; idx < num_indexes; idx++){ \
    for(int jdx=0; jdx < REMOTE_PES; jdx++){\
      value += shmem_ ## type_a ## type_b ## _g(&(test_buffer[idx]), jdx+1); \
    }\
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  return value;\
}

declare_with_common_types(elemental_put_type)
declare_with_common_types(elemental_get_type)

#define block_put_type(type) \
index_t test_shmem_ ## type ## _put(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type *shmem_buffer = (type*)test_buffers->symetric;\
  type *local_buffer = (type*)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type ## _put(shmem_buffer, local_buffer, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES; \
  shmem_quiet();\
  return 0;\
}

#define block_put_type2(type_a, type_b)                                         \
  index_t test_shmem_ ## type_a ## type_b ## _put(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type_a type_b *shmem_buffer = (type_a type_b *)test_buffers->symetric;\
  type_a type_b *local_buffer = (type_a type_b *)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type_a ## type_b ## _put(shmem_buffer, local_buffer, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  shmem_quiet();\
  return 0;\
  }

#define block_get_type(type) \
index_t test_shmem_ ## type ## _get(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type *shmem_buffer = (type*)test_buffers->symetric;\
  type *local_buffer = (type*)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type ## _get(local_buffer, shmem_buffer, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES; \
  return 0;\
}

#define block_get_type2(type_a, type_b)                                         \
  index_t test_shmem_ ## type_a ## type_b ## _get(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type_a type_b *shmem_buffer = (type_a type_b *)test_buffers->symetric;\
  type_a type_b *local_buffer = (type_a type_b *)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type_a ## type_b ## _get(local_buffer, shmem_buffer, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  return 0;\
}

declare_with_common_types(block_put_type)
declare_with_common_types(block_get_type)

index_t test_shmem_put32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_put32(shmem_buffer, local_buffer, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int32_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_put64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int64_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int64_t *shmem_buffer = (int64_t*)test_buffers->symetric;
  int64_t *local_buffer = (int64_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_put64(shmem_buffer, local_buffer, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int64_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_put128(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int128_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int128_t *shmem_buffer = (int128_t*)test_buffers->symetric;
  int128_t *local_buffer = (int128_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_put128(shmem_buffer, local_buffer, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int128_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_get32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_get32(local_buffer, shmem_buffer, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int32_t) * REMOTE_PES;
  return 0;
}

index_t test_shmem_get64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int64_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int64_t *shmem_buffer = (int64_t*)test_buffers->symetric;
  int64_t *local_buffer = (int64_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_get64(local_buffer, shmem_buffer, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int64_t) * REMOTE_PES;
  return 0;
}

index_t test_shmem_get128(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int128_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int128_t *shmem_buffer = (int128_t*)test_buffers->symetric;
  int128_t *local_buffer = (int128_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_get128(local_buffer, shmem_buffer, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int128_t) * REMOTE_PES;
  return 0;
}

#define strided_put_type(type)\
  index_t test_shmem_ ## type ## _iput(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type *shmem_buffer = (type*)test_buffers->symetric;\
  type *local_buffer = (type*)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type ## _iput(shmem_buffer, local_buffer, 2, 2, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES; \
  shmem_quiet();\
  return 0;\
}

#define strided_get_type(type)\
  index_t test_shmem_ ## type ## _iget(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type *shmem_buffer = (type*)test_buffers->symetric;\
  type *local_buffer = (type*)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type ## _iget(local_buffer, shmem_buffer, 2, 2, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES; \
  return 0;\
}

#define strided_put_type2(type_a, type_b)                                         \
index_t test_shmem_ ## type_a ## type_b ## _iput(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type_a type_b *shmem_buffer = (type_a type_b *)test_buffers->symetric;\
  type_a type_b *local_buffer = (type_a type_b *)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type_a ## type_b ## _iput(shmem_buffer, local_buffer, 2, 2, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  shmem_quiet();\
  return 0;\
}

#define strided_get_type2(type_a, type_b)                                         \
index_t test_shmem_ ## type_a ## type_b ## _iget(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b));\
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;\
  type_a type_b *shmem_buffer = (type_a type_b *)test_buffers->symetric;\
  type_a type_b *local_buffer = (type_a type_b *)test_buffers->local;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    shmem_ ## type_a ## type_b ## _iget(local_buffer, shmem_buffer, 2, 2, num_indexes, idx+1); \
  }\
  *bytes_transfered = num_indexes * sizeof(type_a type_b) * REMOTE_PES; \
  return 0;\
}

declare_with_common_types(strided_put_type)
declare_with_common_types(strided_get_type)

index_t test_shmem_iput32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iput32(shmem_buffer, local_buffer, 2, 2, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int32_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_iput64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int64_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int64_t *shmem_buffer = (int64_t*)test_buffers->symetric;
  int64_t *local_buffer = (int64_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iput64(shmem_buffer, local_buffer, 2, 2, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int64_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_iput128(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int128_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int128_t *shmem_buffer = (int128_t*)test_buffers->symetric;
  int128_t *local_buffer = (int128_t*)test_buffers->local;

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iput128(shmem_buffer, local_buffer, 2, 2, num_indexes, idx+1);
  }

  *bytes_transfered = num_indexes * sizeof(int128_t) * REMOTE_PES;
  shmem_quiet();
  return 0;
}

index_t test_shmem_iget32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iget32(local_buffer, shmem_buffer, 2, 2, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int32_t) * REMOTE_PES;
  return 0;
}

index_t test_shmem_iget64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int64_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int64_t *shmem_buffer = (int64_t*)test_buffers->symetric;
  int64_t *local_buffer = (int64_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iget64(local_buffer, shmem_buffer, 2, 2, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int64_t) * REMOTE_PES;
  return 0;
}

index_t test_shmem_iget128(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(int128_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int128_t *shmem_buffer = (int128_t*)test_buffers->symetric;
  int128_t *local_buffer = (int128_t*)test_buffers->local;
  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_iget128(local_buffer, shmem_buffer, 2, 2, num_indexes, idx+1);
  }
  *bytes_transfered = num_indexes * sizeof(int128_t) * REMOTE_PES;
  return 0;
}

#define shmem_collective(type, op)                                         \
  index_t test_shmem_ ## type ## _ ## op ## _to_all(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  collective_buffers_t *local_buffers = (collective_buffers_t *)buffers;\
  type *target = (type *)local_buffers->target;                         \
  type *source = (type *)local_buffers->source;\
  type *pWrk = (type *)local_buffers->pWrk;\
  long *pSync = (long *)local_buffers->pSync;\
  shmem_ ## type ## _ ## op ## _to_all(target, source, num_indexes, 0, 0, N_PES, pWrk, pSync);\
  *bytes_transfered = num_indexes * sizeof(type);\
  return 0;\
  }

#define shmem_collective2(type_a, type_b, op)                                    \
  index_t test_shmem_ ## type_a ## type_b ## _ ## op ## _to_all(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type_a type_b));\
  collective_buffers_t *local_buffers = (collective_buffers_t *)buffers;\
  type_a type_b *target = (type_a type_b *)local_buffers->target;                         \
  type_a type_b *source = (type_a type_b *)local_buffers->source;\
  type_a type_b *pWrk = (type_a type_b *)local_buffers->pWrk;\
  long *pSync =(long *)local_buffers->pSync;\
  shmem_ ## type_a ## type_b ## _ ## op ## _to_all(target, source, num_indexes, 0, 0, N_PES, pWrk, pSync);\
  *bytes_transfered = num_indexes * sizeof(type_a type_b);\
  return 0;\
  }

#define collective_tests_types(collective_function, op)   \
  collective_function(short, op)                \
  collective_function(int, op)                      \
  collective_function(long, op)                             \
  collective_function ## 2(long, long, op)                      \

#define collective_tests_ops(collective_function)\
  collective_tests_types(collective_function, and)\
  collective_tests_types(collective_function, or)\
  collective_tests_types(collective_function, xor)\
  collective_tests_types(collective_function, min)\
  collective_tests_types(collective_function, max)\
  collective_tests_types(collective_function, sum)\
  collective_tests_types(collective_function, prod)

collective_tests_ops(shmem_collective)

#define cswap_type(type)\
  index_t test_shmem_ ## type ## _cswap(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  type new_value=0;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
  for(index_t jdx=0; jdx < num_indexes; jdx++){\
    new_value = shmem_ ## type ## _cswap(&(buffer[jdx]), (type)0, new_value, idx+1); \
  }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return 0;\
  }

cswap_type(int)
cswap_type(long)

index_t test_shmem_longlong_cswap(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  long long new_value=0;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      new_value = shmem_longlong_cswap(&(buffer[jdx]), (long long)0, new_value, idx+1); 
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return 0;
}

#define fadd_type(type)\
  index_t test_shmem_ ## type ## _fadd(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  type new_value=0;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
  for(index_t jdx=0; jdx < num_indexes; jdx++){\
  new_value = shmem_ ## type ## _fadd(&(buffer[jdx]), (type)0, idx+1); \
  }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return new_value;\
  }

fadd_type(int)
fadd_type(long)

index_t test_shmem_longlong_fadd(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  long long new_value=0;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      new_value = shmem_longlong_fadd(&(buffer[jdx]), (long long)0, idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return new_value;
}

#define finc_type(type)\
  index_t test_shmem_ ## type ## _finc(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  type new_value=0;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
  for(index_t jdx=0; jdx < num_indexes; jdx++){\
  new_value = shmem_ ## type ## _finc(&(buffer[jdx]), idx+1); \
  }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return new_value;\
  }

finc_type(int)
finc_type(long)

index_t test_shmem_longlong_finc(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  long long new_value=0;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      new_value = shmem_longlong_finc(&(buffer[jdx]), idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return new_value;
}

#define add_type(type)\
  index_t test_shmem_ ## type ## _add(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
  for(index_t jdx=0; jdx < num_indexes; jdx++){\
  shmem_ ## type ## _add(&(buffer[jdx]), (type)0, idx+1); \
  }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return 0;\
  }

add_type(int)
add_type(long)

index_t test_shmem_longlong_add(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      shmem_longlong_add(&(buffer[jdx]), (long long)0, idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return 0;
}

#define inc_type(type)\
  index_t test_shmem_ ## type ## _inc(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
  for(index_t jdx=0; jdx < num_indexes; jdx++){\
  shmem_ ## type ## _inc(&(buffer[jdx]), idx+1); \
  }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return 0;\
  }

inc_type(int)
inc_type(long)

index_t test_shmem_longlong_inc(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      shmem_longlong_inc(&(buffer[jdx]), idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return 0;
}

#define wait_type(type)\
  index_t test_shmem_ ## type ## _wait(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  type *buffer = (type *)buffers;\
  if(MY_PE == 1){                               \
    buffer[0] = 0;\
  }\
  shmem_barrier_all();\
  if(MY_PE == 1){\
  shmem_ ## type ## _wait(&(buffer[0]),(type)0);\
  }\
  if(MY_PE == 0){\
    shmem_ ## type ## _p(&(buffer[0]), (type)1,1); \
  }\
  *bytes_transfered = 0;\
  return 0;\
  }

wait_type(short)
wait_type(int)
wait_type(long)

index_t test_shmem_wait(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  return test_shmem_long_wait(test_start, test_size, buffers, bytes_transfered);
}

index_t test_shmem_longlong_wait(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  long long *buffer = (long long *)buffers;
  if(MY_PE == 1){
    buffer[0] =0;
  }
  shmem_barrier_all();
  if(MY_PE == 1){
    shmem_longlong_wait(&(buffer[0]),(long long)0);
  }
  if(MY_PE == 0){
    shmem_longlong_p(&(buffer[0]), (long long)1,1);
  }
  *bytes_transfered = 0;
  return 0;
}

/* OpenSHMEM depreciated _SHMEM* since 1.3. To ensure compatibility we check if SHMEM_CMP_NE is defined. If not
 * We define it as equal to _SHMEM_CMP_NE */
#ifndef SHMEM_CMP_NE
#define SHMEM_CMP_NE _SHMEM_CMP_NE
#endif

#define wait_until_type(type)\
  index_t test_shmem_ ## type ## _wait_until(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  type *buffer = (type *)buffers;\
  if(MY_PE == 1){                               \
  buffer[0] = 0;\
  }\
  shmem_barrier_all();\
  if(MY_PE == 1){\
    shmem_ ## type ## _wait_until(&(buffer[0]),SHMEM_CMP_NE,(type)0);  \
  }\
  if(MY_PE == 0){\
  shmem_ ## type ## _p(&(buffer[0]), (type)1,1); \
  }\
  *bytes_transfered = 0;\
  return 0;\
  }

wait_until_type(short)
wait_until_type(int)
wait_until_type(long)

index_t test_shmem_wait_until(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  return test_shmem_long_wait_until(test_start, test_size, buffers, bytes_transfered);
}

index_t test_shmem_longlong_wait_until(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  long long *buffer = (long long *)buffers;
  if(MY_PE == 1){
    buffer[0] = 0;
  }
  shmem_barrier_all();
  if(MY_PE == 1){
    shmem_longlong_wait_until(&(buffer[0]),SHMEM_CMP_NE,(long long)0);
  }
  if(MY_PE == 0){
    shmem_longlong_p(&(buffer[0]), (long long)1,1);
  }
  *bytes_transfered = 0;
  return 0;
}

#define swap_type(type)\
index_t test_shmem_ ## type ## _swap(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){\
  if(MY_PE != 0) return 1;\
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(type));\
  type *buffer = (type *)buffers;\
  type new_value=0;\
  for(int idx=0; idx < REMOTE_PES; idx++){\
    for(index_t jdx=0; jdx < num_indexes; jdx++){\
      new_value = shmem_ ## type ## _swap(&(buffer[jdx]), new_value, idx+1);\
    }\
  }\
  *bytes_transfered = num_indexes * sizeof(type) * REMOTE_PES;\
  return 0;\
}

swap_type(int)
swap_type(long)
swap_type(float)
swap_type(double)

index_t test_shmem_swap(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long));
  long *buffer = (long *)buffers;
  long new_value=0;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      new_value = shmem_swap(&(buffer[jdx]), new_value, idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long) * REMOTE_PES;
  return 0;
}

index_t test_shmem_longlong_swap(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0) return 1;
  index_t num_indexes = compute_indexes_per_pe(test_size, sizeof(long long));
  long long *buffer = (long long *)buffers;
  long long new_value=0;
  for(int idx=0; idx < REMOTE_PES; idx++){
    for(index_t jdx=0; jdx < num_indexes; jdx++){
      new_value = shmem_longlong_swap(&(buffer[jdx]), new_value, idx+1);
    }
  }
  *bytes_transfered = num_indexes * sizeof(long long) * REMOTE_PES;
  return 0;
}

/* Reads from local symetric heap without going through shmem  */

index_t test_shmem_local_read(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  int32_t *shmem_buffer = (int32_t*)buffers;
  index_t ret_val = 0;
  for(int idx=0; idx < indexes_per_pe; idx++){
    ret_val += shmem_buffer[idx];
  }
  *bytes_transfered = indexes_per_pe * sizeof(int32_t);
  return ret_val;
}

/* Writes to local symetric heap without going through shmem */

index_t test_shmem_local_write(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  int32_t *shmem_buffer = (int32_t*)buffers;
  for(int idx=0; idx < indexes_per_pe; idx++){
    shmem_buffer[idx]=5;
  }

  *bytes_transfered = indexes_per_pe * sizeof(int32_t);
  return 1;
}

/* Reads and writes to local symetric heap without going through shmem */

index_t test_shmem_local_readwrite(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  int32_t *shmem_buffer = (int32_t*)buffers;
  for(int idx=0; idx < indexes_per_pe; idx++){
    shmem_buffer[idx]+=5;
  }

  *bytes_transfered = indexes_per_pe * sizeof(int32_t);
  return 1;
}

/* Node 0 reads a total of size bytes from local and remote nodes */

index_t test_shmem_32bit_get(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  index_t total_indexes = indexes_per_pe * REMOTE_PES;
  int32_t *shmem_buffer = (int32_t*)buffers;
  int32_t ret_val = 0, mem_buffer;
  int remote_pe;
  index_t remote_index;
  for(int idx=0; idx < total_indexes; idx++){
    remote_pe = (idx / indexes_per_pe)+1;
    remote_index = (idx % indexes_per_pe);
    shmem_get32(&mem_buffer, &(shmem_buffer[remote_index]), 1, remote_pe);
    ret_val += mem_buffer;
  }

  *bytes_transfered = total_indexes * sizeof(int32_t);
  return ret_val;
}

index_t test_shmem_32bit_put(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  index_t total_indexes = indexes_per_pe * REMOTE_PES;
  int32_t *shmem_buffer = (int32_t*)buffers;
  int32_t mem_buffer=5;
  int remote_pe;
  index_t remote_index;
  for(int idx=0; idx < total_indexes; idx++){
    remote_pe = (idx / indexes_per_pe)+1;
    remote_index = (idx % indexes_per_pe);
    shmem_put32(&(shmem_buffer[remote_index]), &mem_buffer, 1, remote_pe);
  }
  *bytes_transfered = total_indexes * sizeof(int32_t);
  shmem_quiet();
  return 5;
}

index_t test_shmem_32bit_getput(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  index_t total_indexes = indexes_per_pe * REMOTE_PES;
  int32_t *shmem_buffer = (int32_t*)buffers;
  int32_t ret_val = 0, mem_buffer=5;
  int remote_pe;
  index_t remote_index;
  for(int idx=0; idx < total_indexes; idx++){
    remote_pe = (idx / indexes_per_pe)+1;
    remote_index = idx % indexes_per_pe;
    shmem_get32(&mem_buffer, &(shmem_buffer[remote_index]), 1, remote_pe);
    ret_val += mem_buffer;
    shmem_put32(&(shmem_buffer[remote_index]), &ret_val, 1, remote_pe);
  }
  *bytes_transfered = total_indexes * sizeof(int32_t) * 2;
  shmem_quiet();
  return ret_val;
}

index_t test_shmem_bulk_put(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0){
    return 0;
  }
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;
  int n_pes = REMOTE_PES;

  for(int idx=0; idx < n_pes; idx++){
    shmem_put32(shmem_buffer, local_buffer, (int64_t)indexes_per_pe, idx+1);
  }

  *bytes_transfered = indexes_per_pe * n_pes * sizeof(int32_t);
  shmem_quiet();
  return 5;
}

index_t test_shmem_bulk_get(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0){
    return 0;
  }
  size_t indexes_per_pe = (size_t)compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;
  int n_pes = REMOTE_PES;

  for(int idx=0; idx < n_pes; idx++){
    shmem_get32(local_buffer, shmem_buffer, indexes_per_pe, idx+1);
  }

  *bytes_transfered = indexes_per_pe * n_pes * sizeof(int32_t);
  return 5;
}

index_t test_shmem_bulk_getput(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0){
    return 0;
  }
  index_t indexes_per_pe = compute_indexes_per_pe(test_size, sizeof(int32_t));
  sym_and_local_t *test_buffers = (sym_and_local_t *)buffers;
  int32_t *shmem_buffer = (int32_t*)test_buffers->symetric;
  int32_t *local_buffer = (int32_t*)test_buffers->local;
  int n_pes = REMOTE_PES;

  for(int idx=0; idx < n_pes; idx++){
    shmem_get32(local_buffer, shmem_buffer, indexes_per_pe, idx+1);
    shmem_put32(shmem_buffer, local_buffer, indexes_per_pe, idx+1);
  }

  *bytes_transfered = indexes_per_pe * n_pes * 2 * sizeof(int32_t);
  shmem_quiet();

  return 5;
}

index_t test_shmem_barrier_all(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  shmem_barrier_all();
  return 2;
}

index_t test_shmem_broadcast32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;

  shmem_broadcast32(target, source, test_size/sizeof(int32_t), 0, 0, 0, N_PES, pSync);

  *bytes_transfered = test_size;
  
  return 0;
}

index_t test_shmem_broadcast64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;

  shmem_broadcast64(target, source, test_size/sizeof(int64_t), 0, 0, 0, N_PES, pSync);

  *bytes_transfered = test_size;

  return 0;
}

index_t test_shmem_collect32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;
  index_t num_elements = compute_indexes_per_pe(test_size, N_PES);

  shmem_collect32(target, source, num_elements, 0, 0, N_PES, pSync);

  *bytes_transfered = num_elements*sizeof(int32_t);

  return 0;
}

index_t test_shmem_collect64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;
  index_t num_elements = compute_indexes_per_pe(test_size, N_PES);

  shmem_collect64(target, source, num_elements, 0, 0, N_PES, pSync);

  *bytes_transfered = num_elements*sizeof(int64_t);

  return 0;
}

index_t test_shmem_fcollect32(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;
  index_t num_elements = compute_indexes_per_pe(test_size, N_PES);

  shmem_fcollect32(target, source, num_elements, 0, 0, N_PES, pSync);

  *bytes_transfered = num_elements*sizeof(int32_t);

  return 0;
}

index_t test_shmem_fcollect64(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  broadcast_buffers_t *broadcast_buffers = (broadcast_buffers_t *)buffers;
  int32_t *target = broadcast_buffers->target;
  int32_t *source = broadcast_buffers->source;
  long *pSync = broadcast_buffers->pSync;
  index_t num_elements = compute_indexes_per_pe(test_size, N_PES);

  shmem_fcollect64(target, source, num_elements, 0, 0, N_PES, pSync);

  *bytes_transfered = num_elements*sizeof(int64_t);

  return 0;
}

index_t test_shmem_getmem(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0){
    return 1;
  }

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_getmem(buffers, buffers, test_size, idx+1);
  }

  *bytes_transfered = test_size * REMOTE_PES;
  return 0;
}

index_t test_shmem_putmem(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(MY_PE != 0){
    return 1;
  }

  for(int idx=0; idx < REMOTE_PES; idx++){
    shmem_putmem(buffers, buffers, test_size, idx+1);
  }

  *bytes_transfered = test_size * REMOTE_PES;
  shmem_quiet();
  return 0;
}

void init_shmalloc(void **buffer, index_t size){
  buffer_ptr_t *new_buffer = malloc(sizeof(buffer_ptr_t));
  *buffer = (void *)new_buffer;
}

index_t test_shmalloc(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  buffer_ptr_t *new_buffer = (buffer_ptr_t *)buffers;
  new_buffer->buffer = shmem_malloc(test_size);
#ifdef DEBUG_ALLOCATIONS
  shmalloc_count++;
#endif
  *bytes_transfered=test_size;
  return 0;
}

void cleanup_per_iteration_shmalloc(void *doomed){
  buffer_ptr_t *new_buffer = (buffer_ptr_t *)doomed;
  free_distributed_buffer(new_buffer->buffer);
}

void cleanup_shmalloc(void *doomed){
  buffer_ptr_t *new_buffer = (buffer_ptr_t *)doomed;
  free(new_buffer);
}

index_t test_shfree(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  shmem_free(buffers);
  *bytes_transfered=test_size;
#ifdef DEBUG_ALLOCATIONS
  shfree_count++;
#endif
  return 0;
}

index_t test_shmem_clear_lock(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }

  long *shmem_buffer = (long*)buffers;

  shmem_clear_lock(shmem_buffer);

  *bytes_transfered = 0;
  return 0;
}

index_t test_shmem_set_lock(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }

  long *shmem_buffer = (long*)buffers;

  shmem_set_lock(shmem_buffer);

  *bytes_transfered = 0;
  return 0;
}

void init_per_iteration_shmem_clear_lock(void **buffer, index_t size){
  index_t bytes;
  long *locked = (long *)*buffer;
  test_shmem_set_lock(0,0,locked,&bytes);
}

void cleanup_per_iteration_shmem_set_lock(void *locked){
  index_t bytes;
  test_shmem_clear_lock(0,0,locked,&bytes);
}

index_t test_shmem_test_lock(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  if(shmem_my_pe() != 0){
    return 0;
  }

  long *shmem_buffer = (long*)buffers;

  int lock = shmem_test_lock(shmem_buffer);

  *bytes_transfered = 0;
  return lock;
}

#ifdef USE_SHMEM12

index_t test_shmem_init(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  shmem_init();
  return 0;
}

void init_per_iteration_shmem_init(void **buffer, index_t size){
  shmem_finalize();
}

index_t test_shmem_finalize(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered){
  shmem_finalize();
  return 0;
}

void cleanup_per_iteration_shmem_finalize(void *buffer){
  shmem_init();
}

#endif
