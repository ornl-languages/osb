OpenSHMEM Implementation of Graph500 Benchmark
----------------------------------------------


This package contains an implementation of the Graph500 Benchmark using
OpenSHMEM. The code is based on the MPI-2 one-sided reference
implementation.


Building Instructions
---------------------

The source code for OpenSHMEM implementation is under `mpi/` directory.

  - 1. Create make.inc for specifying  compiler options by modifying one
       of the existing files such as `make-incs/make.inc.oshmem.gcc`,
       or `make-incs/make.inc.oshmem.gcc.cray`.
       Make sure that the 'make.inc' file includes `BUILD_OPENSHMEM = Yes`.
       Note, the `make.inc` file should be at the top-level of the Graph500
       software tree, i.e., parent directory containing `mpi/`.

  - 2. On the Cray machines, load the appropriate modules for the 
       Cray implementation of OpenSHMEM.   Also, set the relevant
       environment variables for OpenSHMEM usage.
       (See also: `contrib/run_scripts_2/setup_cray_shmem.sh`)

       For example,

            ```
              module unload PrgEnv-intel
              module unload PrgEnv-pgi
              module unload PrgEnv-cray

              module load   PrgEnv-gnu
              module load   cray-shmem
       
              export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000
              export XT_SYMMETRIC_HEAP_SIZE=1G
      
            ```

  - 2. Setup build environment. On the Cray XK7 Titan, perform
       `source mpi/setup_cray_shmem.sh` to load the appropriate modules for
       the Cray implementation of OpenSHMEM and MPI.

  - 4. Change to the `mpi/` directory.

  - 5. Use `make` to create different executables

        - `make graph500_shmem_one_sided`:  OpenSHMEM implementation
            with default RMAT/Kronecker graph generator

        - `make graph500_shmem_one_sided_er`:  OpenSHMEM implementation
            with Erdos-Renyi graph generator

        - `make graph500_mpi_one_sided`: MPI-2 one-sided reference
            implementation with default RMAT/Kronecker graph generator

        - `make graph500_mpi_one_sided_lock`:  MPI-2 one-sided reference
            implementation with calls to `MPI_Win_lock()`/`MPI_Win_unlock()`

        - `make graph500_mpi_one_sided_lock_er`: MPI-2 one-sided reference
            implementation with calls to `MPI_Win_lock()`/`MPI_Win_unlock()`
            with Erdos-Renyi graph generator

  - 6.  Type `make iodriver_shmem` to create OpenSHMEM driver that reads
        in a graph in edge list ascii text format.  If the file has `.bin`
        as suffix, the file is assumed to be in binary format.

  - 7. Launch benchmark by specifying the number of MPI tasks, the scale
       of the graph (number of vertices = 2^scale) and the degree parameter.
       For example on the Cray XK7 Titan, use the following command
       to launch the benchmark on 128 MPI tasks, on a graph with `2^18`
       vertices and degree parameter of 16.

        ```
          aprun -n 128 ./graph500_shmem_one_sided 18 16
        ```

       Or in another implementation of MPI:

        ```
          mpirun -np 128 ./graph500_shmem_one_sided 17 16
        ```

  - 8. Launch `iodriver_shmem` benchmark for reading in a graph in edge
       list format. The number of vertices and number of edges are contained
       in the first record in the file.
       For example, to read in the graph in `file.s16`:

        ```
          aprun -n 32 ./iodriver_shmem file.s16
        ```

Graph generators
----------------

