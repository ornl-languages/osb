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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <parameters.h>

void init_parameters(parameters_t *parameters)
{
  double scale;
  static const char *scale_name = "SCALE";
  char *result = getenv(scale_name);
  if(result == NULL)
  {
    scale = 22.0;
  }
  else
  {
    scale = atof(result);
  }

  parameters->ENABLE_PAUSE = 0;
  parameters->ENABLE_VERIF = 1;
  parameters->ENABLE_DEBUG = 0;
  parameters->CONSTANT_RNG = 1;

  /*
   *  Scalable Data Generator parameters.
   */

  parameters->MAIN_SEQ_LENGTH   = ceil(pow(2.0, (scale/2.0))); /* Total main codon sequence length. */
  parameters->MATCH_SEQ_LENGTH  = ceil(pow(2.0, (scale/2.0))); /* Total match codon sequence length. */

  /*
   * Kernel parameters.
   */

  /* Kernel 1/2/3 codon sequence similarity scoring function */
  parameters->SIM_EXACT         = 5;                  /* >0  Exact codon match */
  parameters->SIM_SIMILAR       = 4;                  /*     Amino acid (or Stop) match */
  parameters->SIM_DISSIMILAR    = -3;                 /* <0  Different amino acids */
  parameters->GAP_START         = 5;                  /* >=0 Gap-start penalty */
  parameters->GAP_EXTEND        = 2;                  /* >0  Gap-extension penalty */
  parameters->MATCH_LIMIT       = 3*(int)scale;       /* Longest interesting match */

  /* Kernel 4/5 base sequence difference scoring function */
  parameters->MISMATCH_PENALTY  = 20;                  /* >0  Mismatch penalty */
  parameters->SPACE_PENALTY     = 30;                  /* >0  Penalty for each space in a gap */

  /* Kernel 1 */
  parameters->K1_MIN_SCORE      = 20;                 /* >0  Minimum end-point score */
  parameters->K1_MIN_SEPARATION = 5;                  /* >=0 Minimum end-point separation */
  parameters->K1_MAX_REPORTS    = 200;                /* >0  Maximum end-points reported to K2 */

  /* Kernel 2 */
  parameters->K2_MIN_SEPARATION = parameters->K1_MIN_SEPARATION;  /* >=0 Minimum start-point separation */
  parameters->K2_MAX_REPORTS = ceil(parameters->K1_MAX_REPORTS/2);/* >0 Maximum sequences reported to K3 */
  parameters->K2_DISPLAY        = 10;                 /* >=0 Number of reports to display */

  /* Kernel 3 */
  parameters->K3_MIN_SCORE      = 10;                 /* >0 Minimum end-point score */
  parameters->K3_MIN_SEPARATION = parameters->K1_MIN_SEPARATION;  /* >=0 Minimum end-point separation */
  parameters->K3_MAX_REPORTS    = 100;                /* >0 Maximum sequences reported to K4 */
  parameters->K3_MAX_MATCH      = 2.0;                /* match_limit = MAX_MATCH * seq_len */
  parameters->K3_DISPLAY        = 10;                 /* >=0 Number of reports to display */

  /* Kernel 4 */
  parameters->K4_DISPLAY        = 10;                 /* >=0 Number of reports to display */

  /* Kernel 5 */
  parameters->K5_DISPLAY        = 100;                /* >=0 Number of reports to display */

  /* sanity tests */

  if(parameters->SIM_EXACT <= 0 || parameters->SIM_DISSIMILAR >= 0 || parameters->GAP_START < 0 || parameters->GAP_EXTEND <= 0)
  {
    fprintf(stderr, "Similarity parameters set in getUserParameters are invalid\n");
    abort();
  }

  if(parameters->MISMATCH_PENALTY <= 0 || parameters->SPACE_PENALTY <= 0)
  {
    fprintf(stderr, "Difference parameters set in getUserParameters are invalid\n");
    abort();
  }

  if(parameters->K1_MAX_REPORTS <= 0)
  {
    fprintf(stderr, "Kernel 1 parameters set in getUserParameters are invalid\n");
    abort();
  }

  if(parameters->K2_MAX_REPORTS <= 0)
  {
    fprintf(stderr, "Kernel 2 parameters set in getUserParameters are invalid\n");
    abort();
  }

  if(parameters->K3_MAX_REPORTS <= 0)
  {
    fprintf(stderr, "Kernel 3 parameters set in getUserParameters are invalid\n");
    abort();
  }
}
