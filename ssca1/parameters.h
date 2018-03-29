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

#ifndef _PARAMETERS_H
#define _PARAMETERS_H

typedef struct _params
{
 int ENABLE_PAUSE;
 int ENABLE_VERIF;
 int ENABLE_DEBUG;
 int CONSTANT_RNG;

 double MAIN_SEQ_LENGTH;
 double MATCH_SEQ_LENGTH;

 int SIM_EXACT;
 int SIM_SIMILAR;
 int SIM_DISSIMILAR;
 int GAP_START;
 int GAP_EXTEND;
 int MATCH_LIMIT;

 int MISMATCH_PENALTY;
 int SPACE_PENALTY;

 int K1_MIN_SCORE;
 int K1_MIN_SEPARATION;
 int K1_MAX_REPORTS;

 int K2_MIN_SEPARATION;
 int K2_MAX_REPORTS;
 int K2_DISPLAY;

 int K3_MIN_SCORE;
 int K3_MIN_SEPARATION;
 int K3_MAX_REPORTS;
 int K3_MAX_MATCH;
 int K3_DISPLAY;

 int K4_DISPLAY;

 int K5_DISPLAY;

 int threads;
} parameters_t;

void init_parameters(parameters_t *parameters);

#endif
