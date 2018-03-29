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

#define declare_with_common_types(function_name) \
  function_name(short) \
  function_name(int) \
  function_name(long) \
  function_name(float) \
  function_name(double) \
  function_name ## 2(long,long) \
  function_name ## 2(long,double)

#define declare_test(test_name) \
  void init_ ## test_name(void **buffers, index_t size); \
  index_t test_ ## test_name(index_t test_start, index_t test_size, void *buffers, index_t *bytes_transfered); \
  void cleanup_ ## test_name(void *buffers); \
  void collect_results_ ## test_name(test_results_t *test_results); \
  void init_per_iteration_ ## test_name(void **buffers, index_t size); \
  void cleanup_per_iteration_ ## test_name(void *buffers)

#define type_buffer_alloc_prototype(type)\
  void init_distributed_ ## type ## _buffer(void **buffers, index_t size);

#define type_buffer_alloc_prototype2(type_a, type_b)\
  void init_distributed_ ## type_a ## type_b ## _buffer(void **buffers, index_t size);

#define type_buffer_alloc_prototype_bits(size)\
  void init_distributed_buffer ## size(void **buffers, index_t size);

#define type_buffer_alloc_zero_prototype(type)\
  void init_distributed_ ## type ## _buffer_zeroed(void **buffers, index_t size);

#define type_buffer_alloc_zero_prototype2(type_a, type_b)\
  void init_distributed_ ## type_a ## type_b ## _buffer_zeroed(void **buffers, index_t size);


#define init_sym_and_local_prototype(type)\
  void init_sym_and_local_ ## type(void **buffers, index_t size);

#define init_sym_and_local_prototype2(type_a, type_b)\
  void init_sym_and_local_ ## type_a ## type_b(void **buffers, index_t size);

#define init_strided_sym_and_local_prototype(type)\
  void init_strided_sym_and_local_ ## type(void **buffers, index_t size);

#define init_strided_sym_and_local_prototype2(type_a, type_b)\
  void init_strided_sym_and_local_ ## type_a ## type_b(void **buffers, index_t size);

#define common_type_tests(test_name) \
  declare_test(shmem_short_ ## test_name);\
  declare_test(shmem_int_ ## test_name);\
  declare_test(shmem_long_ ## test_name);\
  declare_test(shmem_longlong_ ## test_name);\
  declare_test(shmem_float_ ## test_name);\
  declare_test(shmem_double_ ## test_name);\
  declare_test(shmem_longdouble_ ## test_name);\
  declare_test(shmem_ ## test_name ## 32);\
  declare_test(shmem_ ## test_name ## 64);\
  declare_test(shmem_ ## test_name ## 128)