The Graph500 benchmark has a reference implementation in Matlab/Octave.
There is a Kronecker graph generator in `kronecker_generator.m`.  There is
a Chung-Lu graph generator in `chunglu_generator.m`. This generator takes
as input a degree distribution and generates a graph that approximately
reproduces the desired degree distribution. The approach is described in
"The Similarity between Stochastic Kronecker and Chung-Lu Graph Models" by
Ali Pinar, C. Seshadhri, Tamara G. Kolda (http://arxiv.org/abs/1110.4925).

The Matlab function `write_binary_edgelist.m` writes out the edge list in
binary format instead of ascii text format.


Building Notes (OSHMEM VERSION)
-------------------------------

The basic structure is the same as outlined in the main README
for the `Makefile`, which includes the `make.inc` to define compiler
and flag variables, see `make-incs/` directory.
Example: `incs/make-incs/make.inc.oshmem.gcc`
      or `make-incs/make.inc.oshmem.gcc.cray`

To build the OpenSHMEM executables, set `BUILD_OSHMEM=yes` in your
`make.inc`, and set `OSHCC` if needed.  This is similar to the MPI
build.

Note, there are guards around the OpenSHMEM code paths that allow 
for an entirely OpenSHMEM build. There is no need to define `MPICC`
or `BUILD_MPI=yes` if working with the OpenSHMEM variation of Graph500.
However, you must build from within the `mpi/` directory.  For example,

    ```
        # From top-level Graph500 directory
       make -C mpi shmem
    ```

Or you can build directly within the `mpi/` directory itself.
(see also the make target descriptions in above Building Instructions)

    ```
       cd mpi/
       make shmem
    ```


Running Instructions (OSHMEM VERSION)
-------------------------------------

Example command-lines for running the Graph500-oshmem executable 
with the Open MPI implementation of OpenSHMEM.  

 - Running Scale=16, EdgeFactor=16 over two hosts using 2 PEs (1 per node),
   using the `orterun` launcher from Open MPI.

    ```
      orterun \
        -mca spml ucx \
        -x SHMEM_SYMMETRIC_HEAP_SIZE=640000000 \
        --np 2 \
        --map-by node \
        --hostfile myhosts \
        ./graph500_shmem_one_sided  16 16
    ```

 - Running Scale=24, EdgeFactor=16 over two hosts using 32 PEs (16 per node),
   using the `aprun` launcher from Cray ALPS. 

    ```
      aprun \
        -n 32 \
        -N 16 \
        -q \
        -e XT_SYMMETRIC_HEAP_SIZE=640000000 \
        ./graph500_shmem_one_sided 24 16
    ```



General Notes (OSHMEM VERSION)
------------------------------

 - To disable validation during the stages of the benchmark,
   set the environment variable `SKIP_VALIDATION=1`. 

 - To perform a shorter set of runs/validation during the benchmark,
   set the environment variable `SHORT_VALIDATION=1`.

 - This version of the code was testing with the following configurations
    - Cray XC30 ("Eos") with Aries interconnect running Cray-shmem.

        ```
         bash:$ module load cray-shmem
         bash:$ echo $CRAY_SHMEM_VER
         7.5.2
        ```

    - Linux cluster with Mellanox Infiniband (mlx5) running
      Open MPI implementation of OpenSHMEM using UCX.

        ```
         bash:$ orterun --version
         orterun (OpenRTE) 2.1.2rc4
         Report bugs to http://www.open-mpi.org/community/help/
        ```

        ```
         bash:$ ucx_info -v
         # UCT version=1.4.0 revision baec0c3
         # configured with: --prefix=/ccs/proj/csc040/naughton/ucx_install
        ```

 - Cray XC30 ("Eos"): Verified input parameters

    - NOTE: Status of `ok` means first few iterations/validations worked,
      but did not wait for the full 64 BFS/validations.

    ```
      STATUS    SCALE   EDGE FACTOR     NPROCS (-np)    SYM_HEAP_SZ
      ------    -----   -----------     ------------    -----------
        ok      26       16             32 (2 hosts)    1200000000
        ok      24       16             32 (2 hosts)    1200000000
       fail     24       16             32 (2 hosts)    640000000
        ok      22       16             32 (2 hosts)    640000000
        ok      20       16             32 (2 hosts)    640000000
        ok      18       16             32 (2 hosts)    640000000
        ok      17       16             32 (2 hosts)    640000000
       pass     16       16             32 (2 hosts)    640000000
       pass     14       16             32 (2 hosts)    640000000
       pass     12       16             32 (2 hosts)    640000000
       fail     10       16             32 (2 hosts)    640000000
       fail      8       16             32 (2 hosts)    640000000
    ```

 - Linux cluster: Verified input parameters

    - NOTE: Status of `ok` means first few iterations/validations worked,
      but did not wait for the full 64 BFS/validations.

    ```
      STATUS    SCALE   EDGE FACTOR     NPROCS (-np)    SYM_HEAP_SZ
      ------    -----   -----------     ------------    -----------
        ok      24       16             2 (2 hosts)     1200000000
        ok      22       16             2 (2 hosts)     1200000000
        ok      22       8              2 (2 hosts)     1200000000
       fail     22       8              2 (2 hosts)     640000000
        ok      20       16             2 (2 hosts)     1200000000
       fail     20       16             2 (2 hosts)     640000000
        ok      20       8              2 (2 hosts)     640000000
        ok      18       16             2 (2 hosts)     640000000
       pass     16       16             2 (2 hosts)     640000000
       pass     14      16              2 (2 hosts)     640000000
       pass     12      16              2 (2 hosts)     640000000
       fail     10      16              2 (2 hosts)     640000000
       fail      8      16              2 (2 hosts)     640000000
    ```


Known Issues (OSHMEM VERSION)
-----------------------------

 - Generally should build from sub-directories (e.g., `make -C mpi`)
   and may need to manually modify Makefiles and edit make.inc.

 - To build `generator/`, create `generator/Makefile` from examples in
   that subdirectory and build from sub-directory (e.g., `make -C generator`).
   Note, tested with `generator/Makefile.shmem`.

 - There is currently an error when running `make` from the top-level
   directory that fails to build `seq-list/seq-list` when using a purely
   OpenSHMEM setup.  The additional (non-shmem) binaries are disabled
   in top-level Makefile to avoid this issue. (Issue #55)

 - (Single threading case) When using purely OpenSHMEM setup, we currently
   get warnings about the lack of OpenMP in some cases.  This can generally
   be ignored when not using OpenMP, but guards should be added to avoid
   these warnings in the future. For example:

    ```
      warning: ignoring #pragma omp parallel [-Wunknown-pragmas]
        #pragma omp parallel for
    ```

 - Occasionally, Graph500 experiences validation errors during testing
   with the ORNL oshmem-x.  We are investigating the root cause. (Issue #23)
   Example output:

    ```
         ...<snip>...
       Running BFS 9
       Time for BFS 9 is 0.020741
       Validating BFS 9
       0: Validation error: edge connects vertex 2491 in the BFS tree (depth 2)
       to vertex 417 outside the tree.
       Validate time for BFS 9 is 0.017766
       TEPS for BFS 9 is 2.3697e+06
       Validation failed for this BFS root; skipping rest.
         ...<snip>...
    ```


Troubleshooting (OSHMEM VERSION)
--------------------------------

 - 1. Errors about bad destination address during graph construction.
        - Make sure the `SHMEM_SYMMETRIC_HEAP_SIZE` environment variable is 
          set and large enough for the given problem size.

    ```
       # ERROR
     headnode:$ orterun --hostfile hosts --np 2 --map-by node \
               ./graph500_shmem_one_sided 12 12  
     graph_generation:               5.644663 s
     pe 0: total_recvcounts 8498202 size_in_bytes 16
     graph500_shmem_one_sided: shmem_lib.c:882: shmem_mem_alltoallv: Assertion `target != ((void *)0)` failed.
     pe 1: total_recvcounts 8498202 size_in_bytes 16
     graph500_shmem_one_sided: shmem_lib.c:882: shmem_mem_alltoallv: Assertion `target != ((void *)0)` failed.
     [node1:24872] *** Process received signal ***
     [node1:24872] Signal: Aborted (6)
       ...<snip>...
    ```

    ```
       # WORKING
     headnode:$ orterun --mca spml ucx --hostfile hosts --np 2 --map-by node \
                -x SHMEM_SYMMETRIC_HEAP_SIZE=640000000 \
               ./graph500_shmem_one_sided 12 12  
     graph_generation:               5.680895 s
     construction_time:              1.442636 s
     Running BFS 0
     Time for BFS 0 is 15.518293
     Validating BFS 0
       ...<snip>...
    ```


 - 2. Errors at run time from Open MPI OSHMEM with UCX related to 
      undefined symbol for the UCX components, e.g.,  `mca_spml_ucx`.
      (See example below)
        - Try adding `--mca spml ucx` to the `oshrun` (`orterun`) command-line.

    ```
       # ERROR
     headnode:$ orterun --hostfile hosts --np 2 --map-by node \
               ./graph500_shmem_one_sided 12 12  
     [node2:32786] mca_base_component_repository_open: unable to open mca_atomic_ucx: 
  /opt/sw/ompi_install/lib/openmpi/mca_atomic_ucx.so: undefined symbol: mca_spml_ucx (ignored)
     [node1:32786] mca_base_component_repository_open: unable to open mca_atomic_ucx: 
  /opt/sw/ompi_install/lib/openmpi/mca_atomic_ucx.so: undefined symbol: mca_spml_ucx (ignored)
     graph_generation:               0.047777 s
     construction_time:              0.025800 s
     Running BFS 0
       ...<snip>...
    ```

    ```
       # WORKING
     headnode:$ orterun --mca spml ucx --hostfile hosts --np 2 --map-by node \
               ./graph500_shmem_one_sided 12 12  
     graph_generation:               0.048262 s
     construction_time:              0.026794 s
     Running BFS 0
       ...<snip>...
    ```


 - 3. Errors at run time about `mmap()` failures during allocation.
        - Try removing `-UUSE_MMAP_LARGE -UUSE_MMAP_LARGE_EXT` from 
          the `CFLAGS` (e.g., see `make.inc`), or check host configurations.

