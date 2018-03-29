/* -*- mode: C; mode: folding; fill-column: 70; -*- */
/* Copyright 2010,  Georgia Institute of Technology, USA. */
/* Copyright (C) 2014  Oak Ridge National Laboratory.     */
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


static int64_t nvtx_scale;

static struct packed_edge *restrict IJ;
static int64_t nedge;

static int64_t bfs_root[NBFS_max];

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

int main(int argc, char **argv){
    int *restrict has_adj;
    int fd;
    FILE *ofile;
    int64_t desired_nedge;
    int64_t i;
    ORB_t t1, t2, t3;

#ifdef USE_OPENSHMEM
    start_pes(0);
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
        fprintf (stderr, "Generating edge list...");
    }
    if(use_RMAT){
        nedge = desired_nedge;
        IJ = xmalloc_large_ext (nedge * sizeof (*IJ));
        rmat_edgelist (IJ, nedge, SCALE, A, B, C);
    }
    else {
        make_graph(SCALE, desired_nedge, userseed, userseed, &nedge, (struct packed_edge **)(&IJ));
    }
    ORB_read(t2);

    printf("Generation time: %f\n", ORB_seconds(t2, t1));

    if(VERBOSE){
        fprintf (stderr, " done.\n");
    }

    if(dumpname){
        ofile = fopen(dumpname, "w");
    }
    else {
        ofile = NULL;
    }

    if(ofile == NULL){
        fprintf (stderr, "Cannot open output file : %s\n",
                 (dumpname ? dumpname : "stdout"));
        return EXIT_FAILURE;
    }
    fprintf(ofile, "%ld %ld\n", (long) nvtx_scale, nedge);
    for(i = 0; i < nedge; i++){
        fprintf(ofile, "%ld %ld\n", IJ[i].v0, IJ[i].v1);
    }
    ORB_read(t3);
    printf("Output time: %f\n", ORB_seconds(t3, t2));
    printf("Total time: %f\n", ORB_seconds(t3, t1));
    printf("Output edges: %ld\n", nedge);
    printf("Output vertices: %ld\n", nvtx_scale);
    printf("Output format: edge\n");
    printf("Output file: %s\n", dumpname);

    fclose(ofile);

    return EXIT_SUCCESS;
} /* main */

