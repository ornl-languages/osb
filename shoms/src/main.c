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

#define _ISOC99_SOURCE
#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <mpp/shmem.h>
#include <shoms.h>
#include <config.h>
#include <parameters.h>
#include <orbtimer.h>
#include <math.h>
#include <sched.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <glob.h>

//#define DEBUG
//#define DEBUG_ARGS

#ifdef DEBUG
#define DEBUG_ALLOCATIONS
#endif

int global_npes;
int global_my_pe;

void print_mem_management_stats();

void print_header(test_t *current_test, parsed_options_t *input_options){
  if(shmem_my_pe() == 0){
    fprintf(input_options->output_file, "\n#---------------------------------------------------\n"
                                          "# Benchmarking %s \n# #processes = %d\n"
                                          "#---------------------------------------------------\n"
                                          "       #bytes  #repetitions     t_min[nsec]     t_max[nsec]       t_avg[nsec]      Bw_aggregated[MB/sec]\n",
                                          current_test->name, global_npes );
  }
}

char *NA = "N/A";

void print_performance_data(test_t *current_test, index_t test_data_size, index_t iterations_count, parsed_options_t *input_options){
  if(global_my_pe == 0){
    char bw_buffer[1024];
    char message_size_buffer[1024];
    unsigned long min_time_ns = (unsigned long)round(((double)current_test->test_results->min_time_tick * ORB_REFFREQ) / (double)1000000000);
    unsigned long max_time_ns = (unsigned long)round(((double)current_test->test_results->max_time_tick * ORB_REFFREQ) / (double)1000000000);
    double average_time = (current_test->test_results->avg_time_tick * ORB_REFFREQ) / (double)1000000000;
    double bandwidth = current_test->test_results->bandwidth_bytes_per_tick * ORB_REFFREQ / (double)(1024 * 1024);

    if(current_test->test_results->transfered_data_bytes == 0){
      snprintf(message_size_buffer, 1024, "%13s", NA);
    } else {
      snprintf(message_size_buffer, 1024, "%13lu", (unsigned long)test_data_size);
    }

    if(bandwidth == (double)0){
      snprintf(bw_buffer, 1024, "%12s", NA);
    } else {
      snprintf(bw_buffer, 1024, "%12.2f", bandwidth);
    }

    fprintf(input_options->output_file, "%s %13lu   %13lu   %13lu    %15.2f              %s\n", 
            message_size_buffer, (unsigned long)iterations_count, min_time_ns, max_time_ns, average_time, bw_buffer);
    fflush(input_options->output_file);
  }
}

static void set_max_run_time(struct timeval *timer, parsed_options_t *input_options){
  if(input_options->run_time == 0) return;
  gettimeofday(timer, NULL);
  timer->tv_sec += input_options->run_time;
}

int pWork_time[_SHMEM_REDUCE_MIN_WRKDATA_SIZE];
long pSync_time[_SHMEM_REDUCE_SYNC_SIZE];
int run_time_good;

static int check_run_time(struct timeval *end_timer, parsed_options_t *input_options){
  struct timeval current;
  if(input_options->run_time == 0) return 0;
  gettimeofday(&current, NULL);
  if(end_timer->tv_sec < current.tv_sec){
    run_time_good = 1;
  } else {
    run_time_good = 0;
  }

  shmem_int_max_to_all(&run_time_good, &run_time_good, 1, 0, 0, global_npes, pWork_time, pSync_time);
  return run_time_good;
}

