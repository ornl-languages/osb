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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// constant data, should remain the same.

static const char *a_similarity[] = {"A", "gct", "gcc", "gca", "gcg"};
static const char *c_similarity[] = {"C", "tgt", "tgc"};
static const char *d_similarity[] = {"D", "gat", "gac"};
static const char *e_similarity[] = {"E", "gaa", "gag"};
static const char *f_similarity[] = {"F", "ttt", "ttc"};
static const char *g_similarity[] = {"G", "ggt", "ggc", "gga", "ggg"};
static const char *h_similarity[] = {"H", "cat", "cac"};
static const char *i_similarity[] = {"I", "att", "atc", "ata"};
static const char *k_similarity[] = {"K", "aaa", "aag"};
static const char *l_similarity[] = {"L", "ttg", "tta", "ctt", "ctc", "cta", "ctg"};
static const char *m_similarity[] = {"M", "atg"};
static const char *n_similarity[] = {"N", "aat", "aac"};
static const char *p_similarity[] = {"P", "cct", "ccc", "cca", "ccg"};
static const char *q_similarity[] = {"Q", "caa", "cag"};
static const char *r_similarity[] = {"R", "cgt", "cgc", "cga", "cgg", "aga", "agg"};
static const char *s_similarity[] = {"S", "tct", "tcc", "tca", "tcg", "agt", "agc"};
static const char *t_similarity[] = {"T", "act", "acc", "aca", "acg"};
static const char *v_similarity[] = {"V", "gtt", "gtc", "gta", "gtg"};
static const char *w_similarity[] = {"W", "tgg"};
static const char *y_similarity[] = {"Y", "tat", "tac"};
static const char *star_similarity[] = {"*", "taa", "tag", "tga"};

static const char **similarity[] = {a_similarity, c_similarity, d_similarity, e_similarity, f_similarity, g_similarity, h_similarity, i_similarity, 
                                    k_similarity, l_similarity, m_similarity, n_similarity, p_similarity, q_similarity, r_similarity, s_similarity,
                                    t_similarity, v_similarity,w_similarity, y_similarity, star_similarity};

/* lengths for the similarity matrix above, */
static const int similarity_length[] = {5, 3, 3, 3, 3, 5, 3, 4, 3, 7, 2, 3, 5, 3, 7, 7, 5, 5, 2, 3, 4};

int x_encode_array[128] = {[0 ... 127] = STAR};

/* gen_sim_matrix */
/* generates a similarity matrix.  It defines codon similarity values. */
/* Input:
 *   int exact       - value of an exact codon match
 *   int similar     - value of a similar codon
 *   int dissimilar  - value for codons that are not similar at all
 *   int gapStart    - penelty to start gap
 *   int gapExtend   - penelty for each codon in the gap
 *   int matchLimit  - limit to the largest match.
 *
 * Output:
 *   sim_matrix_t *  - similarity matrix
 *      ->similarity - int  [64][64] a matrix of codon/codon similarity values
 *      ->aminoAcid  - char [64]     an array of codon to aminoAcid ararys
 *      ->bases      - char [5]      1-base encoding to 1 base letter
 *      ->codon      - char [64][3]  1-based codon to base letters table
 *      ->encode     - int  [128]    aminoAcid to the last codon number
 *      ->hyphen     - int           encoding to represent the hyphen
 *      ->star       - int           encoding to represent the star
 *      ->exact      - int           value for exactly matching codons
 *      ->similar    - int           value for similarly matching codons
 *      ->dissimilar - int           value for codons that don't match at all
 *      ->gapStart   - int           penelty to start gap
 *      ->gapExtend  - int           penelty for each codon in the gap
 *      ->matchLimit - int           longest match including hyphens
 */

sim_matrix_t *gen_sim_matrix(int exact, int similar, int dissimilar, int gapStart, int gapExtend, int matchLimit)
{
  char process_acid;
  int ccode = 0;
  sim_matrix_t *the_similarity_matrix = (sim_matrix_t*)malloc(sizeof(sim_matrix_t));
  the_similarity_matrix->star = STAR;
  the_similarity_matrix->hyphen = HYPHEN;
  //strcpy(the_similarity_matrix->codon[64], {(char)HYPHEN,(char)HYPHEN,(char)HYPHEN});
  the_similarity_matrix->codon[64][0] = (char)HYPHEN;
  the_similarity_matrix->codon[64][1] = (char)HYPHEN;
  the_similarity_matrix->codon[64][2] = (char)HYPHEN;
  the_similarity_matrix->aminoAcid[HYPHEN] = '-';
  strcpy(the_similarity_matrix->bases, "agct");
  for(int encode_index = 0; encode_index < 128; encode_index++) the_similarity_matrix->encode[encode_index] = STAR;
  /* 21 rows in similiarity matrix */
  for(int similarity_matrix_index = 0; similarity_matrix_index < 21; similarity_matrix_index++)
  {
    process_acid = similarity[similarity_matrix_index][0][0]; 
    for(int codon_index = 1; codon_index < similarity_length[similarity_matrix_index]; codon_index++)
    {
      ccode = 0;
      for(int base_index = 0; base_index < 3; base_index++)
      {
        switch(similarity[similarity_matrix_index][codon_index][base_index])
        {
          case 'a':
            ccode = 0 + 4 * ccode;
            break;
          case 'g':
            ccode = 1 + 4 * ccode;
            break;
          case 'c':
            ccode = 2 + 4 * ccode;
            break;
          case 't':
            ccode = 3 + 4 * ccode;
            break;
          default:
            fprintf(stderr, "Wandered into territory I wasn't supposed to, aborting.\n");
            abort();
            break;
        }
      }
      strcpy(the_similarity_matrix->codon[ccode],similarity[similarity_matrix_index][codon_index]);
      the_similarity_matrix->aminoAcid[ccode] = process_acid;
    }
    the_similarity_matrix->encode[(int)process_acid] = ccode;
  }
  
  /* We don't get fancy vector notation like in matlab, so let's do all the work in one big loop */

  for(int i=0; i<64; i++)
  {
    for(int j=0; j<64; j++)
    {
      if(i == j)
      {
        the_similarity_matrix->similarity[i][j]=exact;
      }
      else if(the_similarity_matrix->aminoAcid[i] == the_similarity_matrix->aminoAcid[j])
      {
        the_similarity_matrix->similarity[i][j]=similar;
      }
      else
      {
        the_similarity_matrix->similarity[i][j]=dissimilar;
      }
    }
  }

  the_similarity_matrix->exact = exact;
  the_similarity_matrix->similar = similar;
  the_similarity_matrix->dissimilar = dissimilar;
  the_similarity_matrix->gapStart = gapStart;
  the_similarity_matrix->gapExtend = gapExtend;
  the_similarity_matrix->matchLimit = matchLimit;

  return the_similarity_matrix;
}

/* release_sim_matrix
 * frees the sim_matrix, returning the memory to the OS.
 * Input:
 *   sim_matrix_t *doomed_matrix - the matrix to be freed
 * Output:
 *   None
 */

void release_sim_matrix(sim_matrix_t *doomed_matrix)
{
  free(doomed_matrix);
}
