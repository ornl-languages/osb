/* Copyright (c) 2017-2018  UT-Battelle, LLC. All rights reserved.         */
/* Authors: Ed D'Azevedo                                                   */

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "shmem_lib.h"


#ifndef ABS
#define ABS(x)  (  ((x) > 0) ? (x) : (-(x)) )
#endif

#ifndef MIN
#define MIN(x,y)  (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y)  (((x) > (y)) ? (x) : (y))
#endif

#ifndef ULONG_BOR
#define ULONG_BOR(x,y)  ( ( (unsigned long) (x)) | ( (unsigned long) (y)) )
#endif

#ifndef UINT_BOR
#define UINT_BOR(x,y)  ( ( (unsigned int) (x)) | ( (unsigned int) (y)) )
#endif

#ifndef ULONG_BAND
#define ULONG_BAND(x,y)  ( ((unsigned long) (x)) & ((unsigned long) (y)) )
#endif

#ifndef UINT_BAND
#define UINT_BAND(x,y)  ( ((unsigned int) (x)) & ((unsigned int) (y)) )
#endif

#ifndef FALSE
#define FALSE (0 == 1)
#endif

#ifndef TRUE
#define TRUE (1 == 1)
#endif

/*
 * -----------------------------------------
 * return elapse time, similar to MPI_Wtime()
 * -----------------------------------------
 */
double
shmem_wtime()
{
#ifdef USE_CLOCK
  /*
   *  The clock() function returns an approximation of processor time used by the program.
   *  The  value  returned  is  the CPU time used so far as a clock_t
   */
clock_t uptime = clock();
return( ((double) uptime) / ( (double) CLOCKS_PER_SEC) );
#else
#ifdef USE_MPI_WTIME
   extern double MPI_Wtime(void);

   return (MPI_Wtime());

#else
#include <sys/time.h>

    /* Fall back to gettimeofday() if we have nothing else */
    double wtime = 0;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    wtime = tv.tv_sec;
    wtime += (double)tv.tv_usec / 1000000.0;
    return(wtime);
#endif

#endif
}


void
shmem_long_min (long *gvar, long value, int pe)
{
    long cval = 0;
    long lval = 0;

    assert( (0 <= pe) && (pe < shmem_n_pes()) );
    assert( shmem_pe_accessible(pe) );
    assert( shmem_addr_accessible(gvar,pe) );

    shmem_long_get(&lval, gvar, 1, pe);
    if (value < lval) {
        for (;;) {
            cval = shmem_long_cswap(gvar, lval, value, pe);
            if (cval == lval) {
                break;
            } else {
                if (value < cval) {
                    lval = cval;
                } else {
                    break;
                }
            }
        }
    }
}

#ifdef WITH_CTX
void
shmem_ctx_long_min (long *gvar, long value, int pe, shmem_ctx_h * ctx)
{
    long cval = 0;
    long lval = 0;
    int is_done = 0;

    assert( (0 <= pe) && (pe < shmem_n_pes()) );
    assert( shmem_pe_accessible(pe) );
    assert( shmem_addr_accessible(gvar,pe) );

    shmem_ctx_long_get(*ctx, &lval, gvar, 1, pe);
    if (value < lval) {
        do
        {
	        cval = shmem_ctx_long_cswap (*ctx, gvar, lval, MIN (lval, value), pe);
	        is_done = (cval == lval) || (cval <= value);
	        lval = cval;
        } while (!is_done);
    }
}
#endif /* WITH_CTX */


void
shmem_long_max (long *gvar, long value, int pe)
{
  long cval = 0;
  long lval = 0;
  int is_done = 0;

  assert( (0 <= pe) && (pe < shmem_n_pes()) );

    
  lval = shmem_long_fadd(gvar, (long) 0, pe );
  if (value > lval) {
    do
    {
	  cval = shmem_long_cswap (gvar, lval, MAX (lval, value), pe);
	  is_done = (cval == lval) || (cval >= value);
	  lval = cval;
    }
    while (!is_done);
   };
}


