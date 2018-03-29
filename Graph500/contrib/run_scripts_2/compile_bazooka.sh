make -f Makefile clean
make -f Makefile graph500_shmem_one_sided 
mv graph500_shmem_one_sided /tmp
make -f Makefile clean
make -f Makefile graph500_mpi_one_sided 
mv graph500_mpi_one_sided /tmp
make -f Makefile clean
make -f Makefile graph500_mpi_simple 
mv graph500_mpi_simple /tmp

mv /tmp/graph500_shmem_one_sided .
mv /tmp/graph500_mpi_simple .
mv /tmp/graph500_mpi_one_sided .

export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=8M
unset TMPFILE
unset REUSEFILE

