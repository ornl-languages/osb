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

#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#include <shoms.h>
#include <config.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <parameters.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

static struct option long_options[] = {
  {"version", no_argument, NULL, 1},
  {"help", no_argument, NULL, 2},
  {"off_cache", no_argument, NULL, 3},
  {"warmup", no_argument, NULL, 4},
  {"output", required_argument, NULL, 5},
  {"msglen", required_argument, NULL, 7},
  {"minsize", required_argument, NULL, 8},
  {"maxsize", required_argument, NULL, 9},
  {"time", required_argument, NULL, 10},
  {"input", required_argument, NULL, 11},
  {"affinity", no_argument, NULL, 12},
  {0,0,0,0}
};

void print_version(){
  printf("SHOMS version " SHOMS_VERSION_STRING "\n");
}

#define help_string(name,info) printf(" %-20s  %-52s\n", name, info);

void print_help(){

  printf("Usage: shoms  [OPTIONS]\n");

  help_string("[OPTIONS]", "");
  help_string("--version", "Print shoms version string then exit.");
  help_string("--help",    "Prints this help message");
  help_string("--affinity", "Uses affinity mode. Launch with 2 jobs and SHOMS");
  help_string("",           "will run a selection of tests across different");
  help_string("",           "cores between the two nodes. Launching with more");
  help_string("",           "than or fewer than 2 jobs will cause the test to abort.");
  help_string("--off_cache", "Works to minimize the effects of CPU caches.");
  help_string("",            "Greatly increases memory usage.");
  help_string("--warmup",   "Perform warmup runs before doing timed runs to make");
  help_string("",           "caches hot.");
  help_string("--msglen FILE_NAME", "Reads file FILE_NAME and uses the sizes listed");
  help_string("",                   "in the file for test message lengths. Formated");
  help_string("",                   "with one message size per line.");
  help_string("--minsize NUMBER", "Specifices the minimum size as NUMBER bytes");
  help_string("",                 "for message lengths when generating message");
  help_string("",                 "lengths automatically.");
  help_string("",                 "Ignored if --msglen is passed.");
  help_string("--maxsize NUMBER", "Specifices the maximum size as NUMBER bytes");
  help_string("",                 "for message lengths when generating message");
  help_string("",                 "lengths automatically.");
  help_string("",                 "Ignored if --msglen is passed.");
  help_string("--time NUMBER", "Specifies a soft limit of NUMBER seconds for");
  help_string("",              "the maximum amount of time spent on a particular");
  help_string("",              "message size for a particular test. This is not");
  help_string("",              "precise as a running test won't be interrupted.");
  help_string("--input FILE_NAME", "Specifies an input file called FILE_NAME that");
  help_string("",                   "defines which tests to run. Tests are named after");
  help_string("",                   "their OpenSHMEM function. File is formated with one");
  help_string("",                   "test per line.");

}

index_t process_string_to_number(char *input){
  errno = 0;
  index_t output = (index_t)strtol(input, NULL, 0);
  if(errno != 0){
    if(errno == ERANGE){
      if(output == LONG_MAX){
        fprintf(stderr, "Converted value (%s) is larger than what fits in long type\n", optarg);
      } else if(output == LONG_MIN) {
        fprintf(stderr, "Converted value (%s) is smaller than what fits in long type\n", optarg);
      } else {
        fprintf(stderr, "Unknown range error in converting value (%s)\n", optarg);
      }
    } else {
      fprintf(stderr, "Unrecognised error in converting value (%s)\n", optarg);
    }
    abort();
  }
  return output;
}

void free_file_buffer(buffer_from_file_t *doomed){
  free(doomed->buffer);
  free(doomed);
}

