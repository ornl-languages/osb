
Graph500 Benchmark
------------------

The Graph 500 benchmark (http://www.graph500.org) is inspired by the HPL
(High Performance Linpack) Top 500 benchmark (http://www.top500.org) used
for ranking the top 500 supercomputers.

The HPL benchmark mostly measures the floating point capability in solving a
large dense matrix problem via LU factorization.  This benchmark has very
high spatial and temporal locality.  In contrast, the Graph 500 benchmark
was designed for emulating large-scale data intensive applications (such as
computing breadth-first search (BFS) on large undirected graphs) that are
not dominated by floating point calculations and has low spatial and
temporal locality.  This benchmark has fine-grained communication and
seemingly random access patterns.  The Graph 500 benchmark has reference
implementations in GNU Octave (www.octave.org), parallel C version with
OpenMP, basic MPI-1 and MPI-2 one-sided communication.

The benchmark proceeds in several phases.  The first phase is the parallel
generation of graph edges using the Kronecker graph generator, which is
similar to the Recursive MATrix (R-MAT) graph generator.  The benchmark then
records the time for the conversion from the distributed edge list for an
undirected graph to a compact representation as a compressed storage format.

The size of the graph is described by the two parameters scale factor and
edge factor.  The number of graph vertices is given by 2^(scale) factor and
2 x edge factor is the average vertex degree.

For example, scale factor = 20 produces an undirected graph with
2^20 = 1,048,576 vertices.

The total memory required is 2^(scale) factor x edge factor x 2 x 8 bytes.
For ranking purpose, the benchmark defines several scale settings:
    - toy (2^26 vertices, 17 GB of RAM),
    - mini (2^29, 137 GiB),
    - small (2^32, 1.1 TiB),
    - medium (2^36, 17.6 TiB),
    - large (2^39, 140 TiB), and
    - huge (2^42, 1.1 PiB).

The next stage records the time for the generation of the breadth-first
search (BFS) tree on 64 randomly selected vertices.  The benchmark collects
statistics on the run-time for construction of the BFS search tree.

A validation phase checks the correctness of the resulting BFS search tree
that it has no cycles, each edge connects vertices that differ by 1 level,
and all vertices in the connected component are visited.  This verification
of the BFS search tree can be time consuming and is also used to estimate
the performance of the computer system by measuring the number of traversed
edges per second (TEPS).