void run_tests(test_t *test_list, index_t test_length, iteration_data_t *iterations, index_t iterations_length, parsed_options_t *input_options){
  void *test_buffers;
  index_t test_data_size, iterations_count, transfered_count, nbuckets, test_data_alloc_size, test_data_scale_size;
  ORB_t timer_start, timer_stop;
  ORB_tick_t current_time;
  struct timeval end_time;

  for(int idx=0; idx < _SHMEM_REDUCE_SYNC_SIZE; idx++){
    pSync_time[idx]=_SHMEM_SYNC_VALUE;
  }

  for(index_t test_index=0; test_index < test_length; test_index++){
    if(test_list[test_index].test_function == NULL){
      continue;
    }
    print_header(&(test_list[test_index]), input_options);
    test_list[test_index].test_results = malloc(sizeof(test_results_t));
    for(index_t iterations_index=0; iterations_index < iterations_length; iterations_index++)
    {
      set_max_run_time(&end_time, input_options);
      test_data_size = iterations[iterations_index].data_size;
      iterations_count = iterations[iterations_index].iterations_count;

      if(input_options->disable_cache == 1){
        nbuckets = iterations_count;
        test_data_scale_size = test_data_size;
      } else {
        nbuckets = 1;
        test_data_scale_size = 0;
      }
      test_data_alloc_size = test_data_size * nbuckets;
      if(test_list[test_index].init_function != NULL){
        test_list[test_index].init_function(&test_buffers, test_data_alloc_size);
      }
      if(input_options->warmup_run){
        int warmup_runs = iterations_count / 10;
        for(int idx=0; idx < warmup_runs; idx++) {
          if(check_run_time(&end_time, input_options) == 1){
            if(MY_PE == 0) fprintf(input_options->output_file, "Max iteration time exceeded, skipping\n");
            goto cleanup_test;
          }
          transfered_count = 0;
          if(test_list[test_index].per_iteration_init_function != NULL){
            test_list[test_index].per_iteration_init_function(&test_buffers, test_data_alloc_size);
          }
          shmem_barrier_all();

          test_list[test_index].test_function(idx * test_data_scale_size, test_data_size, test_buffers, &transfered_count);

          if(test_list[test_index].per_iteration_cleanup_function != NULL){
            test_list[test_index].per_iteration_cleanup_function(test_buffers);
          }
        }
      }

      test_list[test_index].test_results->accumulated_time_tick = 0;
      test_list[test_index].test_results->min_time_tick = INDEX_MAX;
      test_list[test_index].test_results->max_time_tick = INDEX_MIN;
      test_list[test_index].test_results->transfered_data_bytes = 0;
      test_list[test_index].test_results->iterations = iterations_count;
      transfered_count = 0;

      for(int idx=0; idx < iterations_count; idx++){
        if(check_run_time(&end_time, input_options) == 1){
          if(MY_PE == 0) fprintf(input_options->output_file, "Max iteration time exceeded, skipping\n");
          goto cleanup_test;
        }
        if(test_list[test_index].per_iteration_init_function != NULL){
          test_list[test_index].per_iteration_init_function(&test_buffers, test_data_alloc_size);
        }
        shmem_barrier_all();

        ORB_read(timer_start);
        test_list[test_index].test_function(idx * test_data_scale_size, test_data_size, test_buffers, &transfered_count);
        ORB_read(timer_stop);

        current_time = ORB_cycles_a(timer_stop, timer_start);

        test_list[test_index].test_results->accumulated_time_tick += current_time;
        if(test_list[test_index].test_results->min_time_tick > current_time) {
          test_list[test_index].test_results->min_time_tick = current_time;
        }
        if(test_list[test_index].test_results->max_time_tick < current_time) {
          test_list[test_index].test_results->max_time_tick = current_time;
        }
        test_list[test_index].test_results->transfered_data_bytes += (transfered_count);
        transfered_count = 0;
        if(test_list[test_index].per_iteration_cleanup_function != NULL){
          test_list[test_index].per_iteration_cleanup_function(test_buffers);
        }
      }

      test_list[test_index].collect_results(test_list[test_index].test_results);
      print_performance_data(&(test_list[test_index]), test_data_size, iterations_count, input_options);
cleanup_test:
      if(test_list[test_index].cleanup_function != NULL){
        shmem_barrier_all();
        test_list[test_index].cleanup_function(test_buffers);
      }
      if(input_options->output_file_path != NULL){
        fflush(input_options->output_file);
      }
    }
#ifdef DEBUG_ALLOCATIONS
    print_mem_management_stats(input_options);
#endif
  }
}

#define USING_LINUX

void bind_to_core(core_t *new_core){
#ifdef USING_LINUX
  index_t cpu = new_core->os_id;
  cpu_set_t first_cpu;
  CPU_ZERO(&first_cpu);
  CPU_SET(cpu, &first_cpu);
  int ret_val = sched_setaffinity(0,sizeof(cpu_set_t),&first_cpu);
  assert(ret_val == 0);
#endif
}