void shmem_ulong_bor(unsigned long *gvar, unsigned long value, int pe)
{
    /*
     * perform Bitwise OR
     */
    long cval = 0;
    long lval = 0;
    unsigned long ulval = 0;

    assert( (0 <= pe) && (pe < shmem_n_pes()) );
    assert( shmem_pe_accessible(pe) );
    assert( shmem_addr_accessible(gvar,pe) );

    shmem_long_get(&lval, gvar, 1, pe);
    ulval = lval | value;
    if (ulval == lval) {
        return;
    }
    for (;;) {
        cval = shmem_long_cswap((long *)gvar, lval, ulval, pe);
        if (cval == lval) {
            break;
        } else {
            lval = cval;
            ulval = lval | value;
        }
    }
}

#ifdef WITH_CTX
void shmem_ctx_ulong_bor(unsigned long *gvar, unsigned long value, int pe, shmem_ctx_h * ctx)
{
    /*
     * perform Bitwise OR
     */
    long cval = 0;
    long lval = 0;
    unsigned long ulval = 0;
    unsigned long new_ulval = 0;
    long new_lval = 0;
    int is_done = 0;

    assert( (0 <= pe) && (pe < shmem_n_pes()) );
    assert( shmem_pe_accessible(pe) );
    assert( shmem_addr_accessible(gvar,pe) );

    shmem_ctx_long_get(*ctx, &lval, gvar, 1, pe);
    ulval = lval | value;
    if (ulval == lval) {
        return;
    }
    for (;;) {
        cval = shmem_ctx_long_cswap(*ctx, (long *)gvar, lval, ulval, pe);
        if (cval == lval) {
            break;
        } else {
            lval = cval;
            ulval = lval | value;
        }
    }
}
#endif /* WITH_CTX */

void
shmem_int_min (int *gvar, int value, int pe)
{
  int cval = 0;
  int lval = 0;
  int is_done = 0;

  assert( (0 <= pe) && (pe < shmem_n_pes()) );

  lval = shmem_int_fadd( gvar, (int) 0, pe );
  if (value < lval) {
    do
    {
      is_done = (lval <= value);
      if (!is_done)
	{
	  cval = shmem_int_cswap (gvar, lval, MIN (lval, value), pe);
	  is_done = (cval == lval) || (cval <= value);
	  lval = cval;
	}
    }
    while (!is_done);
   };
}


void
shmem_int_max (int *gvar, int value, int pe)
{
  int cval = 0;
  int lval = 0;
  int is_done = 0;

  assert( (0 <= pe) && (pe < shmem_n_pes()) );

  lval = shmem_int_fadd( gvar, (int) 0, pe );
  if (value > lval) {
    do
    {
      is_done = (lval >= value);
      if (!is_done)
	{
	  cval = shmem_int_cswap (gvar, lval, MAX (lval, value), pe);
	  is_done = (cval == lval) || (cval >= value);
	  lval = cval;
	}
    }
    while (!is_done);
   };
}



void
shmem_uint_bor (int *gvar, unsigned int value, int pe)
{
  /*
   * perform Bitwise OR
   */
  int cval = 0;
  unsigned int ulval = 0;
  int new_lval = 0;
  int lval = 0;
  int is_done = 0;

  assert( (0 <= pe) && (pe < shmem_n_pes()) );

  lval = shmem_int_fadd( gvar, (int) 0, pe );
  do
    {
      ulval = UINT_BOR(lval, value );
      memcpy( &new_lval, &ulval, sizeof(lval) );
      cval = shmem_int_cswap (gvar, lval, new_lval, pe);
      is_done = (cval == lval);
      lval = cval;

    }
  while (!is_done);
}


/*
 * ----------------------------------------
 * find the sum value across all processors
 * thin interface to shmem_long_sum_to_all()
 * ----------------------------------------
 */
long shmem_long_sum_all( long lvalue_in )
{
  static long gvalue = 0;
  static long lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);


  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };

  lvalue = lvalue_in;
  gvalue = lvalue_in;

  shmem_barrier_all();
  shmem_long_sum_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();

#ifdef USE_DEBUG
  fprintf(stderr,"shmem_long_sum_all: rank %d before %ld final %ld\n",
         shmem_my_pe(),  lvalue_in,  gvalue );
  fflush(stderr);
#endif

  return( gvalue );

}


/*
 * ----------------------------------------
 * find the max value across all processors
 * thin interface to shmem_long_max_to_all()
 * ----------------------------------------
 */
