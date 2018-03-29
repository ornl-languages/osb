/* Copyright (C) 2010 The Trustees of Indiana University.                  */
/* Copyright (C) 2014  Oak Ridge National Laboratory.                      */
/*                                                                         */
/* Use, modification and distribution is subject to the Boost Software     */
/* License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at */
/* http://www.boost.org/LICENSE_1_0.txt)                                   */
/*                                                                         */
/*  Authors: Jeremiah Willcock                                             */
/*           Andrew Lumsdaine                                              */
/*           Ed D'Azevedo                                                  */

/* These need to be before any possible inclusions of stdint.h or inttypes.h.
 * */
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include "../generator/make_graph.h"
#include "../generator/utils.h"
#include "common.h"
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>

#ifdef USE_OPENSHMEM
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <shmem.h>
#include "shmem_lib.h"

#define WTIME() shmem_wtime()
#define FILE_Offset off_t

#define FILE_MODE_RDONLY              2  /* ADIO_RDONLY */
#define FILE_MODE_RDWR                8  /* ADIO_RDWR  */
#define FILE_MODE_WRONLY              4  /* ADIO_WRONLY  */
#define FILE_MODE_CREATE              1  /* ADIO_CREATE */ 
#define FILE_MODE_EXCL               64  /* ADIO_EXCL */
#define FILE_MODE_DELETE_ON_CLOSE    16  /* ADIO_DELETE_ON_CLOSE */
#define FILE_MODE_UNIQUE_OPEN        32  /* ADIO_UNIQUE_OPEN */
#define FILE_MODE_APPEND            128  /* ADIO_APPEND */
#define FILE_MODE_SEQUENTIAL        256  /* ADIO_SEQUENTIAL */

#define BARRIER()  {shmem_barrier_all();}

#else
#include <mpi.h>


#define WTIME()  MPI_Wtime()
#define FILE_Offset MPI_Offset


#define FILE_MODE_RDONLY MPI_MODE_RDONLY
#define FILE_MODE_RDWR MPI_MODE_RDWR
#define FILE_MODE_WRONLY MPI_MODE_WRONLY
#define FILE_MODE_CREATE MPI_MODE_CREATE
#define FILE_MODE_EXCL MPI_MODE_EXCL
#define FILE_MODE_DELETE_ON_CLOSE MPI_MODE_DELETE_ON_CLOSE
#define FILE_MODE_UNIQUE_OPEN MPI_MODE_UNIQUE_OPEN
#define FILE_MODE_APPEND MPI_MODE_APPEND
#define FILE_MODE_SEQUENTIAL MPI_MODE_SEQUENTIAL

#define BARRIER()  {MPI_Barrier(MPI_COMM_WORLD);}

#endif

#include "read_edgelist.h"
static int compare_doubles(const void* a, const void* b) {
  double aa = *(const double*)a;
  double bb = *(const double*)b;
  return (aa < bb) ? -1 : (aa == bb) ? 0 : 1;
}

enum {s_minimum, s_firstquartile, s_median, s_thirdquartile, s_maximum, s_mean, s_std, s_LAST};
static void get_statistics(const double x[], int n, double r[s_LAST]) {
  double temp;
  int i;
  /* Compute mean. */
  temp = 0;
  for (i = 0; i < n; ++i) temp += x[i];
  temp /= n;
  r[s_mean] = temp;
  /* Compute std. dev. */
  temp = 0;
  for (i = 0; i < n; ++i) temp += (x[i] - r[s_mean]) * (x[i] - r[s_mean]);
  temp /= n - 1;
  r[s_std] = sqrt(temp);
  /* Sort x. */
  double* xx = (double*)xmalloc( ((size_t) n) * sizeof(double));
  memcpy(xx, x, ( (size_t) n) * sizeof(double));
  qsort(xx, ((size_t) n), sizeof(double), compare_doubles);
  /* Get order statistics. */
  r[s_minimum] = xx[0];
  r[s_firstquartile] = (xx[(n - 1) / 4] + xx[n / 4]) * .5;
  r[s_median] = (xx[(n - 1) / 2] + xx[n / 2]) * .5;
  r[s_thirdquartile] = (xx[n - 1 - (n - 1) / 4] + xx[n - 1 - n / 4]) * .5;
  r[s_maximum] = xx[n - 1];
  /* Clean up. */
  free(xx);
}

