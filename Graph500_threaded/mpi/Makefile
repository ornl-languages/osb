
include ../make.inc

#
# NOTE: Define MPICC and OSHCC (as needed)
#       in the make.inc file.
#

all: shmem

oldall: graph500_mpi_simple graph500_mpi_one_sided graph500_mpi_replicated \
      graph500_mpi_replicated_csc graph500_mpi_custom graph500_shmem_one_sided

shmem: graph500_shmem_one_sided graph500_shmem_one_sided_er iodriver_shmem
# graph500_shmem_one_sided_ctx


GENERATOR_SOURCES = ../generator/graph_generator.c ../generator/make_graph.c ../generator/splittable_mrg.c ../generator/utils.c
SOURCES = main.c oned_csr.c oned_csc.c utils.c validate.c onesided.c onesided_emul.c
SHMEM_SOURCES =  oned_csr.c oned_csc.c utils.c validate.c onesided_shmem.c shmem_lib.c
HEADERS = common.h oned_csr.h oned_csc.h redistribute.h mpi_workarounds.h onesided.h
SHMEM_HEADERS = common.h oned_csr.h oned_csc.h redistribute.h mpi_workarounds.h onesided.h shmem_lib.h

#debug
oned_csr.i:
	$(OSHCC) $(CFLAGS) -E oned_csr.c > oned_csr.i
	indent oned_csr.i

validate.i:
	$(OSHCC) $(CFLAGS) -E validate.c > validate.i

graph500_mpi_simple: bfs_simple.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_mpi_simple bfs_simple.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

graph500_mpi_one_sided_lock: bfs_one_sided.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -DUSE_MPI_WIN_LOCK   -o graph500_mpi_one_sided_lock  bfs_one_sided.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

graph500_mpi_one_sided: bfs_one_sided.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_mpi_one_sided bfs_one_sided.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

graph500_mpi_replicated: bfs_replicated.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_mpi_replicated bfs_replicated.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

graph500_mpi_replicated_csc: bfs_replicated_csc.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_mpi_replicated_csc bfs_replicated_csc.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

graph500_mpi_custom: bfs_custom.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -o graph500_mpi_custom bfs_custom.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

# TJN: THIS IS THE THREADED VERSION OF GRAPH500-oshmem
graph500_shmem_one_sided: bfs_shmem.c main.c $(SHMEM_SOURCES)  $(SHMEM_HEADERS) $(GENERATOR_SOURCES)
	$(OSHCC) $(CFLAGS) $(LDFLAGS) -DUSE_OPENSHMEM -o graph500_shmem_one_sided bfs_shmem.c main.c $(SHMEM_SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)

# ###
# # TJN: If shmem_ctx_ available for use, then pass the `-DWITH_CTX=1`, 
#        otherwise, no reason to build the shmem 1sided ctx executable.
# ###
# graph500_shmem_one_sided_ctx: bfs_shmem.c main.c $(SHMEM_SOURCES)  $(SHMEM_HEADERS) $(GENERATOR_SOURCES)
# 	$(OSHCC) $(CFLAGS) $(LDFLAGS) -DWITH_CTX=1 -DUSE_OPENSHMEM -o graph500_shmem_one_sided_ctx bfs_shmem.c main.c $(SHMEM_SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)
# 

graph500_mpi_one_sided_lock_er: bfs_one_sided.c $(SOURCES) $(HEADERS) $(GENERATOR_SOURCES)
	$(MPICC) $(CFLAGS) $(LDFLAGS) -DUSE_ERDOS_RENYI -DUSE_MPI_WIN_LOCK   -o graph500_mpi_one_sided_lock_er  bfs_one_sided.c $(SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)


graph500_shmem_one_sided_er: bfs_shmem.c $(SHMEM_SOURCES)  $(SHMEM_HEADERS) $(GENERATOR_SOURCES)
	$(OSHCC) $(CFLAGS) -DUSE_OPENSHMEM -DUSE_ERDOS_RENYI $(LDFLAGS) -o graph500_shmem_one_sided_er bfs_shmem.c main.c $(SHMEM_SOURCES) $(GENERATOR_SOURCES) -lm $(LDLIBS)


iodriver_shmem: bfs_shmem.c $(SHMEM_SOURCES)  $(SHMEM_HEADERS) $(GENERATOR_SOURCES) iodriver.c read_edgelist.c read_edgelist.h
	$(OSHCC) $(CFLAGS) $(LDFLAGS) -DUSE_OPENSHMEM -I../generator -o iodriver_shmem  bfs_shmem.c $(SHMEM_SOURCES) $(GENERATOR_SOURCES) read_edgelist.c iodriver.c -lm $(LDLIBS)

clean:
	-rm -f graph500_mpi_one_sided *.o *.a graph500_shmem_one_sided graph500_shmem_one_sided_er iodriver_shmem 
	-rm -f graph500_shmem_one_sided_ctx
	-rm -f graph500_mpi_custom graph500_mpi_replicated graph500_mpi_replicated_csc graph500_mpi_simple
