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

#include <gen_sim_matrix.h>
#include <types.h>

#ifndef _GEN_SCAL_DATA
#define _GEN_SCAL_DATA

/*
 * seqData         - [structure] holds the generated sequences.
 *  main          - [1D uint8 array] generated main codon sequence (1-64).
 *  match         - [1D uint8 array] generated match codon sequence (1-64).
 *  maxValidation - [Integer] longest matching validation string.
 */

/*
typedef struct _seq_data
{
  codon_t *main;
  codon_t *match;
  index_t mainLen;
  index_t node_main;
  index_t matchLen;
  index_t node_match;
  int maxValidation;
} seq_data_t;
*/

seq_data_t *gen_scal_data( sim_matrix_t *simMatrix, index_t mainLen, index_t matchLen, int constant_rng);
void release_scal_data(seq_data_t *doomed_scal_data);
void verifyData(sim_matrix_t *simMatrix, seq_data_t *seqData);

#endif