buffer_from_file_t *read_file(char *file_name){
  int error_code;
  size_t read_count;
  struct stat stat_buffer;
  buffer_from_file_t *return_value = malloc(sizeof(buffer_from_file_t));
  error_code = stat(file_name, &stat_buffer);
  assert(error_code == 0);
  return_value->size = stat_buffer.st_size;
  return_value->buffer = malloc(stat_buffer.st_size);
  
  FILE *input_file = fopen(file_name, "r");
  read_count = fread(return_value->buffer, 1, return_value->size, input_file);
  fclose(input_file);
  //assert(read_count == return_value->size);
  return_value->size = read_count;

  return_value->line_count = 1;
  for(int idx=0; idx < return_value->size; idx++){
    //Mostly because I'm sure I'll forget my own logic here
    //return_value->buffer[idx] == '\n': If this is a new line
    //idx < (return_value->size - 1): This is not the last new line. Also prevents the next one from seg faulting
    //return_value->buffer[idx+1] != '\n': The next character isn't a new line either (we'll skip it)
    if(return_value->buffer[idx] == '\n' && idx < (return_value->size - 1) && return_value->buffer[idx+1] != '\n'){
      return_value->line_count+=1;
    }
  }

  
  return return_value;
}

void extract_line(size_t *start, char *output_buffer, size_t max_out_length, char *in_buffer, size_t max_in_length){
  int idx=0;
  memset(output_buffer, '\0', max_out_length);

  for(idx=0; idx <= max_out_length; idx++){
    if( idx + *start == max_in_length || in_buffer[*start+idx] == '\n' || in_buffer[*start+idx] == '\0'){
      break;
    }
    output_buffer[idx] = in_buffer[*start+idx];
  }

  output_buffer[idx] = '\0';
  *start += idx + 1;
  if(*start > max_in_length){
    *start = max_in_length;
  }
}

void process_iteration_file(char *file_name, iteration_data_t **iterations, index_t *iterations_length){
  buffer_from_file_t *file_content = read_file(file_name);
  *iterations_length = file_content->line_count;
  *iterations = (iteration_data_t *)malloc(sizeof(iteration_data_t)*file_content->line_count);
  iteration_data_t *local_iterations = *iterations;
  size_t start = 0;
  int idx=0;
  const size_t max_length = 1024;
  char number_buffer[max_length];

  for(idx=0; idx < file_content->line_count; idx++){
    do{
      extract_line(&start, number_buffer, max_length, file_content->buffer, file_content->size);
      if(start == file_content->size && number_buffer[0] == '\0'){
        goto free_iteration_buffers;
      }
    }while(number_buffer[0] == '\0');

    local_iterations[idx].data_size = process_string_to_number(number_buffer);
    local_iterations[idx].iterations_count = decide_iterations(local_iterations[idx].data_size);
  }

free_iteration_buffers:
  *iterations_length = idx;
  free(file_content->buffer);
  free(file_content);
}

void string_lower_case(char *buffer, size_t max_size){
  for(int idx=0; idx < max_size; idx++){
    if(buffer[idx] == '\0') {
      break;
    }
    buffer[idx] = tolower(buffer[idx]);
  }
}

extern test_t *global_test_index;

void add_test(test_t *new_test, char *test_string, parsed_options_t *input_options){
  for(int idx=0; idx < MAX_TESTS; idx++){
    if(global_test_index[idx].name != NULL){
      if(strcmp(test_string, global_test_index[idx].name) == 0){
        memcpy(new_test, &(global_test_index[idx]), sizeof(test_t));
        return;
      }
    }
  }
  memset(new_test, '\0', sizeof(test_t));
  fprintf(input_options->output_file, "Could not find test %s, will be skipped\n", test_string);
}