int main(int argc, char** argv) {
#ifdef USE_OPENSHMEM
  start_pes(0);

#else
  MPI_Init(&argc, &argv);
#endif

  setup_globals();

  /* Parse arguments. */
  int SCALE = 16;
  int edgefactor = 1; /* nedges / nvertices, i.e., 2*avg. degree */

#ifdef ORIGINAL
     if (argc >= 2) SCALE = atoi(argv[1]);
     if (argc >= 3) edgefactor = atoi(argv[2]);
     if (argc <= 1 || argc >= 4 || SCALE == 0 || edgefactor == 0) {
       if (rank == 0) {
         fprintf(stderr, "Usage: %s SCALE edgefactor\n  SCALE = log_2(# vertices) [integer, required]\n  edgefactor = (# edges) / (# vertices) = .5 * (average vertex degree) [integer, defaults to 16]\n(Random number seed and Kronecker initiator are in main.c)\n", argv[0]);
       }
   #ifdef USE_OPENSHMEM
       exit(1);
   #else
       MPI_Abort(MPI_COMM_WORLD, 1);
   #endif
     }

#else
     char *input_filename = NULL;
     if (argc >= 2) {
         input_filename = strdup( argv[1] );
     } else {
         input_filename = strdup( "input_file" );
     }
     if (rank == 0) {
       fprintf(stderr,"Edge list file  is %s\n", input_filename );
     }


#endif



  uint64_t seed1 = 2, seed2 = 3;

 /*
  * -----------------
  * read in edge list
  * -----------------
  */
 packed_edge* IJ = NULL;
 long gnvertex = 0;
 long gnedge = 0;
 long lnedge = 0;

 BARRIER();
 double read_time_start = WTIME();
 int istat;

 /*
  * ---------------------------------------
  * assume binary file if it ends in ".bin"
  * ---------------------------------------
  */
 char *p = strstr(input_filename,".bin"); 
 int use_binary = (p != NULL) && (strlen(p) == strlen(".bin"));
 const int idebug = 0;


 if ((idebug >= 1) && (rank == 0)) {
     int strlen_p = (p != NULL) ? strlen(p) : 0;
     int strlen_filename = (input_filename != NULL) ? strlen(input_filename) : 0;

     printf("input_filename %s strlen(input_filename)  %d strlen(p) %d use_binary %d\n",
             input_filename, strlen_filename, strlen_p,  use_binary );
 };
     
 istat = read_edgelist( input_filename, 
                      &gnvertex, &gnedge, &lnedge, &IJ, use_binary );
 assert( istat == 0);
 if ((idebug >= 1) && (rank == 0)) {
   printf("gnvertex %ld gnedge %ld lnedge %ld  (IJ == 0) %d \n",
       (long) gnvertex, (long) gnedge, (long) lnedge,  (IJ == 0) );
 };


 if (NULL != input_filename) {
    free(input_filename);
    input_filename = NULL;
 }


 BARRIER();
 double read_time_end = WTIME();
 if (rank == 0) {
   fprintf(stderr,"time to read edge list %f s\n", 
           read_time_end - read_time_start);
   fprintf(stderr,"number of vertices %ld, number of edges %ld\n", 
           gnvertex, gnedge );
 };


 assert( IJ != NULL );
 assert( gnvertex >= 1 );

#if (0)
 /*
  * ---------------------------
  * compute starting edge index
  * ---------------------------
  */
 {
   long ledge_start = 0;
   int n_pes = size;
   int my_pe = rank;
   long isize = (gnedge/n_pes);
   int imod = (int) (gnedge % ((long) n_pes) );

   if (my_pe < imod) {
       ledge_start = my_pe * (isize + 1);
   }
   else {
     ledge_start = imod * (isize + 1) + (my_pe - imod)*(isize);
   };
 }
#endif /* ZERO */





#ifndef USE_OPENSHMEM
  const char* filename = getenv("TMPFILE");
  const int reuse_file = getenv("REUSEFILE")? 1 : 0;
#endif
  /* If filename is NULL, store data in memory */

  tuple_graph tg;

#ifdef ORIGINAL
  tg.nglobaledges = (int64_t)(edgefactor) << SCALE;
  int64_t nglobalverts = (int64_t)(1) << SCALE;
#else
  {
  int n_pes = size;

  tg.nglobaledges = (int64_t) gnedge;
  tg.max_edgememory_size = (int64_t) ((gnedge + (n_pes-1))/n_pes);
  tg.edgememory_size = (int64_t) lnedge;
  }

  int64_t nglobalverts = (int64_t) gnvertex;

#endif

#ifdef USE_OPENSHMEM
  tg.data_in_file = 0;
  tg.write_file = 1;

#else
  tg.data_in_file = (filename != NULL);
  tg.write_file = 1;

  if (tg.data_in_file) {
    int is_opened = 0;
    int mode = FILE_MODE_RDWR | FILE_MODE_EXCL | FILE_MODE_UNIQUE_OPEN;
    if (!reuse_file) {
      mode |= FILE_MODE_CREATE | FILE_MODE_DELETE_ON_CLOSE;
    } else {
      MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_RETURN);
      if (MPI_File_open(MPI_COMM_WORLD, (char*)filename, mode,
			MPI_INFO_NULL, &tg.edgefile)) {
	if (0 == rank && getenv("VERBOSE"))
          fprintf (stderr, "%d: failed to open %s, creating\n",
		   rank, filename);
	mode |= FILE_MODE_RDWR | FILE_MODE_CREATE;
      } else {
	FILE_Offset size;
	MPI_File_get_size(tg.edgefile, &size);
	if (size == ((FILE_Offset) tg.nglobaledges) * sizeof(packed_edge)) {
	  is_opened = 1;
	  tg.write_file = 0;
	} else /* Size doesn't match, assume different parameters. */
	  MPI_File_close (&tg.edgefile);
      }
    }
    MPI_File_set_errhandler(MPI_FILE_NULL, MPI_ERRORS_ARE_FATAL);
    if (!is_opened) {
      MPI_File_open(MPI_COMM_WORLD, (char*)filename, mode, MPI_INFO_NULL, &tg.edgefile);
      MPI_File_set_size(tg.edgefile, ((FILE_Offset) tg.nglobaledges) * sizeof(packed_edge));
    }
    MPI_File_set_view(tg.edgefile, 0, packed_edge_mpi_type, packed_edge_mpi_type, "native", MPI_INFO_NULL);
    MPI_File_set_atomicity(tg.edgefile, 0);
  }
