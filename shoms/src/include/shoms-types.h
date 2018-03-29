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

#include <stdio.h>

#define USE_UINT64_INDEX
#if defined(USE_INT32_INDEX)
typedef int32_t index_t;
#define INDEX_MIN INT32_MIN
#define INDEX_MAX INT32_MAX
#elif defined(USE_INT64_INDEX)
typedef uint32_t index_t;
#define INDEX_MIN 0
#define INDEX_MAX UINT32_MAX
#elif defined(USE_UINT64_INDEX)
typedef uint64_t index_t;
#define INDEX_MIN 0
#define INDEX_MAX UINT64_MAX
#elif defined(USE_INT64_INDEX)
typedef int64_t index_t;
#define INDEX_MIN INT64_MIN
#define INDEX_MAX INT64_MAX
#endif

typedef struct {
  ORB_tick_t accumulated_time_tick;
  ORB_tick_t min_time_tick;
  ORB_tick_t max_time_tick;
  double avg_time_tick;
  double bandwidth_bytes_per_tick;
  index_t transfered_data_bytes;
  index_t message_size;
  index_t iterations;
} test_results_t;

typedef struct {
  int32_t *symetric;
  int32_t *local;
} sym_and_local_t;

typedef struct {
  index_t(*test_function)(index_t, index_t, void*, index_t *);
  void(*init_function)(void **, index_t);
  void(*cleanup_function)(void *);
  void(*per_iteration_init_function)(void **, index_t);
  void(*per_iteration_cleanup_function)(void *);
  void(*collect_results)(test_results_t *);
  char *name;
  test_results_t *test_results;
} test_t;

typedef struct {
  index_t data_size;
  index_t iterations_count;
} iteration_data_t;

typedef struct {
  void *target;
  void *source;
  long *pSync;
} broadcast_buffers_t;

typedef struct {
  void *target;
  void *source;
  void *pWrk;
  long *pSync;
} collective_buffers_t;

typedef struct {
  char *buffer;
  size_t size;
  index_t line_count;
} buffer_from_file_t;

typedef struct {
  index_t os_id;
  index_t core_id;
  index_t socket_id;
  index_t cache_size;
} core_t;

typedef struct {
  core_t **cores;
  index_t core_count;
  index_t socket_id;
  index_t node_id;
  index_t cache_size;
} socket_t;

typedef struct {
  socket_t *socket;
  index_t socket_count;
  index_t node_id;
  index_t cache_size;
} node_t;

typedef struct {
  node_t *nodes;
  index_t node_count;
  index_t comm_id;
  index_t cache_size;
} comm_t;

typedef struct {
  int64_t A;
  int64_t B;
} int128_t;

typedef struct {
  void *buffer;
} buffer_ptr_t;

typedef struct parsed_options {
  int32_t affinity_test;
  int32_t disable_cache;
  int32_t warmup_run;
  int32_t reduce_op;
  index_t message_length;
  index_t minimum_size;
  index_t maximum_size;
  index_t run_time;
  char *output_file_path;
  FILE *output_file;
} parsed_options_t;
