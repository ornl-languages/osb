/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*- */
#include<time.h>
#include<sys/time.h>

#include <sys/mman.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define FSTR64 "%ld"
#define FSTRU64 "%lu"
#define ZERO64B 0L
#define SHMEM_BCAST_SYNC_SIZE 64
#define SHMEM_REDUCE_SYNC_SIZE 128

// Type definitions
typedef int64_t s64;
typedef uint64_t u64;

/* Random number generator */
#ifdef LONG_IS_64BITS
#define POLY 0x0000000000000007UL
#define PERIOD 1317624576693539401L
#else
#define POLY 0x0000000000000007ULL
#define PERIOD 1317624576693539401LL
#endif

/* For timing */

static double RTSEC()
{
 struct timeval tp;
 gettimeofday (&tp, NULL);
 return tp.tv_sec + tp.tv_usec/(double)1.0e6;
}

extern s64 starts (u64);

#define WANT_MPI2_TEST 0


#define HPCC_TRUE 1
#define HPCC_FALSE 0
#define HPCC_DONE 0

#define FINISHED_TAG 1
#define UPDATE_TAG   2
#define USE_NONBLOCKING_SEND 1

#define MAX_TOTAL_PENDING_UPDATES 1024
#define LOCAL_BUFFER_SIZE MAX_TOTAL_PENDING_UPDATES

#define USE_MULTIPLE_RECV 1

#ifdef USE_MULTIPLE_RECV
#define MAX_RECV					16
#else
#define MAX_RECV					1
#endif

extern u64 *HPCC_Table;

extern u64 LocalSendBuffer[LOCAL_BUFFER_SIZE];
extern u64 LocalRecvBuffer[MAX_RECV*LOCAL_BUFFER_SIZE];

extern void
Power2NodesRandomAccessUpdate(u64 logTableSize,
                                 u64 TableSize,
                                 u64 LocalTableSize,
                                 u64 MinLocalTableSize,
                                 u64 GlobalStartMyProc,
                                 u64 Top,
                                 int logNumProcs,
                                 int NumProcs,
                                 int Remainder,
                                 int MyProc,
                                 s64 ProcNumUpdates);

