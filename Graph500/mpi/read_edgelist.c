/* Copyright (C) 2014  Oak Ridge National Laboratory */
/*  Authors: Ed D'Azevedo  (OpenSHMEM version)       */
#include "read_edgelist.h"

/*
 * broadcast a single long from root = 0 to all processors
 */
#ifdef USE_OPENSHMEM
void long_bcast_all( long *ldest, long *lsrc )
{

  /* ---------------------------------------------------------------------  */
  /* void shmem_broadcast64(void *target, const void *source, size_t nlong, */
  /* int PE_root, int PE_start, int logPE_stride, int PE_size, long *pSync) */
  /* ---------------------------------------------------------------------  */
  const int idebug = 0;
  const int my_pe = shmem_my_pe();
  const int n_pes = shmem_n_pes();

  const size_t nlong = 1;
  const int PE_root = 0;
  const int PE_start = 0;
  const int logPE_stride = 0;
  const int PE_size = n_pes;
  static long pSync[ _SHMEM_BCAST_SYNC_SIZE ];

  static long src = 0;
  static long dest = 0;

  assert( lsrc != NULL );
  assert( ldest != NULL );

  

  if (n_pes == 1) {
    /* --------------------------------------- */
    /* just 1 processor, no need for broadcast */
    /* --------------------------------------- */
    *ldest = *lsrc;
  }
  else {

  int i;
  for(i=0; i <  _SHMEM_BCAST_SYNC_SIZE; i++) {
      pSync[i] = _SHMEM_SYNC_VALUE;
      };
  shmem_barrier_all();



  src = *lsrc;
  dest = src;
  shmem_broadcast64( &dest, &src, nlong, 
        PE_root, PE_start, logPE_stride, PE_size, pSync );
  /*
   * -------------------------------------------------------------
   * note dest/target in PE_root may  not be modified in broadcast
   * -------------------------------------------------------------
   */
  *ldest = dest;
  if (idebug >= 1) {
  printf("long_bcast_all: my_pe %d src %ld dest %ld *lsrc %ld *ldest %ld\n",
           my_pe, src, dest, *lsrc, *ldest );
  };



  };


}
#else
void long_bcast_all( long *ldest, long *lsrc )
{
  /* 
   * --------------------------------------------------------------
   * int MPI_Bcast( void *buffer, int count, MPI_Datatype datatype, 
   *                   int root, MPI_Comm comm ) 
   * --------------------------------------------------------------
   */
  int istatus = 0;
  long lbuffer[1]; 
  const int count = 1;
  const int root = 0;
  const MPI_Datatype datatype = MPI_LONG;


  assert( lsrc != NULL );
  assert( ldest != NULL );

  lbuffer[0] = *lsrc;
  istatus = MPI_Bcast( lbuffer,  count, datatype,
            root, MPI_COMM_WORLD );

  assert( istatus == MPI_SUCCESS );
 
  *ldest = lbuffer[0];
}
#endif


