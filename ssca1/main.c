/*
   This file is part of SSCA1.

   Copyright (C) 2008-2015, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SSCA1 developers at:
   bakermb@ornl.gov
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <parameters.h>
#include <gen_sim_matrix.h>
#include <sys/time.h>
#include <time.h>
#include <gen_scal_data.h>
#include <pairwise_align.h>
#include <scan_backwards.h>
#include <limits.h>
#include <util.h>

unsigned int random_seed;
int num_nodes;
int rank;

/* unsigned int get_dev_rand() - 
     routine to get an unsigned int from /dev/random.
   Intput-
     None
   Output-
     unsigned int- value from /dev/random
 */

unsigned int get_dev_rand()
{
  int rand_file = open("/dev/urandom", O_RDONLY);
  unsigned int value;
  ssize_t read_value = read(rand_file, &value, sizeof(unsigned int));
  close(rand_file);
  if(read_value != sizeof(unsigned int))
  {
    fprintf(stderr, "Error reading random values, aborting\n");
    abort();
  }
  return value;
}

/* void display_elapsed(struct timeval *start_time)
       Given a start time, display how much time has elapsed since thing
     Input- 
       struct timeval *start_time- pointer to the struct with the starting time
    Output-
       None
 */

void display_elapsed(struct timeval *start_time)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  long hours_elapsed = 0;
  long minutes_elapsed = 0;
  time_t seconds_elapsed = now.tv_sec - start_time->tv_sec;
  long miliseconds_elapsed = 0;
  long u_elapsed = now.tv_usec - start_time->tv_usec;
  if(u_elapsed < 0)
  {
    seconds_elapsed--;
    u_elapsed = 1000000 + u_elapsed;
  }

  if(seconds_elapsed > 60)
  {
    minutes_elapsed = seconds_elapsed / 60;
    seconds_elapsed = seconds_elapsed % 60;
  }

  if(minutes_elapsed > 60)
  {
    hours_elapsed = minutes_elapsed / 60;
    minutes_elapsed = minutes_elapsed % 60;
  }

  if(u_elapsed > 1000)
  {
    miliseconds_elapsed = u_elapsed / 1000;
    u_elapsed = u_elapsed % 1000;
  }

  printf("\n\tElapsed time: %li hour(s), %li minute(s), %li second(s), %li milliseconds,  %li micro second(s).\n", hours_elapsed, minutes_elapsed, seconds_elapsed, miliseconds_elapsed, u_elapsed);
}

/* int main(int argc, char **argv)
     Entry routine.  Calls each kernel once, displaying elapsed time
 */

int main(int argc, char **argv)
{
  parameters_t global_parameters;
  sim_matrix_t *sim_matrix;
  seq_data_t *seq_data;
  struct timeval start_time;
  good_match_t *A;
#ifdef USE_MPI3
  MPI_Info winfo;
#endif

#ifdef _OPENMP
  printf("Running with OpenMP\n");
#endif

#ifdef USE_MPI3
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Info_create(&winfo);
  MPI_Info_set(winfo, "same_size", "true");
  MPI_Info_set(winfo, "alloc_shm", "false");
  window_size = 1000000;
  window_base = malloc(window_size);
  MPI_Win_create(window_base, window_size,  1, winfo, MPI_COMM_WORLD, &window);
  MPI_Win_lock_all(0, window);
  next_window_address = window_base;
  MPI_Info_free(&winfo);
  printf("Running with MPI-3, world size is %d\n", num_nodes);
#else
#ifdef USE_SHMEM
  start_pes(0);
  num_nodes=shmem_n_pes();
  rank=shmem_my_pe();
  if(rank == 0)
    printf("Running with OpenSHMEM, npes = %i\n", num_nodes);
#else
  num_nodes=1;
  rank=0;
#endif
#endif

#ifdef USE_PREFETCH
  if(rank == 0) printf("Using OpenSHMEM prefetching\n");
#else
  if(rank == 0) printf("Disabling OpenSHMEM prefetching\n");
#endif

  init_parameters(&global_parameters);

  if(argc > 1 && !strcmp(argv[1],"--threads"))
  {
    global_parameters.threads = atoi(argv[2]);
  }
  else
  {
    global_parameters.threads = 1;
  }

  good_match_t *S[global_parameters.K2_MAX_REPORTS];
  memset(S, 0, sizeof(good_match_t *)*global_parameters.K2_MAX_REPORTS);

  if(global_parameters.ENABLE_VERIF || global_parameters.CONSTANT_RNG)
  {
    //printf("\n\tVerification run, using constant seed for RNG\n");
    // interesting values that have uncovered bugs in the past,
    // 2613174141 -- segfault caused by insert_validation producting two identical values
    // -550696422 -- segfault caused by the RNG producing 0.
    random_seed = (unsigned int)2613174141;
  }
  else
  {
    random_seed = (unsigned int)time(NULL); /* casting from time_t to unsigned int we can lose precision... no big deal here */
    random_seed += get_dev_rand();
    distribute_rng_seed(random_seed);
  }

  if(rank == 0){
    printf("HPCS SSCA #1 Bioinformatics Sequence Alignment Executable Specification:\nRunning...\n");

    printf("Using seed %u\n", random_seed);

    printf("\nScalable Data Generator - genScalData() beginning execution...\n");
  }

  /*
  int gogogo=0;
  printf("Ready to debug on pid=%i\n", getpid());
  while(gogogo == 0){
  }
  shmem_barrier_all();
  */

  gettimeofday(&start_time, NULL);

  sim_matrix = gen_sim_matrix(global_parameters.SIM_EXACT, global_parameters.SIM_SIMILAR, global_parameters.SIM_DISSIMILAR, global_parameters.GAP_START, global_parameters.GAP_EXTEND, global_parameters.MATCH_LIMIT);

  seq_data = gen_scal_data(sim_matrix, global_parameters.MAIN_SEQ_LENGTH, global_parameters.MATCH_SEQ_LENGTH, global_parameters.CONSTANT_RNG);

  if(rank == 0){
    display_elapsed(&start_time);

  if(global_parameters.ENABLE_VERIF)
  {
    verifyData(sim_matrix, seq_data);
  }
  }

  /* Kernel 1 run */

  if(rank == 0){
  printf("\nBegining Kernel 1 execution.\n");

  gettimeofday(&start_time, NULL);
  }
#ifdef USE_MPI3  
  QUIET();
#endif
  A=pairwise_align(seq_data, sim_matrix, global_parameters.K1_MIN_SCORE, global_parameters.K1_MAX_REPORTS, global_parameters.K1_MIN_SEPARATION);

  if(rank == 0){
  display_elapsed(&start_time);
  }

  /* Kernel 2 run */

  if(rank == 0){
  printf("\nBegining Kernel 2 execution.\n");

  gettimeofday(&start_time, NULL);
 
  scanBackward(A, global_parameters.K2_MAX_REPORTS, global_parameters.K2_MIN_SEPARATION);

  display_elapsed(&start_time);

  if(global_parameters.ENABLE_VERIF)
  {
    verify_alignment(A, global_parameters.K2_DISPLAY);
  }
  }
  //release_good_match(A);
  release_sim_matrix(sim_matrix);
  release_scal_data(seq_data);

  BARRIER_ALL();
#ifdef USE_MPI3
  MPI_Win_unlock_all(window);
#endif

  return 0;
}