long shmem_long_max_all( long lvalue_in )
{
  static long gvalue = 0;
  static long lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);

  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };

  lvalue = lvalue_in;
  gvalue = lvalue_in;

  shmem_barrier_all();
  shmem_long_max_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();
#ifdef USE_DEBUG2
    fprintf(stderr,"shmem_long_max_all: rank %d before %ld after %ld\n",
                 shmem_my_pe(), lvalue_in, gvalue );
    fflush(stderr);
#endif

  assert( gvalue >= lvalue );
  return( gvalue );

}


/*
 * ----------------------------------------
 * find the max value across all processors
 * thin interface to shmem_longdouble_max_to_all()
 * ----------------------------------------
 */
long double shmem_longdouble_max_all( long double lvalue_in )
{
  static long double gvalue = 0;
  static long double lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long double pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long double);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);

  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  (long double) 0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };

  lvalue = lvalue_in;
  gvalue = lvalue_in;

  shmem_barrier_all();
  shmem_longdouble_max_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();

  assert( gvalue >= lvalue );
  return( gvalue );

}


/*
 * ----------------------------------------
 * find the min value across all processors
 * thin interface to shmem_long_min_to_all()
 * ----------------------------------------
 */
long shmem_long_min_all( long lvalue_in )
{
  static long gvalue = 0;
  static long lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);

  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };

  lvalue = lvalue_in;

  shmem_barrier_all();
  shmem_long_min_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();

  assert( gvalue <= lvalue );
  return( gvalue );

}



/*
 * ----------------------------------------
 * find the bitwise or value across all processors
 * thin interface to shmem_long_or_to_all()
 * ----------------------------------------
 */
long shmem_long_or_all( long lvalue_in )
{
  static long gvalue = 0;
  static long lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);

  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };


  lvalue = lvalue_in;
  shmem_barrier_all();
  shmem_long_or_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();

  return( gvalue );

}


/*
 * ----------------------------------------
 * find the bitwise and value across all processors
 * thin interface to shmem_long_and_to_all()
 * ----------------------------------------
 */
long shmem_long_and_all( long lvalue_in )
{
  static long gvalue = 0;
  static long lvalue = 0;

  int i = 0;
  int PE_start = 0;
  int logPE_stride = 0;
  int PE_size = shmem_n_pes();
  const int nreduce = 1;
  static long pWrk[2 + 1 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE];
  const int len_pWrk = sizeof(pWrk)/sizeof(long);

  static long pSync[_SHMEM_REDUCE_SYNC_SIZE];
  const int len_pSync = sizeof(pSync)/sizeof(long);

  for(i=0; i < len_pWrk; i++) {
     pWrk[i] =  0;
     };

  for(i=0; i < len_pSync; i++) {
     pSync[i] = _SHMEM_SYNC_VALUE;
    };
#ifndef NDEBUG
    shmem_barrier_all();
#endif

  lvalue = lvalue_in;

  shmem_barrier_all();
  shmem_long_and_to_all( &gvalue, &lvalue, nreduce, 
                          PE_start, logPE_stride, PE_size, 
                          pWrk, pSync );
  shmem_barrier_all();

  return( gvalue );

}

int  shmem_addr_accessible_all( void *ptr )
{
  int pe = 0;
  int npes = shmem_n_pes();

  int is_accessible_all;

  is_accessible_all = TRUE;
  for(pe=0; (pe < npes) ; pe++) {
    is_accessible_all = is_accessible_all &&
           shmem_pe_accessible(pe) && 
           shmem_addr_accessible(ptr,pe);
    };

  return(is_accessible_all);
  
} 




/*
 * -------------------------------------------
 * perform bitwise OR reduction across all processors
 * thin interface to call shmem_int_or_to_all()
 * -------------------------------------------
 */
  
