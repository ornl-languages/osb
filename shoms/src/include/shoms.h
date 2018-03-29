/*
   This file is part of SHOMS.

   Copyright (C) 2014-2105, UT-Battelle, LLC.

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

#include <config.h>

#ifdef SGI_HEADERS
#include <mpp/shmem.h>
#else
#include <shmem.h>
#endif

#include <stdint.h>
#include <orbtimer.h>
#include <limits.h>
#include <macros.h>
#include <shoms-types.h>

extern int global_npes;
extern int global_my_pe;

#define REMOTE_PES (global_npes-1)
#define N_PES (global_npes)
#define MY_PE (global_my_pe)

#define NUM_TESTS ((index_t)128)
#define MAX_TESTS ((index_t)128)

void init_tests();
index_t all_tests(test_t **tests_array);
index_t affinity_tests(test_t **tests_array);
void free_test_t(test_t *doomed, index_t size);

index_t init_iterations(iteration_data_t **iterations_array, index_t data_min, index_t data_max);
void free_iterations(iteration_data_t doomed, index_t size);

buffer_from_file_t *read_file(char *file_name);
void free_file_buffer(buffer_from_file_t *doomed);
index_t process_string_to_number(char *input);

index_t decide_iterations(index_t current_size);

void init_distributed_32bit_bufffer(void **buffers,index_t size);
void init_sym_and_local_32bit(void **buffers, index_t size);
void init_sym_and_local_64bit(void **buffers, index_t size);
void init_sym_and_local_128bit(void **buffers, index_t size);
void init_strided_sym_and_local_32bit(void **buffers, index_t size);
void init_strided_sym_and_local_64bit(void **buffers, index_t size);
void init_strided_sym_and_local_128bit(void **buffers, index_t size);
void init_collective_short(void **buffers,index_t size);
void init_collective_int(void **buffers,index_t size);
void init_collective_long(void **buffers,index_t size);
void init_collective_longlong(void **buffers,index_t size);
void free_collective(void *doomed);
void init_shmem_broadcast(void **buffers,index_t size);
void free_shmem_broadcast(void *doomed);
void free_sym_and_local_t(void *doomed);
void cleanup_distributed_buffer(void *buffer);
void calculate_local_performance(test_results_t *test_results);
void calculate_global_performance(test_results_t *test_results);
void calculate_global_no_bw_performance(test_results_t *test_results);

declare_with_common_types(type_buffer_alloc_prototype)
declare_with_common_types(init_sym_and_local_prototype)
declare_with_common_types(init_strided_sym_and_local_prototype)
declare_with_common_types(type_buffer_alloc_zero_prototype)

common_type_tests(p);
common_type_tests(put);
common_type_tests(iput);

common_type_tests(g);
common_type_tests(get);
common_type_tests(iget);

common_type_tests(and_to_all);
common_type_tests(or_to_all);
common_type_tests(xor_to_all);
common_type_tests(min_to_all);
common_type_tests(max_to_all);
common_type_tests(sum_to_all);
common_type_tests(prod_to_all);

declare_test(shmem_int_swap);
declare_test(shmem_long_swap);
declare_test(shmem_swap);
declare_test(shmem_longlong_swap);
declare_test(shmem_float_swap);
declare_test(shmem_double_swap);

declare_test(shmem_int_cswap);
declare_test(shmem_long_cswap);
declare_test(shmem_longlong_cswap);

declare_test(shmem_int_fadd);
declare_test(shmem_long_fadd);
declare_test(shmem_longlong_fadd);

declare_test(shmem_int_finc);
declare_test(shmem_long_finc);
declare_test(shmem_longlong_finc);

declare_test(shmem_int_add);
declare_test(shmem_long_add);
declare_test(shmem_longlong_add);

declare_test(shmem_int_inc);
declare_test(shmem_long_inc);
declare_test(shmem_longlong_inc);

declare_test(shmem_wait);
declare_test(shmem_short_wait);
declare_test(shmem_int_wait);
declare_test(shmem_long_wait);
declare_test(shmem_longlong_wait);

declare_test(shmem_wait_until);
declare_test(shmem_short_wait_until);
declare_test(shmem_int_wait_until);
declare_test(shmem_long_wait_until);
declare_test(shmem_longlong_wait_until);

declare_test(shmem_local_read);
declare_test(shmem_local_write);
declare_test(shmem_local_readwrite);
declare_test(shmem_barrier_all);
declare_test(shmem_broadcast32);
declare_test(shmem_broadcast64);
declare_test(shmem_collect32);
declare_test(shmem_collect64);
declare_test(shmem_fcollect32);
declare_test(shmem_fcollect64);
declare_test(shmem_getmem);
declare_test(shmem_putmem);
declare_test(shmalloc);
declare_test(shfree);

declare_test(shmem_set_lock);
declare_test(shmem_clear_lock);
declare_test(shmem_test_lock);

#ifdef USE_SHMEM12
declare_test(shmem_init);
declare_test(shmem_finalize);


#endif