#endif

  /* Make the raw graph edges. */
  /* Get roots for BFS runs, plus maximum vertex with non-zero degree (used by
   * validator). */
#ifdef USE_DEBUG3
  int num_bfs_roots = 1; 
#else
  int num_bfs_roots = 64; 
#endif

  /* 
   * ------------------
   * Faster verification
   * ------------------
   */
  if (getenv("SHORT_VALIDATION")) { 
#ifdef USE_DEBUG
    num_bfs_roots = 1;
#else
    num_bfs_roots = 8;
#endif
    };
  int64_t* bfs_roots = (int64_t*)xmalloc( ((size_t) num_bfs_roots) * sizeof(int64_t));
  int64_t max_used_vertex = 0;



  BARRIER();
  double make_graph_start = WTIME();
  {
    /* Spread the two 64-bit numbers into five nonzero values in the correct
     * range. */
    uint_fast32_t seed[5];
    make_mrg_seed(seed1, seed2, seed);

    /* As the graph is being generated, also keep a bitmap of vertices with
     * incident edges.  We keep a grid of processes, each row of which has a
     * separate copy of the bitmap (distributed among the processes in the
     * row), and then do an allreduce at the end.  This scheme is used to avoid
     * non-local communication and reading the file separately just to find BFS
     * roots. */
    FILE_Offset nchunks_in_file = (tg.nglobaledges + FILE_CHUNKSIZE - 1) / FILE_CHUNKSIZE;
    int64_t bitmap_size_in_bytes = int64_min(BITMAPSIZE, (nglobalverts + CHAR_BIT - 1) / CHAR_BIT);
    if (bitmap_size_in_bytes * size * CHAR_BIT < nglobalverts) {
      bitmap_size_in_bytes = (nglobalverts + size * CHAR_BIT - 1) / (size * CHAR_BIT);
    }
    int ranks_per_row = ((nglobalverts + CHAR_BIT - 1) / CHAR_BIT + bitmap_size_in_bytes - 1) / bitmap_size_in_bytes;
    int nrows = size / ranks_per_row;
    int my_row = -1, my_col = -1;
    unsigned char* restrict has_edge = NULL;
#ifndef USE_OPENSHMEM
    MPI_Comm cart_comm;
#else
    void * cart_comm = &cart_comm;
#endif
#ifndef USE_OPENSHMEM
    {
      int dims[2] = {size / ranks_per_row, ranks_per_row};
      int periods[2] = {0, 0};
      MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &cart_comm);
    }
