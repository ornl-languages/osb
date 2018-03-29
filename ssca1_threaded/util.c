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
#include <util.h>
#include <pairwise_align.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

extern unsigned int random_seed;
extern int num_nods;
extern int rank;
long seed_psync[_SHMEM_BCAST_SYNC_SIZE];

void distribute_rng_seed(unsigned int new_seed){
  for(int idx=0; idx < _SHMEM_BCAST_SYNC_SIZE; idx++){
    seed_psync[idx] = _SHMEM_SYNC_VALUE;
  }
  shmem_barrier_all();
  shmem_broadcast32(&random_seed,&new_seed,1,0,0,0,num_nodes,seed_psync);
}

void seed_rng(int adjustment){
  srand(random_seed + adjustment);
}

void extend_seq(seq_t *extended, index_t extend_size){
  codon_t *realloc_temp = (codon_t*)realloc(extended->sequence, sizeof(codon_t) * (extended->backing_memory + extend_size));
  if(realloc_temp == NULL)
  {
    printf("Realloc error\n");
    abort();
  }
  extended->sequence = realloc_temp;
  extended->backing_memory += extend_size;
}

seq_t *alloc_global_seq(index_t size){
  seq_t *new = malloc(sizeof(seq_t));
  new->local_size = size / num_nodes;
  new->length = new->local_size * num_nodes;
  new->backing_memory = new->local_size;
  new->sequence = shmem_malloc(sizeof(codon_t)*new->local_size);
  if(new->sequence == NULL){
    printf("Shmalloc error\n");
    abort();
  }
  return new;
}

void free_global_seq(seq_t *doomed){
  if(doomed == NULL)return;
  shmem_free(doomed->sequence);
  free(doomed);
}

seq_t *alloc_local_seq(index_t size){
  seq_t *new = (seq_t*)malloc(sizeof(seq_t));
  new->local_size = size;
  new->length = size;
  new->backing_memory = size;
  new->sequence = (codon_t*)malloc(sizeof(codon_t)*new->local_size);
  if(new->sequence == NULL){
    printf("malloc error\n");
    abort();
  }
  return new;
}

void free_local_seq(seq_t *doomed){
  if(doomed == NULL)return;
  free(doomed->sequence);
  free(doomed);
}

void touch_memory(void *mem, size_t size) {
  index_t page_size = sysconf(_SC_PAGESIZE);
  index_t *this_memory = (index_t *)mem;
  index_t size_increment = page_size / sizeof(index_t);
  index_t size_max = size / sizeof(index_t);
  for(index_t idx=0; idx < size_max; idx+=size_increment) {
    this_memory[idx] = 0;
  }
  this_memory[size_max-1] = 0;
}

/* Fix up a sequence to remove the gaps
   Input-
        good_match_t *A    - the match struct for the sequence.  Used for the hyphen member
        int *dest          - destination array.  Needs to be allocated before it is passed in
        int *source        - sequence to be scrubbed
        int length         - length of the source.  Dest should be the same size
   Output-
        int *dest          - will be full of a gapless sequence
        int (return value) - size of dest
*/

index_t scrub_hyphens(good_match_t *A, seq_t *dest, seq_t *source, index_t length)
{
  index_t source_index=0, dest_index=0;
  while(source_index < length)
  {
    while(source_index < length && source->sequence[source_index] == A->simMatrix->hyphen) source_index++;
    dest->sequence[dest_index] = source->sequence[source_index];
    source_index++; dest_index++;
  }
  dest->length = dest_index-1;
  return dest_index-1;
}

/* Helper function for displaying acid chains.  This function will take a chain and make an
   ascii representation of the acids as defined in A
   Input-
       good_match_t *A    - the match struct for the sequence.
       char *result       - the resulting chain as a sequnce of acids
       int *chain         - the sequence to convert into ascii
       int length         - size of the chain
*/

void assemble_acid_chain(good_match_t *A, char *result, seq_t *chain, index_t length)
{
  memset(result, '\0', length);
  if(length > chain->length) length = chain->length;
  for(int idx=0; idx < length; idx++)
  {
    result[idx] = (char)A->simMatrix->aminoAcid[chain->sequence[idx]];
  }
  result[length] = '\0';  
}

/* Helper function for displaying codon chains.  This function will take a chain and make an
   ascii representation of the codon sequence out of it
   Input-
       good_match_t *A    - the match struct for the sequence.
       char *result       - the resulting chain as a sequnce of codons
       int *chain         - the sequence to convert into ascii
       int length         - size of the chain
*/

void assemble_codon_chain(good_match_t *A, char *result, seq_t *chain, index_t length)
{
  memset(result, '\0', length);
  if(length > chain->length) length = chain->length;
  for(int idx=0; idx < length; idx++)
  {
    if(chain->sequence[idx] == HYPHEN)
    {
      result[idx*3] = '-';
      result[idx*3+1] = '-';
      result[idx*3+2] = '-';
    }
    else
    {
      result[idx*3] = (char)A->simMatrix->codon[chain->sequence[idx]][0];
      result[idx*3+1] = (char)A->simMatrix->codon[chain->sequence[idx]][1];
      result[idx*3+2] = (char)A->simMatrix->codon[chain->sequence[idx]][2];
    }
  }
  result[length*3] = '\0';
}

/* Very simple scoring routine.  Compares main to match.  Both must be
   at least length long.
   Input-
       goot_match_t *A - the match structure used to generate main and match
       int main[]      - the main sequences
       int match[]     - the sequence to compare to main
       int length      - the size of smallest of main or match
  Output-
       int             - the score of the match
*/

score_t simple_score(good_match_t *A, seq_t *main, seq_t *match)
{
  score_t score = 0;
  int mainMatch = 1;
  int matchMatch = 1;
  index_t length = main->length;
  if(match->length < length){
    length = match->length;
  }
  
  // recompute score by a brain dead simple method
  for(int i=0; i < length; i++)
  {
    if(main->sequence[i] == A->simMatrix->hyphen)
    {
      if(mainMatch == 1)
      {
        mainMatch = 0;
        score = score - A->simMatrix->gapStart;
      }
      score = score - A->simMatrix->gapExtend;
      continue;
    }
    if(match->sequence[i] == A->simMatrix->hyphen)
    {
      if(matchMatch == 1)
      {
        matchMatch = 0;
        score = score - A->simMatrix->gapStart;
      }
      score = score - A->simMatrix->gapExtend;
      continue;
    }
    mainMatch = 1;
    matchMatch = 1;
    score = score + A->simMatrix->similarity[main->sequence[i]][match->sequence[i]];
  }
  return score;
}
