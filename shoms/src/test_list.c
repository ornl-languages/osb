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

#include <shoms.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include <string.h>
#include <stdio.h>

#define SET_SHOMS_TEST(test_name, alloc_function, clean_function, per_alloc_function, per_clean_function, stats_function) \
        ((test_t){test_ ## test_name, alloc_function, clean_function, per_alloc_function, per_clean_function, \
        calculate_ ## stats_function ## _performance, #test_name, NULL})

test_t *global_test_index;

void free_distributed_buffer(void *buffer);

void init_tests(){
  global_test_index = (test_t *)malloc(sizeof(test_t)*NUM_TESTS);
  memset(global_test_index, '\0', sizeof(test_t)*NUM_TESTS);

  global_test_index[0]  = SET_SHOMS_TEST(shmem_local_read, init_distributed_32bit_bufffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[1]  = SET_SHOMS_TEST(shmem_local_write, init_distributed_32bit_bufffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[2]  = SET_SHOMS_TEST(shmalloc, init_shmalloc, cleanup_shmalloc, NULL, cleanup_per_iteration_shmalloc, global_no_bw);
  global_test_index[3]  = SET_SHOMS_TEST(shfree, NULL, NULL, init_distributed_32bit_bufffer, NULL, global_no_bw);
  global_test_index[4]  = SET_SHOMS_TEST(shmem_short_p, init_distributed_short_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[5]  = SET_SHOMS_TEST(shmem_int_p, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[6]  = SET_SHOMS_TEST(shmem_long_p, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[7]  = SET_SHOMS_TEST(shmem_longlong_p, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[8]  = SET_SHOMS_TEST(shmem_float_p, init_distributed_float_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[9]  = SET_SHOMS_TEST(shmem_double_p, init_distributed_double_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[10] = SET_SHOMS_TEST(shmem_longdouble_p, init_distributed_longdouble_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[11] = SET_SHOMS_TEST(shmem_short_put, init_sym_and_local_short, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[12] = SET_SHOMS_TEST(shmem_int_put, init_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[13] = SET_SHOMS_TEST(shmem_long_put, init_sym_and_local_long, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[14] = SET_SHOMS_TEST(shmem_longlong_put, init_sym_and_local_longlong, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[15] = SET_SHOMS_TEST(shmem_float_put, init_sym_and_local_float, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[16] = SET_SHOMS_TEST(shmem_double_put, init_sym_and_local_double, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[17] = SET_SHOMS_TEST(shmem_longdouble_put, init_sym_and_local_longdouble, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[18] = SET_SHOMS_TEST(shmem_put32, init_sym_and_local_32bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[19] = SET_SHOMS_TEST(shmem_put64, init_sym_and_local_64bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[20] = SET_SHOMS_TEST(shmem_put128, init_sym_and_local_128bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[21] = SET_SHOMS_TEST(shmem_short_iput, init_strided_sym_and_local_short, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[22] = SET_SHOMS_TEST(shmem_int_iput, init_strided_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[23] = SET_SHOMS_TEST(shmem_long_iput, init_strided_sym_and_local_long, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[24] = SET_SHOMS_TEST(shmem_longlong_iput, init_strided_sym_and_local_longlong, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[25] = SET_SHOMS_TEST(shmem_float_iput, init_strided_sym_and_local_float, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[26] = SET_SHOMS_TEST(shmem_double_iput, init_strided_sym_and_local_double, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[27] = SET_SHOMS_TEST(shmem_longdouble_iput, init_strided_sym_and_local_longdouble, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[28] = SET_SHOMS_TEST(shmem_iput32, init_strided_sym_and_local_32bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[29] = SET_SHOMS_TEST(shmem_iput64, init_strided_sym_and_local_64bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[30] = SET_SHOMS_TEST(shmem_iput128, init_strided_sym_and_local_128bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[31] = SET_SHOMS_TEST(shmem_short_g, init_distributed_short_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[32] = SET_SHOMS_TEST(shmem_int_g, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[33] = SET_SHOMS_TEST(shmem_long_g, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[34] = SET_SHOMS_TEST(shmem_longlong_g, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[35] = SET_SHOMS_TEST(shmem_float_g, init_distributed_float_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[36] = SET_SHOMS_TEST(shmem_double_g, init_distributed_double_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[37] = SET_SHOMS_TEST(shmem_longdouble_g, init_distributed_longdouble_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[38] = SET_SHOMS_TEST(shmem_short_get, init_sym_and_local_short, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[39] = SET_SHOMS_TEST(shmem_int_get, init_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[40] = SET_SHOMS_TEST(shmem_long_get, init_sym_and_local_long, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[41] = SET_SHOMS_TEST(shmem_longlong_get, init_sym_and_local_longlong, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[42] = SET_SHOMS_TEST(shmem_float_get, init_sym_and_local_float, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[43] = SET_SHOMS_TEST(shmem_double_get, init_sym_and_local_double, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[44] = SET_SHOMS_TEST(shmem_longdouble_get, init_sym_and_local_longdouble, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[45] = SET_SHOMS_TEST(shmem_get32, init_sym_and_local_32bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[46] = SET_SHOMS_TEST(shmem_get64, init_sym_and_local_64bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[47] = SET_SHOMS_TEST(shmem_get128, init_sym_and_local_128bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[48] = SET_SHOMS_TEST(shmem_short_iget, init_strided_sym_and_local_short, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[49] = SET_SHOMS_TEST(shmem_int_iget, init_strided_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[50] = SET_SHOMS_TEST(shmem_long_iget, init_strided_sym_and_local_long, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[51] = SET_SHOMS_TEST(shmem_longlong_iget, init_strided_sym_and_local_longlong, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[52] = SET_SHOMS_TEST(shmem_float_iget, init_strided_sym_and_local_float, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[53] = SET_SHOMS_TEST(shmem_double_iget, init_strided_sym_and_local_double, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[54] = SET_SHOMS_TEST(shmem_longdouble_iget, init_strided_sym_and_local_longdouble, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[55] = SET_SHOMS_TEST(shmem_iget32, init_strided_sym_and_local_32bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[56] = SET_SHOMS_TEST(shmem_iget64, init_strided_sym_and_local_64bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[57] = SET_SHOMS_TEST(shmem_iget128, init_strided_sym_and_local_128bit, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[58] = SET_SHOMS_TEST(shmem_short_and_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[59] = SET_SHOMS_TEST(shmem_int_and_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[60] = SET_SHOMS_TEST(shmem_long_and_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[61] = SET_SHOMS_TEST(shmem_longlong_and_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[62] = SET_SHOMS_TEST(shmem_short_or_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[63] = SET_SHOMS_TEST(shmem_int_or_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[64] = SET_SHOMS_TEST(shmem_long_or_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[65] = SET_SHOMS_TEST(shmem_longlong_or_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[66] = SET_SHOMS_TEST(shmem_short_xor_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[67] = SET_SHOMS_TEST(shmem_int_xor_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[68] = SET_SHOMS_TEST(shmem_long_xor_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[69] = SET_SHOMS_TEST(shmem_longlong_xor_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[70] = SET_SHOMS_TEST(shmem_short_min_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[71] = SET_SHOMS_TEST(shmem_int_min_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[72] = SET_SHOMS_TEST(shmem_long_min_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[73] = SET_SHOMS_TEST(shmem_longlong_min_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[74] = SET_SHOMS_TEST(shmem_short_max_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[75] = SET_SHOMS_TEST(shmem_int_max_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[76] = SET_SHOMS_TEST(shmem_long_max_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[77] = SET_SHOMS_TEST(shmem_longlong_max_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[78] = SET_SHOMS_TEST(shmem_short_sum_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[79] = SET_SHOMS_TEST(shmem_int_sum_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[80] = SET_SHOMS_TEST(shmem_long_sum_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[81] = SET_SHOMS_TEST(shmem_longlong_sum_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[82] = SET_SHOMS_TEST(shmem_short_prod_to_all, init_collective_short, free_collective, NULL, NULL, global);
  global_test_index[83] = SET_SHOMS_TEST(shmem_int_prod_to_all, init_collective_int, free_collective, NULL, NULL, global);
  global_test_index[84] = SET_SHOMS_TEST(shmem_long_prod_to_all, init_collective_long, free_collective, NULL, NULL, global);
  global_test_index[85] = SET_SHOMS_TEST(shmem_longlong_prod_to_all, init_collective_longlong, free_collective, NULL, NULL, global);
  global_test_index[86] = SET_SHOMS_TEST(shmem_broadcast32, init_shmem_broadcast, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[87] = SET_SHOMS_TEST(shmem_broadcast64, init_shmem_broadcast, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[88] = SET_SHOMS_TEST(shmem_collect32, init_shmem_collect32, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[89] = SET_SHOMS_TEST(shmem_collect64, init_shmem_collect64, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[90] = SET_SHOMS_TEST(shmem_fcollect32, init_shmem_collect32, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[91] = SET_SHOMS_TEST(shmem_fcollect64, init_shmem_collect64, free_shmem_broadcast, NULL, NULL, global);
  global_test_index[92] = SET_SHOMS_TEST(shmem_int_swap, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[93] = SET_SHOMS_TEST(shmem_swap, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[94] = SET_SHOMS_TEST(shmem_long_swap, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[95] = SET_SHOMS_TEST(shmem_longlong_swap, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[96] = SET_SHOMS_TEST(shmem_float_swap, init_distributed_float_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[97] = SET_SHOMS_TEST(shmem_double_swap, init_distributed_double_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[98] = SET_SHOMS_TEST(shmem_int_cswap, init_distributed_int_buffer_zeroed, free_distributed_buffer, NULL, NULL, local);
  global_test_index[99] = SET_SHOMS_TEST(shmem_long_cswap, init_distributed_long_buffer_zeroed, free_distributed_buffer, NULL, NULL, local);
  global_test_index[100] = SET_SHOMS_TEST(shmem_longlong_cswap, init_distributed_longlong_buffer_zeroed, free_distributed_buffer, NULL, NULL, local);
  global_test_index[101] = SET_SHOMS_TEST(shmem_int_fadd, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[102] = SET_SHOMS_TEST(shmem_long_fadd, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[103] = SET_SHOMS_TEST(shmem_longlong_fadd, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);  
  global_test_index[104] = SET_SHOMS_TEST(shmem_int_finc, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[105] = SET_SHOMS_TEST(shmem_long_finc, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[106] = SET_SHOMS_TEST(shmem_longlong_finc, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[107] = SET_SHOMS_TEST(shmem_int_add, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[108] = SET_SHOMS_TEST(shmem_long_add, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[109] = SET_SHOMS_TEST(shmem_longlong_add, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[110] = SET_SHOMS_TEST(shmem_int_inc, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[111] = SET_SHOMS_TEST(shmem_long_inc, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[112] = SET_SHOMS_TEST(shmem_longlong_inc, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[113] = SET_SHOMS_TEST(shmem_wait, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[114] = SET_SHOMS_TEST(shmem_short_wait, init_distributed_short_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[115] = SET_SHOMS_TEST(shmem_int_wait, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[116] = SET_SHOMS_TEST(shmem_long_wait, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[117] = SET_SHOMS_TEST(shmem_longlong_wait, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[118] = SET_SHOMS_TEST(shmem_wait_until, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[119] = SET_SHOMS_TEST(shmem_short_wait_until, init_distributed_short_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[120] = SET_SHOMS_TEST(shmem_int_wait_until, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[121] = SET_SHOMS_TEST(shmem_long_wait_until, init_distributed_long_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[122] = SET_SHOMS_TEST(shmem_longlong_wait_until, init_distributed_longlong_buffer, free_distributed_buffer, NULL, NULL, global_no_bw);
  global_test_index[123] = SET_SHOMS_TEST(shmem_clear_lock, init_distributed_long_buffer, free_distributed_buffer, init_per_iteration_shmem_clear_lock, NULL, global_no_bw);
  global_test_index[124] = SET_SHOMS_TEST(shmem_set_lock, init_distributed_long_buffer, free_distributed_buffer, NULL, cleanup_per_iteration_shmem_set_lock, global_no_bw);
  global_test_index[125] = SET_SHOMS_TEST(shmem_test_lock, init_distributed_long_buffer, free_distributed_buffer, NULL, cleanup_per_iteration_shmem_set_lock, global_no_bw);
#ifdef USE_SHMEM12
  global_test_index[126] = SET_SHOMS_TEST(shmem_init, NULL, NULL, init_per_iteration_shmem_init, NULL, global_no_bw);
  global_test_index[127] = SET_SHOMS_TEST(shmem_finalize, NULL, NULL, NULL, cleanup_per_iteration_shmem_finalize, global_no_bw);
#endif
}

index_t all_tests(test_t **tests_array){
  *tests_array = global_test_index;
  return NUM_TESTS;
}

#define NUM_AFF_TESTS 17
index_t aff_test_list[] = {0,1,5,12,22,32,39,49,92,98,101,104,107,110,123,124,125};

index_t affinity_tests(test_t **tests_array){
  /*
  global_test_index[0]  = SET_SHOMS_TEST(shmem_local_read, init_distributed_32bit_bufffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[1]  = SET_SHOMS_TEST(shmem_local_write, init_distributed_32bit_bufffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[5]  = SET_SHOMS_TEST(shmem_int_p, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[12] = SET_SHOMS_TEST(shmem_int_put, init_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[22] = SET_SHOMS_TEST(shmem_int_iput, init_strided_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[32] = SET_SHOMS_TEST(shmem_int_g, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[39] = SET_SHOMS_TEST(shmem_int_get, init_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[49] = SET_SHOMS_TEST(shmem_int_iget, init_strided_sym_and_local_int, free_sym_and_local_t, NULL, NULL, local);
  global_test_index[92] = SET_SHOMS_TEST(shmem_int_swap, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[98] = SET_SHOMS_TEST(shmem_int_cswap, init_distributed_int_buffer_zeroed, free_distributed_buffer, NULL, NULL, local);
  global_test_index[101] = SET_SHOMS_TEST(shmem_int_fadd, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[104] = SET_SHOMS_TEST(shmem_int_finc, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[107] = SET_SHOMS_TEST(shmem_int_add, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[110] = SET_SHOMS_TEST(shmem_int_inc, init_distributed_int_buffer, free_distributed_buffer, NULL, NULL, local);
  global_test_index[123] = SET_SHOMS_TEST(shmem_clear_lock, init_distributed_long_buffer, free_distributed_buffer, init_per_iteration_shmem_clear_lock, NULL, global_no_bw);
  global_test_index[124] = SET_SHOMS_TEST(shmem_set_lock, init_distributed_long_buffer, free_distributed_buffer, NULL, cleanup_per_iteration_shmem_set_lock, global_no_bw);
  global_test_index[125] = SET_SHOMS_TEST(shmem_test_lock, init_distributed_long_buffer, free_distributed_buffer, NULL, cleanup_per_iteration_shmem_set_lock, global_no_bw);
   */
  (*tests_array) = (test_t *)malloc(sizeof(test_t)*NUM_AFF_TESTS);
  for(int idx=0; idx < NUM_AFF_TESTS; idx++){
    memcpy(&((*tests_array)[idx]), &(global_test_index[aff_test_list[idx]]), sizeof(test_t));
  }
  return NUM_AFF_TESTS;
}

void free_test_t(test_t *doomed, index_t size) {
  for(index_t idx=0; idx < size; idx++){
    free(doomed[idx].test_results);
  }
  free(doomed);
}

#define KB ((index_t)1024)
#define MB ((index_t)(KB*(index_t)1024))
#define GB ((index_t)(MB*(index_t)1024))
#define TB ((index_t)(GB*(index_t)1024))

index_t decide_iterations(index_t current_size){
  index_t iterations, scale;
  index_t nodes = shmem_n_pes();

  if(current_size <= 16 * KB) {
    iterations = 1000;
  } else if(current_size <= 512 * KB) {
    iterations = 500;
  } else if(current_size <= 4 * MB) {
    iterations = 250;
  } else if(current_size <= 16 * MB) {
    iterations = 100;
  } else {
    iterations = 50;
  }

  if(nodes < 16) {
    scale = 1;
  } else if(nodes < 64) {
    scale = 2;
  } else if(nodes < 256) {
    scale = 3;
  } else if(nodes < 1024) {
    scale = 4;
  } else {
    scale = 5;
  }

  iterations /= scale;
  if(iterations < 1) {
    iterations = 1;
  }

  return iterations;
}

index_t iteration_decent(index_t count, iteration_data_t **iteration_data, index_t current_data, index_t max_size){
  if(current_data > max_size){
    (*iteration_data) = malloc(sizeof(iteration_data_t)*(count+1));
    return count;
  }

  index_t depth = iteration_decent(count + 1, iteration_data, current_data * 2, max_size);
  (*iteration_data)[count].data_size = current_data;
  (*iteration_data)[count].iterations_count = decide_iterations(current_data);
  return depth;
}

index_t init_iterations(iteration_data_t **iteration_data, index_t data_min, index_t data_max){
  index_t iterations_count = iteration_decent(0, iteration_data, data_min, data_max);
  assert((*iteration_data)[0].data_size == data_min);
  assert((*iteration_data)[iterations_count-1].data_size <= data_max);

  return iterations_count;
}

void free_iteration_data_t(iteration_data_t *doomed, index_t size){
  free(doomed);
}