void run_affinity_tests(test_t *test_list, index_t test_length, iteration_data_t *iterations, index_t iterations_length, parsed_options_t *input_options, comm_t *shmem_comm){
  node_t *pe0_node;
  node_t *pe1_node;
  socket_t *pe0_socket;
  socket_t *pe1_socket;

  if(MY_PE == 0){
    pe0_node = shmem_comm->nodes;
    pe0_socket = NULL;
    pe1_node = malloc(sizeof(node_t));
    pe1_socket = malloc(sizeof(socket_t));
    shmem_getmem(pe1_node, shmem_comm->nodes, sizeof(node_t), 1);
  } else {
    pe0_node = malloc(sizeof(node_t));
    pe0_socket = malloc(sizeof(socket_t));
    pe1_node = shmem_comm->nodes;
    pe1_socket = NULL;
    shmem_getmem(pe0_node, shmem_comm->nodes, sizeof(node_t), 0);
  }

  for(int idx=0; idx < pe0_node->socket_count; idx++){
    if(MY_PE==1){
      shmem_getmem(pe0_socket, &(pe1_node->socket[idx]), sizeof(socket_t), 0);
    } else {
      pe0_socket = &(pe0_node->socket[idx]);
    }
    for(int mdx=0; mdx < pe0_socket->core_count; mdx++){
      if(MY_PE==0){
        bind_to_core(pe0_socket->cores[mdx]);
      }
      for(int jdx=0; jdx < pe1_node->socket_count; jdx++){
        if(MY_PE==0){
          shmem_getmem(pe1_socket, &(pe0_node->socket[idx]), sizeof(socket_t), 1);
        } else {
          pe1_socket = &(pe1_node->socket[idx]);
        }
        for(int ndx=0; ndx < pe1_socket->core_count; ndx++){
          if(MY_PE==1){
            bind_to_core(pe1_socket->cores[ndx]);
          } else {
            fprintf(input_options->output_file, "Running tests. PE0 socket %i core %i. PE1 socket %i core %i\n", idx, mdx, jdx, ndx);
          }
          run_tests(test_list, test_length, iterations, iterations_length, input_options);
        }
      }
    }
  }
}

index_t count_cpus_from_range_string(char *range){
  index_t string_size = strlen(range);
  index_t buffer_index=0, range_index=0;
  index_t extracted_values[2];
  char extraction_buffer[string_size];

  for(int idx=0; idx < 2; idx++){
    buffer_index = 0;
    while(buffer_index < string_size && range[range_index] != '-' && range[range_index] != '\n'){
      extraction_buffer[buffer_index] = range[range_index];
      buffer_index++;
      range_index++;
    }
    extraction_buffer[buffer_index] = '\0';
    range_index++;
    extracted_values[idx] = process_string_to_number(extraction_buffer);
  }

  return (extracted_values[1] - extracted_values[0]) + 1;
}

index_t count_cpus_from_sys_string(char *sys_data){
  int comma_count = 0, extraction_index, sys_index;
  index_t cpu_count=0, string_size;
  string_size = strlen(sys_data);
  char extraction_buffer[string_size];

  for(int idx=0; idx < string_size; idx++){
    if(sys_data[idx] == ','){
      comma_count++;
    }
  }

  sys_index=0;

  for(int idx=0; idx <= comma_count; idx++){
    extraction_index=0;
    while(sys_index < string_size && sys_data[sys_index] != ','  && sys_data[sys_index] != '\n'){
      extraction_buffer[extraction_index] = sys_data[sys_index];
      extraction_index++;
      sys_index++;
    }
    extraction_buffer[extraction_index] = '\0';
    cpu_count += count_cpus_from_range_string(extraction_buffer);
    sys_index++;
  }

  return cpu_count;
}

index_t get_cores_count(){
  index_t cores_count=0;
  buffer_from_file_t *online_cpus = read_file("/sys/devices/system/cpu/online");
  cores_count = count_cpus_from_sys_string(online_cpus->buffer);
  free_file_buffer(online_cpus);
  return cores_count;
}

void remove_last_newline(buffer_from_file_t *in_string){
  index_t buffer_index = in_string->size-1;
  while(in_string->buffer[buffer_index] != '\n') buffer_index -= 1;
  in_string->buffer[buffer_index] = '\0';
}

int sort_cores_cmp(const void *a, const void *b){
  core_t *core_a, *core_b;
  core_a = (core_t *)a;
  core_b = (core_t *)b;

  return core_a->os_id - core_b->os_id;
}