void process_test_file(char *file_name, test_t **test_list, index_t *test_length, parsed_options_t *input_options){
  buffer_from_file_t *file_content = read_file(file_name);
  *test_length = file_content->line_count;
  *test_list = (test_t *)malloc(sizeof(test_t)*file_content->line_count);
  int idx=0;
  size_t start = 0;
  const size_t max_length = 1024;
  char test_buffer[max_length];
  test_t *local_tests = (test_t *)*test_list;

  for(idx = 0; idx < file_content->line_count; idx++){
    do{
      extract_line(&start, test_buffer, max_length, file_content->buffer, file_content->size);
      if(start == file_content->size && test_buffer[0] == '\0'){
        goto free_test_buffers;
      }
    }while(test_buffer[0] == '\0');

    string_lower_case(test_buffer, max_length);
    add_test(&(local_tests[idx]), test_buffer, input_options);
  }
free_test_buffers:
  *test_length = idx;
  free(file_content->buffer);
  free(file_content);
}

//In our parameters processing, if the return of getopt_long_only() is 1 we exit, on 0 success, on -1 error
void process_params(int argc, char **argv, parsed_options_t *set_options, test_t **test_list, index_t *test_length, iteration_data_t **iterations, index_t *iterations_length){
  int opt_index;
  int value;
  index_t option_string_length=0;
  char *message_file_path = NULL;
  char *input_file_path = NULL;

  memset(set_options, '\0', sizeof(parsed_options_t));
  set_options->maximum_size = MAXSIZE;
  set_options->minimum_size = MINSIZE;
  set_options->output_file = stdout;

  while(1){
    value = getopt_long_only(argc, argv, "", long_options, &opt_index);
    if(value == -1){
      break;
    }
    switch(value) {
      case 1:
        print_version();
    #ifdef USE_SHMEM12
        shmem_finalize();
    #endif
        exit(0);
      case 2:
        print_version();
        print_help();
    #ifdef USE_SHMEM12
        shmem_finalize();
    #endif
        exit(0);
      case 3:
        set_options->disable_cache = 1;
        break;
      case 4:
        set_options->warmup_run = 1;
        break;
      case 5:
        option_string_length = strlen(optarg)+1;
        set_options->output_file_path = malloc(option_string_length);
        memcpy(set_options->output_file_path, optarg, option_string_length);
        break;
      case 7:
        option_string_length = strlen(optarg)+1;
        message_file_path = malloc(option_string_length);
        memcpy(message_file_path, optarg, option_string_length);
        break;
      case 8:
        set_options->minimum_size = process_string_to_number(optarg);
        break;
      case 9:
        set_options->maximum_size = process_string_to_number(optarg);
        break;
      case 10:
        set_options->run_time = process_string_to_number(optarg);
        break;
      case 11:
        option_string_length = strlen(optarg)+1;
        input_file_path = malloc(option_string_length);
        memcpy(input_file_path, optarg, option_string_length);
        break;
      case 12:
        set_options->affinity_test = 1;
        if(N_PES != 2){
          fprintf(stderr, "The --affinity flag requires the use of exactly 2 PEs!\n");
          abort();
        }
        break;
      case '?':
        break;
      default:
        fprintf(stderr, "Unknown option 0%o\n", value);
    }
  }

  init_tests();

  if(set_options->output_file_path != NULL) {
    set_options->output_file = fopen(set_options->output_file_path, "w");
  }

  if(input_file_path != NULL){
    process_test_file(input_file_path, test_list, test_length, set_options);
    free(input_file_path);
  } else {
    if(set_options->affinity_test == 0){
      *test_length = all_tests(test_list);
      if(shmem_my_pe() == 0)fprintf(set_options->output_file, "Created all test list.\n");
    } else {
      *test_length = affinity_tests(test_list);
      if(shmem_my_pe() == 0)fprintf(set_options->output_file, "created affinity test list.\n");
    }
  }

  if(shmem_my_pe() == 0) fprintf(set_options->output_file, "Will be running with %lu different tests\n", (*test_length));

  if(message_file_path != NULL) {
    process_iteration_file(message_file_path, iterations, iterations_length);
    free(message_file_path);
  } else {
    *iterations_length = init_iterations(iterations, set_options->minimum_size, set_options->maximum_size);
  }

  if(shmem_my_pe() == 0) fprintf(set_options->output_file, "Will be running with %lu different size configurations\n", (*iterations_length));
}