int shmem_int_or_all( int lvalue )
{
    static int out_global_var = 0;
    static int in_global_var = 0;

    static long pSync[ _SHMEM_REDUCE_SYNC_SIZE];
    static int pWrk[ 4 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE ];
    const int len_pWrk = sizeof(pWrk)/sizeof(int);
    const int len_pSync = sizeof(pSync)/sizeof(long);

    int any_set = lvalue;

    int *target = &out_global_var;
    int *source = &in_global_var;
    int pe_start = 0;
    int nreduce = 1;
    int logpe_stride = 0;
    int pe_size = shmem_n_pes();


    int i = 0;

    out_global_var = any_set;
    in_global_var = any_set;

    for(i=0; i < len_pSync; i++) {
       pSync[i] = _SHMEM_SYNC_VALUE;
       };

    for(i=0; i < len_pWrk;i++) {
       pWrk[i] = 0;
       };

    shmem_barrier_all();
    shmem_int_or_to_all(  target,  source, nreduce,
                  pe_start, logpe_stride, pe_size,
                  pWrk, pSync );
    shmem_barrier_all();

#ifdef USE_DEBUG2
    fprintf(stderr,"shmem_int_or_all: rank %d before %d after %d\n",
                 shmem_my_pe(), lvalue, (*target) );
    fflush(stderr);
#endif

    return(  *target );
}

/*
 * ----------------------
 * perform the logical OR 
 * across all processors
 * ----------------------
 */
int shmem_int_lor_all( int lvalue )
{
  int gvalue = 0;
  
  shmem_barrier_all();
  gvalue = (shmem_int_or_all( (lvalue != 0) ) != 0);
  shmem_barrier_all();

#ifdef USE_DEBUG2
  fprintf(stderr,"shmem_int_lor_all: rank %d before %d after %d\n",  
       shmem_my_pe(), lvalue, gvalue );
  fflush( stderr );
#endif
  
  return(gvalue);
}



/*
 * -------------------------------------------
 * perform bitwise AND reduction across all processors
 * thin interface to call shmem_int_and_to_all()
 * -------------------------------------------
 */
  
int shmem_int_and_all( int lvalue )
{
    static int out_global_var = 0;
    static int in_global_var = 0;

    static long pSync[ _SHMEM_REDUCE_SYNC_SIZE ];
    static int pWrk[ 4 + _SHMEM_REDUCE_MIN_WRKDATA_SIZE ];
    const int len_pWrk = sizeof(pWrk)/sizeof(int);
    const int len_pSync = sizeof(pSync)/sizeof(long);

    int any_set = lvalue;

    int *target = &out_global_var;
    int *source = &in_global_var;
    int pe_start = 0;
    int nreduce = 1;
    int logpe_stride = 0;
    int pe_size = shmem_n_pes();


    int i = 0;

    out_global_var = any_set;
    in_global_var = any_set;

    for(i=0; i < len_pSync; i++) {
       pSync[i] = _SHMEM_SYNC_VALUE;
       };

    for(i=0; i < len_pWrk;i++) {
       pWrk[i] = 0;
       };

    shmem_barrier_all();
    shmem_int_and_to_all(  target,  source, nreduce,
                  pe_start, logpe_stride, pe_size,
                  pWrk, pSync );
    shmem_barrier_all();

    return(  *target );
}

/*
 * -------------------------------------------
 * perform logical AND reduction across all processors
 * thin interface to call shmem_int_and_to_all()
 * -------------------------------------------
 */
int shmem_int_land_all( int lvalue )
{
  int gvalue = ( (shmem_int_and_all(  (lvalue != 0) ) != 0) );
#ifdef USE_DEBUG2
  fprintf(stderr,"shmem_int_land_all: rank %d before %d final %d\n",
              shmem_my_pe(), lvalue, gvalue );
  fflush(stderr);
#endif

  return(gvalue);
}

/*
 * ----------------------------------------------------------------------
 * each processor send data to all other processors (including self)
 * each processor receive data from all other processors (including self)
 * ----------------------------------------------------------------------
 */
void shmem_int_alltoall( int *sendbuf, int *recvbuf )
{
  int *source = 0;
  int *target = 0;
  unsigned int n_pes = (unsigned int) shmem_n_pes();
  unsigned int my_pe = (unsigned int) shmem_my_pe();
  unsigned int pe = 0;

  size_t nbytes = sizeof(int);
  nbytes *=  n_pes;
  source = (int *) shmalloc( nbytes );
  assert( source != NULL );

  nbytes = sizeof(int);
  nbytes *= n_pes;
  target = (int *) shmalloc( nbytes );
  assert( target != NULL );

  for(pe=0; pe < n_pes; pe++) {
    source[pe] = sendbuf[pe];
    };

  shmem_barrier_all();
  
  for(pe=0; pe < n_pes; pe++) {
    size_t len = 1;
    shmem_int_put( &(target[my_pe]), &(source[pe]), len, (int) pe );
    };

  shmem_barrier_all();

  for(pe=0; pe < n_pes; pe++) {
    recvbuf[pe] = target[pe];
    };

  shmem_barrier_all();
  
  shfree( source );
  shfree( target );

}