#endif
    int in_generating_rectangle = 0;
#ifndef USE_OPENSHMEM
    if (cart_comm != MPI_COMM_NULL) {
#else
     if (cart_comm != NULL) {
#endif
      in_generating_rectangle = 1;
      {
#ifndef USE_OPENSHMEM
        int dims[2], periods[2], coords[2];
        MPI_Cart_get(cart_comm, 2, dims, periods, coords);
        my_row = coords[0];
        my_col = coords[1];
#else
        assert( ranks_per_row >= 1 );
        my_row = shmem_my_pe()/ranks_per_row;
        my_col = (shmem_my_pe() % ranks_per_row);
#endif
      }
#ifndef USE_OPENSHMEM
      MPI_Comm this_col;
      MPI_Comm_split(cart_comm, my_col, my_row, &this_col);
      MPI_Comm_free(&cart_comm);
#endif
      has_edge = (unsigned char*)xMPI_Alloc_mem( ((size_t) bitmap_size_in_bytes));
      memset(has_edge, 0, ((size_t) bitmap_size_in_bytes));
      /* Every rank in a given row creates the same vertices (for updating the
       * bitmap); only one writes them to the file (or final memory buffer). */
      packed_edge* buf = (packed_edge*)xmalloc(FILE_CHUNKSIZE * sizeof(packed_edge));
      FILE_Offset block_limit = (nchunks_in_file + nrows - 1) / nrows;
      /* fprintf(stderr, "%d: nchunks_in_file = %" PRId64 ", block_limit = %" PRId64 " in grid of %d rows, %d cols\n", rank, (int64_t)nchunks_in_file, (int64_t)block_limit, nrows, ranks_per_row); */
      if (tg.data_in_file) {
        tg.edgememory_size = 0;
        tg.edgememory = NULL;
      } else {
        int my_pos = my_row + my_col * nrows;
        int64_t last_pos = (tg.nglobaledges % ((int64_t)FILE_CHUNKSIZE * nrows * ranks_per_row) != 0) ?
                       (tg.nglobaledges / FILE_CHUNKSIZE) % (nrows * ranks_per_row) :
                       -1;
        int64_t edges_left = tg.nglobaledges % FILE_CHUNKSIZE;
        int64_t nedges = FILE_CHUNKSIZE * (tg.nglobaledges / ((int64_t)FILE_CHUNKSIZE * nrows * ranks_per_row)) +
                         FILE_CHUNKSIZE * (my_pos < (tg.nglobaledges / FILE_CHUNKSIZE) % (nrows * ranks_per_row)) +
                         (my_pos == last_pos ? edges_left : 0);
        /* fprintf(stderr, "%d: nedges = %" PRId64 " of %" PRId64 "\n", rank, (int64_t)nedges, (int64_t)tg.nglobaledges); */
        tg.edgememory_size = nedges;
        tg.edgememory = (packed_edge*)xmalloc( ((size_t) nedges) * sizeof(packed_edge));
      }
      FILE_Offset block_idx;
      for (block_idx = 0; block_idx < block_limit; ++block_idx) {
        /* fprintf(stderr, "%d: On block %d of %d\n", rank, (int)block_idx, (int)block_limit); */
        FILE_Offset start_edge_index = int64_min(FILE_CHUNKSIZE * (block_idx * nrows + my_row), tg.nglobaledges);
        FILE_Offset edge_count = int64_min(tg.nglobaledges - start_edge_index, FILE_CHUNKSIZE);
        packed_edge* actual_buf = (!tg.data_in_file && block_idx % ranks_per_row == my_col) ?
                                  tg.edgememory + FILE_CHUNKSIZE * (block_idx / ranks_per_row) :
                                  buf;
        /* fprintf(stderr, "%d: My range is [%" PRId64 ", %" PRId64 ") %swriting into index %" PRId64 "\n", rank, (int64_t)start_edge_index, (int64_t)(start_edge_index + edge_count), (my_col == (block_idx % ranks_per_row)) ? "" : "not ", (int64_t)(FILE_CHUNKSIZE * (block_idx / ranks_per_row))); */
        if (!tg.data_in_file && block_idx % ranks_per_row == my_col) {
          assert (FILE_CHUNKSIZE * (block_idx / ranks_per_row) + edge_count <= tg.edgememory_size);
        }
	if (tg.write_file) {
#ifdef ORIGINAL
	  generate_kronecker_range(seed, SCALE, start_edge_index, start_edge_index + edge_count, actual_buf);
#else
      {
        const int idebug = 0;
        long end_edge_index = (start_edge_index + edge_count);
        long iedge;
        

        if (idebug >= 1) {
          fprintf(stderr,"rank %d: start_edge_index %ld, edge_count %ld\n",
                          rank,    start_edge_index,     edge_count );
        };

        int n_pes = size;
        for( iedge=start_edge_index; iedge < end_edge_index; iedge++) {
            long v0, v1;
            long idest = (iedge - start_edge_index);
            int pe;
            long loffset;

            pe = (iedge % n_pes );
            loffset = iedge/n_pes;
#if (0)
            long isize = gnedge/n_pes;
            long imod = (gnedge % n_pes );

            if (iedge < imod*(isize+1)) {
                pe = iedge/(isize+1);
                loffset = iedge = pe*(isize+1);
                assert( (0 <= loffset) && (loffset < isize+1));
            }
            else {
                long jedge = iedge - imod*(isize+1);
                pe = (jedge/isize) + imod;
                loffset = jedge - pe*isize;
                assert( (0 <= loffset) && (loffset < isize ) );
            };
            int isok = (0 <= pe) && (pe < n_pes);
            if (!isok) {
                fprintf(stderr,"rank %d: isize %ld imod %ld iedge %ld pe %d loffset %ld\n",
                                rank,    isize,    imod,    iedge,    pe,   loffset );
            };
#endif
            assert( (0 <= pe) && (pe < n_pes) );

            v0 = shmem_long_g( &(IJ[loffset].v0), pe );
            v1 = shmem_long_g( &(IJ[loffset].v1), pe );

            actual_buf[idest].v0 = v0;
            actual_buf[idest].v1 = v1;
        };
        if (idebug >= 1) {
          fprintf(stderr,"rank %d: after for loop 440\n",rank);
          };
      }
#endif
	  if (tg.data_in_file && my_col == (block_idx % ranks_per_row)) { /* Try to spread writes among ranks */
#ifndef USE_OPENSHMEM
	    MPI_File_write_at(tg.edgefile, start_edge_index, actual_buf, edge_count, packed_edge_mpi_type, MPI_STATUS_IGNORE);
#endif
	  }
	} else {
	  /* All read rather than syncing up for a row broadcast. */
#ifndef USE_OPENSHMEM
	  MPI_File_read_at(tg.edgefile, start_edge_index, actual_buf, edge_count, packed_edge_mpi_type, MPI_STATUS_IGNORE);
#endif
	}
        ptrdiff_t i;
#ifdef _OPENMP
#pragma omp parallel for
#endif
        for (i = 0; i < edge_count; ++i) {
          int64_t src = get_v0_from_edge(&actual_buf[i]);
          int64_t tgt = get_v1_from_edge(&actual_buf[i]);
          if (src == tgt) continue;
          if (src / bitmap_size_in_bytes / CHAR_BIT == my_col) {
#ifdef _OPENMP
#pragma omp atomic
#endif
            has_edge[(src / CHAR_BIT) % bitmap_size_in_bytes] |= (1 << (src % CHAR_BIT));
          }
          if (tgt / bitmap_size_in_bytes / CHAR_BIT == my_col) {
#ifdef _OPENMP
#pragma omp atomic
#endif
            has_edge[(tgt / CHAR_BIT) % bitmap_size_in_bytes] |= (1 << (tgt % CHAR_BIT));
          }
        }
      }
      free(buf);
#if 0
      /* The allreduce for each root acts like we did this: */
      MPI_Allreduce(MPI_IN_PLACE, has_edge, bitmap_size_in_bytes, MPI_UNSIGNED_CHAR, MPI_BOR, this_col);
#endif

#ifndef USE_OPENSHMEM
      MPI_Comm_free(&this_col);
#endif
    } else {
      tg.edgememory = NULL;
      tg.edgememory_size = 0;
    }
#ifdef USE_OPENSHMEM
   {
    long lvalue = tg.edgememory_size;
    long gvalue = shmem_long_max_all( lvalue );
    tg.max_edgememory_size = gvalue;

    assert( 0 <= tg.edgememory_size  );
    assert( tg.edgememory_size <= tg.max_edgememory_size );
   }
#else
    MPI_Allreduce(&tg.edgememory_size, &tg.max_edgememory_size, 1, MPI_INT64_T, MPI_MAX, MPI_COMM_WORLD);
#endif
    /* Find roots and max used vertex */
    {
      uint64_t counter = 0;
      int bfs_root_idx;
      for (bfs_root_idx = 0; bfs_root_idx < num_bfs_roots; ++bfs_root_idx) {
        int64_t root;
        while (1) {
          double d[2];
          make_random_numbers(2, seed1, seed2, counter, d);
          root = (int64_t)((d[0] + d[1]) * nglobalverts) % nglobalverts;
          counter += 2;
          if (counter > ( 2 * ((uint64_t) nglobalverts))) break;
          int is_duplicate = 0;
          int i;
          for (i = 0; i < bfs_root_idx; ++i) {
            if (root == bfs_roots[i]) {
              is_duplicate = 1;
              break;
            }
          }
          if (is_duplicate) continue; /* Everyone takes the same path here */
          int root_ok = 0;
          if (in_generating_rectangle && (root / CHAR_BIT / bitmap_size_in_bytes) == my_col) {
            root_ok = (has_edge[(root / CHAR_BIT) % bitmap_size_in_bytes] & (1 << (root % CHAR_BIT))) != 0;
          }
#ifdef USE_OPENSHMEM
         {
         int org_root_ok = root_ok;
         root_ok = shmem_int_lor_all( root_ok );
 
         if (org_root_ok) {
           assert( root_ok );
           };
         }
#else
          MPI_Allreduce(MPI_IN_PLACE, &root_ok, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
#endif
          if (root_ok) break;
        }
        bfs_roots[bfs_root_idx] = root;
      }
      num_bfs_roots = bfs_root_idx;

      /* Find maximum non-zero-degree vertex. */
      {
        int64_t i;
        max_used_vertex = 0;
        if (in_generating_rectangle) {
          for (i = bitmap_size_in_bytes * CHAR_BIT; i > 0; --i) {
            if (i > nglobalverts) continue;
            if (has_edge[(i - 1) / CHAR_BIT] & (1 << ((i - 1) % CHAR_BIT))) {
              max_used_vertex = (i - 1) + my_col * CHAR_BIT * bitmap_size_in_bytes;
              break;
            }
          }
        }
#ifdef USE_OPENSHMEM
      {
      long lvalue = max_used_vertex;
      long gvalue = shmem_long_max_all( lvalue );
      max_used_vertex = gvalue;

#ifdef USE_DEBUG
      printf("%d: lvalue %ld gvalue %ld max_used_vertex %ld \n",
          shmem_my_pe(), lvalue, gvalue, max_used_vertex );
#endif
      }
#else
        MPI_Allreduce(MPI_IN_PLACE, &max_used_vertex, 1, MPI_INT64_T, MPI_MAX, MPI_COMM_WORLD);
#endif
      }
    }
    if (in_generating_rectangle) {
      xMPI_Free_mem(has_edge);
    }

#ifndef USE_OPENSHMEM
    if (tg.data_in_file && tg.write_file) {
      MPI_File_sync(tg.edgefile);
    }
#endif
  }
  BARRIER();
  double make_graph_stop = WTIME();
  double make_graph_time = make_graph_stop - make_graph_start;
  if (rank == 0) { /* Not an official part of the results */
    fprintf(stderr, "graph_generation:               %f s\n", make_graph_time);
  }

  if (IJ != NULL) {
#ifdef USE_OPENSHMEM
    shfree( IJ );
#else
    free( IJ );
#endif
  };

  /* Make user's graph data structure. */
  BARRIER();
  double data_struct_start = WTIME();
  make_graph_data_structure(&tg);
  BARRIER();
  double data_struct_stop = WTIME();
  double data_struct_time = data_struct_stop - data_struct_start;
  if (rank == 0) { /* Not an official part of the results */
    fprintf(stderr, "construction_time:              %f s\n", data_struct_time);
  }

  /* Number of edges visited in each BFS; a double so get_statistics can be
   * used directly. */
  double* edge_counts = (double*)xmalloc( ( (size_t) num_bfs_roots) * sizeof(double));

  /* Run BFS. */
  int validation_passed = 1;
  double* bfs_times = (double*)xmalloc( ( (size_t) num_bfs_roots) * sizeof(double));
  double* validate_times = (double*)xmalloc( ((size_t) num_bfs_roots) * sizeof(double));
  uint64_t nlocalverts = get_nlocalverts_for_pred();
  int64_t* pred = (int64_t*)xMPI_Alloc_mem( ( (size_t) nlocalverts) * sizeof(int64_t));
 

  int bfs_root_idx;
  for (bfs_root_idx = 0; bfs_root_idx < num_bfs_roots; ++bfs_root_idx) {
    int64_t root = bfs_roots[bfs_root_idx];

    if (rank == 0) fprintf(stderr, "Running BFS %d\n", bfs_root_idx);

    /* Clear the pred array. */
    memset(pred, 0, nlocalverts * sizeof(int64_t));

    /* Do the actual BFS. */
    BARRIER();
    double bfs_start = WTIME();
    run_bfs(root, &pred[0]);
#ifdef USE_DEBUG
    printf("after run_bfs: rank %d pred[0] %ld\n", rank, pred[0] );
    {
    int pe = 0;
    int istat = 0;
    long i = 0;
    FILE *fp = NULL;

    BARRIER();
    for(pe=0; pe < size; pe++) {
       BARRIER();
       if (pe == shmem_my_pe()) {
        fp = fopen("bfs_output.txt", "a" );
        assert( fp != NULL );
        istat = fseek(fp, (long) 0, SEEK_END );
        assert( istat  != -1 );

        fprintf(fp,"%% rank %d root %ld nlocalverts %ld\n",rank,root,nlocalverts);
        fprintf(fp,"%% global_vertex    pred\n");
        for(i=0; i < nlocalverts; i++) {
           fprintf(fp,"%ld %ld\n",VERTEX_TO_GLOBAL(rank,i),pred[i]);
        };
        istat = fclose(fp);
        assert( istat == 0);
       };
      };
    BARRIER();
    }

#endif
    BARRIER();
    double bfs_stop = WTIME();
    bfs_times[bfs_root_idx] = bfs_stop - bfs_start;
    if (rank == 0) fprintf(stderr, "Time for BFS %d is %f\n", bfs_root_idx, bfs_times[bfs_root_idx]);

    /* Validate result. */
    if (!getenv("SKIP_VALIDATION")) {
      if (rank == 0) fprintf(stderr, "Validating BFS %d\n", bfs_root_idx);

      BARRIER();
      double validate_start = WTIME();
      int64_t edge_visit_count;

#ifdef USE_DEBUG
      printf("before validate_bfs_result: max_used_vertex %ld, nlocalverts %ld, root %ld, pred[0] %ld\n",
           (long) max_used_vertex, (long) nlocalverts, (long) root, (long) pred[0] );
#endif


      int validation_passed_one = validate_bfs_result(&tg, max_used_vertex + 1, nlocalverts, root, pred, &edge_visit_count);

      BARRIER();
      double validate_stop = WTIME();
      validate_times[bfs_root_idx] = validate_stop - validate_start;
      if (rank == 0) fprintf(stderr, "Validate time for BFS %d is %f\n", bfs_root_idx, validate_times[bfs_root_idx]);
      edge_counts[bfs_root_idx] = (double)edge_visit_count;
      if (rank == 0) fprintf(stderr, "TEPS for BFS %d is %g\n", bfs_root_idx, (double) edge_visit_count / bfs_times[bfs_root_idx]);

      if (!validation_passed_one) {
	validation_passed = 0;
	if (rank == 0) fprintf(stderr, "Validation failed for this BFS root; skipping rest.\n");
	break;
      }
    } else {
      validate_times[bfs_root_idx] = -1;
    }
  }

  xMPI_Free_mem(pred);
  free(bfs_roots);
  free_graph_data_structure();

  if (tg.data_in_file) {
#ifndef USE_OPENSHMEM
    MPI_File_close(&tg.edgefile);
#endif
  } else {
    free(tg.edgememory); tg.edgememory = NULL;
  }

  /* Print results. */
  if (rank == 0) {
    if (!validation_passed) {
      fprintf(stdout, "No results printed for invalid run.\n");
    } else {
      int i;
      fprintf(stdout, "SCALE:                          %d\n", SCALE);
      fprintf(stdout, "edgefactor:                     %d\n", edgefactor);
      fprintf(stdout, "NBFS:                           %d\n", num_bfs_roots);
      fprintf(stdout, "graph_generation:               %g\n", make_graph_time);
      fprintf(stdout, "num_mpi_processes:              %d\n", size);
      fprintf(stdout, "construction_time:              %g\n", data_struct_time);
      double stats[s_LAST];
      get_statistics(bfs_times, num_bfs_roots, stats);
      fprintf(stdout, "min_time:                       %g\n", stats[s_minimum]);
      fprintf(stdout, "firstquartile_time:             %g\n", stats[s_firstquartile]);
      fprintf(stdout, "median_time:                    %g\n", stats[s_median]);
      fprintf(stdout, "thirdquartile_time:             %g\n", stats[s_thirdquartile]);
      fprintf(stdout, "max_time:                       %g\n", stats[s_maximum]);
      fprintf(stdout, "mean_time:                      %g\n", stats[s_mean]);
      fprintf(stdout, "stddev_time:                    %g\n", stats[s_std]);
      get_statistics(edge_counts, num_bfs_roots, stats);
      fprintf(stdout, "min_nedge:                      %.11g\n", stats[s_minimum]);
      fprintf(stdout, "firstquartile_nedge:            %.11g\n", stats[s_firstquartile]);
      fprintf(stdout, "median_nedge:                   %.11g\n", stats[s_median]);
      fprintf(stdout, "thirdquartile_nedge:            %.11g\n", stats[s_thirdquartile]);
      fprintf(stdout, "max_nedge:                      %.11g\n", stats[s_maximum]);
      fprintf(stdout, "mean_nedge:                     %.11g\n", stats[s_mean]);
      fprintf(stdout, "stddev_nedge:                   %.11g\n", stats[s_std]);
      double* secs_per_edge = (double*)xmalloc( ((size_t) num_bfs_roots) * sizeof(double));
      for (i = 0; i < num_bfs_roots; ++i) secs_per_edge[i] = bfs_times[i] / edge_counts[i];
      get_statistics(secs_per_edge, num_bfs_roots, stats);
      fprintf(stdout, "min_TEPS:                       %g\n", 1. / stats[s_maximum]);
      fprintf(stdout, "firstquartile_TEPS:             %g\n", 1. / stats[s_thirdquartile]);
      fprintf(stdout, "median_TEPS:                    %g\n", 1. / stats[s_median]);
      fprintf(stdout, "thirdquartile_TEPS:             %g\n", 1. / stats[s_firstquartile]);
      fprintf(stdout, "max_TEPS:                       %g\n", 1. / stats[s_minimum]);
      fprintf(stdout, "harmonic_mean_TEPS:             %g\n", 1. / stats[s_mean]);
      /* Formula from:
       * Title: The Standard Errors of the Geometric and Harmonic Means and
       *        Their Application to Index Numbers
       * Author(s): Nilan Norris
       * Source: The Annals of Mathematical Statistics, Vol. 11, No. 4 (Dec., 1940), pp. 445-448
       * Publisher(s): Institute of Mathematical Statistics
       * Stable URL: http://www.jstor.org/stable/2235723
       * (same source as in specification). */
      fprintf(stdout, "harmonic_stddev_TEPS:           %g\n", stats[s_std] / (stats[s_mean] * stats[s_mean] * sqrt(num_bfs_roots - 1)));
      free(secs_per_edge); secs_per_edge = NULL;
      free(edge_counts); edge_counts = NULL;
      get_statistics(validate_times, num_bfs_roots, stats);
      fprintf(stdout, "min_validate:                   %g\n", stats[s_minimum]);
      fprintf(stdout, "firstquartile_validate:         %g\n", stats[s_firstquartile]);
      fprintf(stdout, "median_validate:                %g\n", stats[s_median]);
      fprintf(stdout, "thirdquartile_validate:         %g\n", stats[s_thirdquartile]);
      fprintf(stdout, "max_validate:                   %g\n", stats[s_maximum]);
      fprintf(stdout, "mean_validate:                  %g\n", stats[s_mean]);
      fprintf(stdout, "stddev_validate:                %g\n", stats[s_std]);
#if 0
      for (i = 0; i < num_bfs_roots; ++i) {
        fprintf(stdout, "Run %3d:                        %g s, validation %g s\n", i + 1, bfs_times[i], validate_times[i]);
      }
#endif
    }
  }
  free(bfs_times);
  free(validate_times);

  cleanup_globals();
#ifdef USE_OPENSHMEM
  shmem_barrier_all();
#else
  MPI_Finalize();
#endif
  return 0;
}
