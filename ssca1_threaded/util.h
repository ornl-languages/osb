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

#ifndef __UTIL_H
#define __UTIL_H

#include <pairwise_align.h>
#include <types.h>
#include <stdio.h>

#ifdef SGI_SHMEM
#include <mpp/shmem.h>
#else
#include <shmem.h>
#endif

static inline int global_index_to_rank(const seq_t *in, const index_t codon_index){
  return codon_index / in->local_size;
}

static inline int global_index_to_local_index(const seq_t *in, const index_t codon_index){
  return codon_index % in->local_size;
}

static inline void fetch_from_seq(const seq_t *in, index_t const codon_index, codon_t *out){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  shmem_short_get((short *)out, &(typed_seq[local_index]), 1, target_ep);
}
static inline void write_to_seq(const seq_t *in, const index_t codon_index, codon_t data){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  short typed_data = (short)data;
  shmem_short_put(&(typed_seq[local_index]), &typed_data, 1, target_ep);
}
static inline void fetch_from_seq_nb(const seq_t *in, index_t const codon_index, codon_t *out){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  shmem_short_get_nbi((short *)out, &(typed_seq[local_index]), 1, target_ep);
}

void distribute_rng_seed(unsigned int new_seed);
void seed_rng(int adjustment);
void touch_memory(void *mem, index_t size);
index_t scrub_hyphens(good_match_t *A, seq_t *dest, seq_t *source, index_t length);
void assemble_acid_chain(good_match_t *A, char *result, seq_t *chain, index_t length);
void assemble_codon_chain(good_match_t *A, char *result, seq_t *chain, index_t length);
score_t simple_score(good_match_t *A, seq_t *main, seq_t *match);
seq_t *alloc_global_seq(index_t seq_size);
seq_t *alloc_local_seq(index_t seq_size);
void free_global_seq(seq_t *doomed);
void free_local_seq(seq_t *doomed);

#endif