/*
 * ----------------------------------------------------------------------
 * each processor send data to all other processors (including self)
 * each processor receive data from all other processors (including self)
 * ----------------------------------------------------------------------
 */
void shmem_long_alltoall( long *sendbuf, long *recvbuf )
{
  long *source = 0;
  long *target = 0;
  unsigned int n_pes = (unsigned int) shmem_n_pes();
  unsigned int my_pe = (unsigned int) shmem_my_pe();
  unsigned int pe = 0;

  source = (long *) shmalloc( sizeof(int) * n_pes );
  assert( source != NULL );

  target = (long *) shmalloc( sizeof(int) * n_pes );
  assert( target != NULL );

  for(pe=0; pe < n_pes; pe++) {
    source[pe] = sendbuf[pe];
    };

  shmem_barrier_all();
  
  for(pe=0; pe < n_pes; pe++) {
    size_t len = 1;
    shmem_long_put( &(target[my_pe]), &(source[pe]), len, (int) pe );
    };

  shmem_barrier_all();

  for(pe=0; pe < n_pes; pe++) {
    recvbuf[pe] = target[pe];
    };

  shmem_barrier_all();
  
  shfree( source );
  shfree( target );

}






/*
 * ----------------------------------------------------
 * perform equivalent of MPI_Alltoallv but all in bytes
 * ----------------------------------------------------
 */
void shmem_mem_alltoallv( void *sendbuf_in, 
                          int *sendcounts, 
                          int *sdispls,
                          void *recvbuf_in, 
                          int *recvcounts, 
                          int *rdispls,
                          size_t size_in_bytes )
{

  char *sendbuf = (char *) sendbuf_in;
  char *recvbuf = (char *) recvbuf_in;
  char *target = NULL;

  unsigned int n_pes = (unsigned int) shmem_n_pes();
  unsigned int my_pe = (unsigned int) shmem_my_pe();

  unsigned long total_recvcounts = 0;
  unsigned long umax_recvcounts = 0;
  long max_recvcounts = 0;
  unsigned int pe = 0;

  const size_t max_memory =  1024 * 1024 * 128;

  assert( sendbuf != NULL );
  assert( recvbuf != NULL );
  assert( sdispls != NULL );
  assert( rdispls != NULL );
  assert( sendcounts != NULL );
  assert( recvcounts != NULL );

  /*
   * -----------------
   * compute local max
   * -----------------
   */
  umax_recvcounts = 0;
  for(pe=0; pe < n_pes; pe++) {
    unsigned long len = (unsigned long) recvcounts[pe];
    umax_recvcounts = MAX( umax_recvcounts, len );
    };

  max_recvcounts = (long) umax_recvcounts;
  assert( max_recvcounts >= 0);

  max_recvcounts = shmem_long_max_all( max_recvcounts );
  umax_recvcounts = (unsigned long) max_recvcounts;

  total_recvcounts = n_pes * umax_recvcounts;


  long nb  = MAX(1,((max_memory/size_in_bytes) / n_pes));
  long ntimes = (max_recvcounts  + (nb-1))/nb;
  long itime = 0;

#ifdef USE_DEBUG
  if (my_pe == 0) {
      printf("nb %ld umax_recvcounts %ld ntimes %ld\n",
            (long) nb, (long) umax_recvcounts, (long) ntimes );
  };
#endif




  target =  (char *) shmalloc( MIN( nb * n_pes * size_in_bytes,
                                    total_recvcounts * size_in_bytes )
                              );
  if (target == NULL) {
     printf("pe %d: total_recvcounts %ld size_in_bytes %ld\n",
           shmem_my_pe(), total_recvcounts, size_in_bytes );
     };
  assert( target != NULL );


  /*
   * -------------------
   * perform remote copy
   * -------------------
   */
  for(itime=1; itime <= ntimes; itime += 1) {

  shmem_barrier_all();

  for(pe=0; pe < n_pes; pe++) {
    int ipe = (int) pe;

    size_t ioff = ( (size_t) sdispls[ipe]) + (itime-1)*nb ;
    
    long jstart = 1 + (itime-1)*nb;
    long jend = MIN(jstart + nb-1, sendcounts[ipe] );
    long jsize = jend - jstart + 1;
    int  has_work = (jsize > 0);

    size_t len = (size_t) jsize;
    size_t istart = my_pe * nb; 



    len *= size_in_bytes;
    ioff *= size_in_bytes;
    istart *= size_in_bytes;

    if (has_work) {
      shmem_putmem( &(target[istart]), 
                    &(sendbuf[ioff]),             len, (int) pe); 
      };
    };

  shmem_barrier_all();

  /*
   * ---------------------------------------
   * copy from local shmem buffer to recvbuf
   * ---------------------------------------
   */
  for(pe=0; pe < n_pes; pe++) {
     int ipe = (int) pe;
     long jstart = 1 + (itime-1)*nb;
     long jend = MIN( jstart+nb-1, recvcounts[ipe]);
     long jsize = jend - jstart + 1;
     int has_work = (jsize > 0);

     size_t len = (size_t) jsize;
     size_t ioff = ((size_t) rdispls[ipe]) + (itime-1)*nb; 
     size_t istart = pe * nb; 


    len *= size_in_bytes;
    ioff *= size_in_bytes;
    istart *= size_in_bytes;
    if (has_work) {
      memcpy( &(recvbuf[ioff]),   &(target[istart]), len );
      };
     };

  }; /* for (itime) */

  shmem_barrier_all();
  shfree( target );
}