int read_edgelist( const char *filename, 
                   long *p_gnvertex, 
                   long *p_gnedge, 
                   long *p_nedge,  
                   struct packed_edge ** p_IJ, 
                   int use_binary) 
{

  const int idebug = 0;

  struct packed_edge *IJ = NULL;

  int my_pe = 0;
  int n_pes = 0;
  int pe = 0;
  int istatus = 0;

  static long v0;
  static long v1;

  long gnvertex = 0;
  long gnedge = 0;
  long nedge = 0;
  int i = 0;
  long tmp[2];
  size_t isize;

  const char *mode = (use_binary ? "rb" : "r");
  FILE *fid = NULL;

 assert( p_gnedge != NULL );
 assert( p_gnvertex != NULL );
 assert( p_nedge != NULL );
 assert( p_IJ != NULL );

#ifdef USE_OPENSHMEM
  my_pe = shmem_my_pe();
  n_pes = shmem_n_pes();
#else
  MPI_Comm_rank( MPI_COMM_WORLD, &my_pe);
  MPI_Comm_size( MPI_COMM_WORLD, &n_pes );
#endif
  

 assert( filename != NULL );
  
 if (my_pe == 0) {
  fid = fopen(filename,mode);
  if (fid == NULL) {
     fprintf(stderr,"Cannot open file : %s\n",
               filename );
     return EXIT_FAILURE;
     };

  rewind( fid );

  if (use_binary) {
    isize = fread( &(tmp[0]), sizeof(long), 2, fid );
    assert( isize == 2 );
    gnvertex = tmp[0];
    gnedge = tmp[1];
  }
  else {
    istatus = fscanf( fid, "%ld %ld\n", &gnvertex, &gnedge );
    if (istatus != 2) {
      return( istatus );
      };
    };

  if (idebug >= 1) {
      printf("after reading header: gnvertex %ld gnedge %ld\n",
              (long) gnvertex, (long) gnedge );
      };
  };



  /* ----------------------------------------------------------- */
  /* broadcast the global number of vertices and number of edges */
  /* ----------------------------------------------------------- */
  {
  long lsrc = 0;
  long ldest = 0;

   lsrc = gnvertex;
   long_bcast_all(&ldest, &lsrc );
   gnvertex = ldest;

   lsrc = gnedge;
   long_bcast_all( &ldest,  &lsrc );
   gnedge = ldest;


   if (idebug >= 1) {
      printf("after bcast: my_pe %d, gnvertex %ld gnedge %ld\n",
             my_pe, (long) gnvertex, (long) gnedge );
      };
   }


  /*
   * ------------------------------
   * allocate global storage for IJ
   * ------------------------------
   */
  {
  long nitems = (long) ( (gnedge + (n_pes-1))/(n_pes) );
  size_t nbytes = nitems * sizeof( *IJ );
#ifdef USE_OPENSHMEM
  IJ = (struct packed_edge *) shmalloc( nbytes );
#else
  IJ = malloc( nbytes );
#endif
  assert( IJ != NULL );

  if (idebug >= 1) {
      printf("after xmalloc: my_pe %d, gnedge %ld, nitems %ld\n",
               my_pe, gnedge, nitems );
      };
  }





  for(pe = 0; pe < n_pes; pe++) {
    
#ifdef USE_OPENSHMEM
    shmem_barrier_all();
#else
    istatus = MPI_Barrier( MPI_COMM_WORLD );
    assert( istatus == MPI_SUCCESS );
#endif
 
    nedge = gnedge/n_pes;
    if (pe < ( gnedge %  n_pes )) { nedge++; };

    for(i=0; i < nedge; i++) {

      if (my_pe == 0) {
        if (use_binary) {
          size_t isize = fread( &(tmp[0]), sizeof(long), 2, fid );
          assert( isize == 2 );
          v0 = tmp[0];
          v1 = tmp[1];
          }
        else {
          istatus = fscanf( fid, "%ld %ld\n", &v0, &v1 ); 
          assert( istatus  == 2 );
          };
        };

      if (my_pe == 0) {

#ifdef USE_OPENSHMEM
          /*
           * -----------
           * 1-sided put
           * -----------
           */
          shmem_long_p( &(IJ[i].v0), v0, pe );
          shmem_long_p( &(IJ[i].v1), v1, pe );
          shmem_quiet();
#else
         /*
          * ---------------------------------------------
          * int MPI_Send(const void *buf, int count, 
          *         MPI_Datatype datatype, int dest, int tag,
          *         MPI_Comm comm)
          * ---------------------------------------------
          */
          {
           long buf2[2];
           const int count = 2;
           const MPI_Datatype datatype = MPI_LONG;
           const int dest = pe;
           const int tag = pe + 1;

           buf2[0] = v0;
           buf2[1] = v1;

           istatus = MPI_Send(buf2, count, datatype, dest, tag, 
                     MPI_COMM_WORLD );
           assert(istatus == MPI_SUCCESS );
          }
#endif
          }
      else {
#ifdef USE_OPENSHMEM
         /* do nothing */
#else
          /*
           * -------------------------------------------------------
           * int MPI_Recv(void *buf, int count, 
           *              MPI_Datatype datatype, int source, int tag,
           *              MPI_Comm comm, MPI_Status *status)
           * -------------------------------------------------------
           */
             if (my_pe == pe) {
               long buf2[2];
               const int count = 2;
               const int root = 0;
               const int source = root;
               const tag = my_pe + 1;
               const MPI_Datatype datatype = MPI_LONG;
               MPI_Status status;

               istatus = MPI_Recv( buf2, count, datatype, source, tag, 
                         MPI_COMM_WORLD, &status );
               assert( istatus == MPI_SUCCESS );
               v0 = buf2[0];
               v1 = buf2[1];
               IJ[i].v0 = v0;
               IJ[i].v1 = v1;
               };
#endif
   
        };
      }; /* end for i */
   }; /* end for pe */

  if (my_pe == 0) {
    istatus = fclose( fid );
    assert( istatus == 0 );
    };


 nedge = gnedge/n_pes;
 if (my_pe < (gnedge % n_pes)) { nedge++; };



 *p_gnedge = gnedge;
 *p_gnvertex = gnvertex;
 *p_nedge = nedge;
 *p_IJ = IJ;

#ifdef USE_OPENSHMEM
 shmem_barrier_all();
#else
  istatus = MPI_Barrier( MPI_COMM_WORLD );
  assert(istatus == MPI_SUCCESS );
#endif  

 return(0);
}