void process_cores(core_t *core, index_t cores_count){
  glob_t glob_results;
  buffer_from_file_t *cpu_data;
  char path_buffer[1024];
  int return_code = glob("/sys/devices/system/cpu/cpu[0-9]*", GLOB_ONLYDIR, NULL, &glob_results);

  for(int idx=0; idx < glob_results.gl_pathc; idx++){
    core[idx].os_id = process_string_to_number((glob_results.gl_pathv[idx])+27);

    snprintf(path_buffer, 1024, "%s/topology/core_id", glob_results.gl_pathv[idx]);
    cpu_data = read_file(path_buffer);
    remove_last_newline(cpu_data);
    core[idx].core_id = process_string_to_number(cpu_data->buffer);
    free_file_buffer(cpu_data);

    snprintf(path_buffer, 1024, "%s/topology/physical_package_id", glob_results.gl_pathv[idx]);
    cpu_data = read_file(path_buffer);
    remove_last_newline(cpu_data);
    core[idx].socket_id = process_string_to_number(cpu_data->buffer);
    free_file_buffer(cpu_data);
  }

  globfree(&glob_results);
  qsort((void *)core, cores_count, sizeof(core_t), sort_cores_cmp);
}

void probe_system(comm_t *comms){
  index_t cores_count = get_cores_count();
  core_t *system_cores = (core_t *)shmalloc(sizeof(core_t)*cores_count);
  socket_t *system_sockets;
  node_t *this_node;
  index_t socket_count=0;
  index_t cores_per_socket;

  process_cores(system_cores, cores_count);
  for(int idx=0; idx < cores_count; idx++){
    if(system_cores[idx].socket_id > socket_count) socket_count = system_cores[idx].socket_id;
  }
  socket_count += 1;
  system_sockets = (socket_t *)shmalloc(sizeof(socket_t)*socket_count);
  for(int idx=0; idx < socket_count; idx++){
    cores_per_socket=0;
    for(int jdx=0; jdx < cores_count; jdx++){
      if(system_cores[jdx].socket_id == idx){
        cores_per_socket++;
      }
    }
    system_sockets[idx].cores = malloc(sizeof(core_t *)*cores_per_socket);
    cores_per_socket=0;
    for(int jdx=0; jdx < cores_count; jdx++){
      if(system_cores[jdx].socket_id == idx){
        system_sockets[idx].cores[cores_per_socket] = (core_t*)&(system_cores[jdx]);
        cores_per_socket++;
      }
    }
    system_sockets[idx].core_count = cores_per_socket;
    system_sockets[idx].socket_id = idx;
    system_sockets[idx].node_id = MY_PE;
  }
  this_node = (node_t *)shmalloc(sizeof(node_t));
  this_node->socket = system_sockets;
  this_node->socket_count = socket_count;
  this_node->node_id = MY_PE;
  comms->nodes = this_node;
}

static void wait_for_debugger(){
#ifdef DEBUG
  char hostname[1024];
  gethostname(hostname,1024);
  int gogogo=0;
  printf("[%s]Ready to debug on PID %i\n", hostname, getpid());
  while(gogogo == 0){};
  shmem_barrier_all();
#endif
}

int main(int argc, char **argv){
  index_t test_length, iterations_length;
  test_t *test_list;
  iteration_data_t *iterations;
  parsed_options_t input_parameters;
  comm_t *shmem_system = NULL;

#ifdef USE_SHMEM12
  shmem_init();
#else
  start_pes(0);
#endif
  global_npes = shmem_n_pes();
  global_my_pe = shmem_my_pe();

#ifdef USE_SHMEM12
  int major_version = 1, minor_version = 1;
  shmem_info_get_version(&major_version, &minor_version);
  printf("Using OpenSHMEM version %i.%i\n", major_version, minor_version);
#else
  printf("using OpenSHMEM version 1.1 or earlier\n");
#endif

  wait_for_debugger();

  process_params(argc, argv, &input_parameters, &test_list, &test_length, &iterations, &iterations_length);

  ORB_calibrate();

  if(input_parameters.affinity_test != 0){
    shmem_system = (comm_t *)shmalloc(sizeof(comm_t));
    probe_system(shmem_system);
  }

  if(input_parameters.affinity_test == 0){
    if(MY_PE == 0){
      fprintf(input_parameters.output_file, "Running tests\n");
    }
    run_tests(test_list, test_length, iterations, iterations_length, &input_parameters);
  } else {
    if(MY_PE == 0){
      fprintf(input_parameters.output_file, "Running affinity tests\n");
    }
    run_affinity_tests(test_list, test_length, iterations, iterations_length, &input_parameters, shmem_system);
  }

#ifdef USE_SHMEM12
  shmem_finalize();
#endif

  return 0;
}
