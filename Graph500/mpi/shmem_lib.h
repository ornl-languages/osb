#ifndef SHMEM_LIB_H
#define SHMEM_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <shmem.h>


double shmem_wtime();

void shmem_long_min (long *gvar, long value, int pe);
void shmem_long_max (long *gvar, long value, int pe);
void shmem_ulong_bor (unsigned long *gvar, unsigned long value, int pe);


void shmem_int_min (int *gvar, int value, int pe);
void shmem_int_max (int *gvar, int value, int pe);
void shmem_uint_bor (int *gvar, unsigned int value, int pe);

unsigned long shmem_ulong_bor_all( unsigned long ulvalue );
unsigned long shmem_ulong_max_all( unsigned long ulvalue );
long double shmem_longdouble_max_all( long double ulvalue );

long shmem_long_sum_all( long lvalue );
long shmem_long_max_all( long lvalue );
long shmem_long_min_all( long lvalue );
long shmem_long_or_all( long lvalue );
long shmem_long_and_all( long lvalue );

int shmem_addr_accessible_all( void *ptr );

int shmem_int_or_all( int lvalue );
int shmem_int_lor_all( int lvalue );
int shmem_int_and_all( int lvalue );
int shmem_int_land_all( int lvalue );

void shmem_int_alltoall( int *sendbuf, int *recvbuf );
void shmem_long_alltoall( long *sendbuf, long *recvbuf );

void shmem_mem_alltoallv( void *sendbuf, 
                          int *sendcounts, 
                          int *sdispls,
                          void *recvbuf, 
                          int *recvcounts, 
                          int *rdispls,
                          size_t size_in_bytes);

 

#endif