unsigned long shmem_ulong_bor_all( unsigned long val_in )
{
 /*
  * -------------------------------------------------
  * compute the bitwise or operation on unsigned long
  * -------------------------------------------------
  */
  unsigned int n_pes = (unsigned int) shmem_n_pes();
  unsigned int my_pe = (unsigned int) shmem_my_pe();

  static unsigned long source;
  static unsigned long target;
  static unsigned long result;

  target = 0;
  source = val_in;
  result = val_in;

  shmem_barrier_all();
  if (my_pe == 0) {
     /*
      * ---------------------------
      * perform computation on pe 0
      * ---------------------------
      */
     unsigned int pe;
     for(pe=1; pe < n_pes; pe++) {
        shmem_getmem( &target, &source, sizeof(unsigned long), (int) pe );
        result |= target;
        };

     /*
      * ----------------
      * put results back
      * ----------------
      */
     for(pe=1; pe < n_pes; pe++) {
         shmem_putmem( &result, &result, sizeof(unsigned long), (int) pe );
         };
     
    };
  shmem_barrier_all();
  return(result);
}


unsigned long shmem_ulong_max_all( unsigned long val_in )
{
 /*
  * -------------------------------------------------
  * compute the MAX operation on unsigned long
  * -------------------------------------------------
  */
 const int use_longdouble = (sizeof(long double) > sizeof(unsigned long));
 unsigned long final_result = 0;

 if (use_longdouble) {
   final_result = ( (unsigned long) shmem_longdouble_max_all( (long double) val_in ) );
   }
 else {



  unsigned int n_pes = (unsigned int) shmem_n_pes();
  unsigned int my_pe = (unsigned int) shmem_my_pe();

  static unsigned long source;
  static unsigned long target;
  static unsigned long result;

  target = 0;
  source = val_in;
  result = val_in;

  shmem_barrier_all();
  if (my_pe == 0) {
     /*
      * ---------------------------
      * perform computation on pe 0
      * ---------------------------
      */
     unsigned int pe;
     for(pe=1; pe < n_pes; pe++) {
        shmem_getmem( &target, &source, sizeof(unsigned long), (int) pe );
        result = MAX(result,target);
        };

     /*
      * ----------------
      * put results back
      * ----------------
      */
     for(pe=1; pe < n_pes; pe++) {
         shmem_putmem( &result, &result, sizeof(unsigned long), (int) pe );
         };
     
    };
  shmem_barrier_all();
  final_result = (result);
  };
#ifdef USE_DEBUG
 printf("shmem_ulong_max_all: rank %d  initial %lu final_result %lu\n",
               shmem_my_pe(), val_in, final_result );
#endif

 return( final_result );
}
        
  
