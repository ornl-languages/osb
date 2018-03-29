/* -*- mode: C; mode: folding; fill-column: 70; -*- */
/* Copyright 2010,  Georgia Institute of Technology, USA. */
/* Copyright (c) 2017-2018  UT-Battelle, LLC. All rights reserved.         */
/* See COPYING for license. */

#define _BSD_SOURCE

#include "compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include <assert.h>

#include <alloca.h> /* Portable enough... */
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#if !defined(__MTA__)
  #include <getopt.h>
#endif

#include "graph500.h"
#include "rmat.h"
#include "kronecker.h"
#include "verify.h"
#include "prng.h"
#include "xalloc.h"
#include "options.h"
#include "generator/splittable_mrg.h"
#include "generator/make_graph.h"

#ifdef USE_OPENSHMEM
#include <shmem.h>
#include "shmem_lib.h"
#else
#include <mpi.h>
#endif

#ifdef USE_ORB
#include "orbconfig.h"
#include "orbtimer.h"
#else
#include <time.h>
/*
 * ----------------------------
 * equivalent capability of ORB
 * ----------------------------
 */

typedef float ORB_t;
#define ORB_calibrate() { clock();}
#define ORB_read(t1) { t1 = (float) ( ((double) clock())/ ( (double) CLOCKS_PER_SEC) ); }
#define ORB_seconds(t2,t1) ( (float) ( ((double)  (t2)) - ((double) (t1)) ) )

#endif

#include "read_edgelist.h"


static const int idebug  = 0;

static int64_t nvtx_scale;

static struct packed_edge *IJ;
static int64_t nedge;
static int64_t gnedge;

#ifndef USE_OPENSHMEM
static int64_t bfs_root[NBFS_max];
#endif

/**********************************************************/
/* Subtract the `struct timeval' values X and Y storing   */
/* result in RESULT. Return 1 if the diff is neg, else 0  */
/**********************************************************/
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y){
    /* Perform the carry for the later subtraction by updating y. */
    if(x->tv_usec < y->tv_usec){
        int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
        y->tv_usec -= 1000000 * nsec;
        y->tv_sec += nsec;
    }
    if(x->tv_usec - y->tv_usec > 1000000){
        int nsec = (y->tv_usec - x->tv_usec) / 1000000;
        y->tv_usec += 1000000 * nsec;
        y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
} /* timeval_subtract */



  
/*
 * -------------------------------------------------------------
 * launch job as 
 * aprun -n 4  ./read-edgelist-shmem -o new.file.s14 -r file.s14
 * -------------------------------------------------------------
 */


int main(int argc, char **argv){

#ifndef USE_OPENSHMEM
    int *restrict has_adj;
    int fd;
#endif

    FILE *ofile;
    int64_t desired_nedge;
    int64_t i;
    ORB_t t1, t2, t3;
    int my_pe = 0;
    int n_pes = 1;

#ifdef USE_OPENSHMEM
    start_pes(0);
    my_pe = shmem_my_pe();
    n_pes = shmem_n_pes();
#else
    MPI_Init(&argc,&argv);
    MPI_Comm_rank( MPI_COMM_WORLD, &my_pe );
    MPI_Comm_size( MPI_COMM_WORLD, &n_pes );
#endif

    if(sizeof (int64_t) < 8){
        fprintf (stderr, "No 64-bit support.\n");
        return EXIT_FAILURE;
    }

    if(argc > 1){
        get_options (argc, argv);
    }

    nvtx_scale = 1L << SCALE;

    ORB_calibrate();
    init_random ();

    desired_nedge = nvtx_scale * edgefactor;
    /* Catch a few possible overflows. */
    assert (desired_nedge >= nvtx_scale);
    assert (desired_nedge >= edgefactor);

    ORB_read(t1);
    if(VERBOSE){
        fprintf (stderr, "Reading edge list...");
    }


#if (0)
    if(use_RMAT){
        nedge = desired_nedge;
        IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
        rmat_edgelist (IJ, nedge, SCALE, A, B, C);
    }
    else {
        make_graph(SCALE, desired_nedge, userseed, userseed, &nedge, (struct packed_edge **)(&IJ));
    }
#else

    {
    long gnvertex = 0;
    int istatus = 0;
    const char *filename = (rootname == NULL) ? "file.s14" : rootname;

    char *p = strstr( filename, ".bin");
    int use_binary =  (p != NULL) && (strlen(p) == strlen(".bin"));

    istatus = read_edgelist(filename, &gnvertex, &gnedge, &nedge, &IJ, use_binary );
    nvtx_scale = gnvertex;
    assert( istatus == 0);
    if ((idebug >= 1) && (my_pe == 0)) {
       printf("from %s: gnvertex %ld gnedge %ld nedge %ld\n", 
               filename, gnvertex,gnedge,nedge );
       };
    }


#endif
    ORB_read(t2);

    printf("Reading time: %f\n", ORB_seconds(t2, t1));

    if(VERBOSE){
        fprintf (stderr, " done.\n");
    }

#ifdef USE_OPENSHMEM
    {
     gnedge =  shmem_long_sum_all( (long) nedge );
    }
#else
    MPI_Allreduce( &nedge, &gnedge, 1, MPI_INT64_T, MPI_SUM, MPI_COMM_WORLD );
#endif
    

    { int pe = 0;
      for(pe=0; pe < n_pes; pe++) {
#ifdef USE_OPENSHMEM
       shmem_barrier_all();  
#else
       MPI_Barrier( MPI_COMM_WORLD );
#endif
      if (pe != my_pe) { continue; };

      if(dumpname){
        if (pe == 0) {
           ofile = fopen(dumpname, "w");
           }
        else {
           ofile = fopen(dumpname, "a");
           };
      }
      else {
        ofile = NULL;
      }

      if(ofile == NULL){
        fprintf (stderr, "Cannot open output file : %s\n",
                 (dumpname ? dumpname : "stdout"));
        return EXIT_FAILURE;
      }
      if (pe == 0) {
         // fprintf(ofile, "%ld %ld\n", (long) nvtx_scale, nedge);
         fprintf(ofile, "%ld %ld\n", (long) nvtx_scale, (long) gnedge);
         };
      for(i = 0; i < nedge; i++){
        fprintf(ofile, "%ld %ld\n", IJ[i].v0, IJ[i].v1);
      }
      fclose(ofile);

      };
    };
    ORB_read(t3);

    if (my_pe == 0) {
    printf("Output time: %f\n", ORB_seconds(t3, t2));
    printf("Total time: %f\n", ORB_seconds(t3, t1));
    printf("Output edges: %ld\n", nedge);
    printf("Output vertices: %ld\n", nvtx_scale);
    printf("Output format: edge\n");
    printf("Output file: %s\n", dumpname);
    };


    return EXIT_SUCCESS;
} /* main */

