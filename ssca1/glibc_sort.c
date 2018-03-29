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

#include <sort.h>
#include <stdlib.h>

/* function to be passed to to glibc's qsort function */

int sort_cmp(const void *first, const void *second)
{
  index_t *number_a = (index_t *)first;
  index_t *number_b = (index_t *)second;
  return (int)(((score_t)*number_a) - ((score_t)*number_b));
}

int ends_cmp(const void *first, const void *second){
  sort_ends_t *end_a = (sort_ends_t *)first;
  sort_ends_t *end_b = (sort_ends_t *)second;
  return (int)(end_b->score - end_a->score);
}

/* A wrapper around glibc's qsort.
     Input-
         int *numbers   - values to be sorted
         int array_size - size of the numbers array
     Output-
         int *numbers   - sorted values
*/

void sort(index_t numbers[], index_t array_size)
{
  qsort(numbers, array_size, sizeof(index_t), sort_cmp);
}

/* Sort the values and return a list of indexes of the sorted values
     Input-
         int *numbers   - values to be sorted
         int *indexes   - an int array size of array_size
         int array_size - size of the numbers array
     Output-
         int *numbers   - sorted values
         int *indexes   - the index where the value used to be before sorting
*/

void index_sort(score_t numbers[], index_t indexes[], index_t array_size)
{
  index_t big_index[array_size][2];
  for(index_t idx=0; idx < array_size; idx++)
  {
    big_index[idx][0] = (index_t)numbers[idx];
    big_index[idx][1] = idx;
  }

  qsort(big_index, array_size, sizeof(index_t)*2, sort_cmp);

  for(int idx=0; idx < array_size; idx++)
  {
    numbers[idx] = (score_t)big_index[idx][0];
    indexes[idx] = big_index[idx][1];
  }
}

void ends_sort(sort_ends_t *ends, index_t array_size){
  qsort(ends, array_size, sizeof(sort_ends_t), ends_cmp);
}
