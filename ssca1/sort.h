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

#ifndef _SORT_H
#define _SORT_H

#include <types.h>

void index_sort(score_t numbers[], index_t indexes[], index_t array_size);
void sort(index_t numbers[], index_t array_size);

typedef struct {
  score_t score;
  index_t main_end;
  index_t match_end;
} sort_ends_t;

void ends_sort(sort_ends_t *ends, index_t array_size);

#endif
