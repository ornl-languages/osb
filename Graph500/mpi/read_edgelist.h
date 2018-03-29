#ifndef READ_EDGELIST_H
#define READ_EDGELIST_H

#ifdef USE_OPENSHMEM
#include <shmem.h>
#else
#include <mpi.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "graph_generator.h"
extern void long_bcast_all( long *ldest, long *lsrc );
extern int read_edgelist( const char *filename, 
                   long *p_gnvertex, 
                   long *p_gnedge, 
                   long *p_nedge,  
                   struct packed_edge ** p_IJ, int use_binary);

#endif
