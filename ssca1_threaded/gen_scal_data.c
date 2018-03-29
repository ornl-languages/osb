/*
   This file is part of SSCA1.

   Copyright (C) 2008-2017, UT-Battelle, LLC.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sort.h>
#include <gen_scal_data.h>
#include <types.h>
#include <util.h>

#ifdef _OPENMP
#include <omp.h>
#endif

extern unsigned int random_seed;
extern int rank;
extern int num_nodes;

char validations[2][3][32] =
  {
    {"ACDEFG*IDENTICAL*HIKLMN", "ACDEFG*MISQRMATCHES*HIKLMN", "ACDEFG*STARTGAPMIDSTEND*HIKLMN"},
    {"MNLKIH*IDENTICAL*GFEDCA", "MNLKIH*MISRQMATCHES*GFEDCA", "MNLKIH*STARTMIDSTGAPEND*GFEDCA"}
  };

/* print the output */

void verifyData(sim_matrix_t *simMatrix, seq_data_t *seqData)
{
  printf("\n");
  printf("   Length of main sequence in codons: %lu\n", seqData->main->length);
  printf("  Length of match sequence in codons: %lu\n", seqData->match->length);
  printf("  Weight for exactly matching codons: %i\n", simMatrix->exact);
  printf("           Weight for similar codons: %i\n", simMatrix->similar);
  printf("        Weight for dissimilar codons: %i\n", simMatrix->dissimilar);
  printf("              Penalty to start a gap: %i\n", simMatrix->gapStart);
  printf("     Penalty for each codon in a gap: %i\n", simMatrix->gapExtend);
}

index_t *gen_indexes(int num_indexes, index_t rand_max, index_t min_seperation) {
  int not_done;
  index_t *indexes = (index_t*)malloc(sizeof(index_t) * num_indexes);

  do{
    not_done = 0;
    for(int idx=0; idx < num_indexes; idx++) {
      indexes[idx] = rand()%rand_max;
    }
    for(int idx=0; idx < num_indexes; idx++) {
      for(int jdx=idx+1; jdx < num_indexes; jdx++) {
        if(indexes[idx] - indexes[jdx] < min_seperation) not_done = 1;
      }
    }
  } while(not_done);
  return indexes;
}

void create_sequence(seq_t *sequence, char validations[][32], int num_validations, sim_matrix_t *simMatrix){
  index_t total_length = sequence->length;
  index_t end;
  index_t *indexes = gen_indexes(num_validations, total_length-32, 32);

  for(index_t idx=0; idx < sequence->local_size; idx++) {
    sequence->sequence[idx] = rand()%64;
  }

  if(rank == 0){
    for(int idx=0; idx < num_validations; idx++) {
      end = strlen(validations[idx]);
      printf("Inserting sequence %s in location %lu\n", validations[idx], indexes[idx]);
      for(int jdx=0; jdx < end; jdx++){
        write_to_seq(sequence, indexes[idx]+jdx, simMatrix->encode[(int)validations[idx][jdx]]);
      }
    }
  }
  free(indexes);

}

seq_data_t *gen_scal_data( sim_matrix_t *simMatrix, index_t mainLen, index_t matchLen, int constant_rng) {
  seq_data_t *new_scal_data = (seq_data_t *)malloc(sizeof(seq_data_t));
  int validation_length, validation_size=0;

  memset(new_scal_data, '\0', sizeof(seq_data_t));

  for(int jdx=0; jdx < 3; jdx++){
    validation_length = strlen(validations[0][jdx]);
    validation_size += validation_length;
    if(validation_length > new_scal_data->max_validation)
     new_scal_data->max_validation = validation_length;
  }

  new_scal_data->max_validation -= 12;

  index_t main_size_with_validation = mainLen + validation_size;
  index_t match_size_with_validation = mainLen + validation_size;

  new_scal_data->main = alloc_global_seq(main_size_with_validation);
  new_scal_data->match = alloc_global_seq(match_size_with_validation);

  seq_t *gen_sequences[2] = {new_scal_data->main, new_scal_data->match};
  seed_rng(rank + 1);

#ifdef _OPENMP
  int thread_number = 2;
  if(constant_rng==1){
    thread_number = 1;
  }

#pragma omp parallel for num_threads(thread_number)
#endif
  for(int idx=0; idx < 2; idx++){
    touch_memory(gen_sequences[idx]->sequence, sizeof(codon_t)*gen_sequences[idx]->backing_memory);
    create_sequence(gen_sequences[idx], validations[idx], 3, simMatrix);
  }

  return new_scal_data;
}

void release_scal_data(seq_data_t *doomed_seq) {
  free_global_seq(doomed_seq->main);
  free_global_seq(doomed_seq->match);
  free((void *)doomed_seq);
}
