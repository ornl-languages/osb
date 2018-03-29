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


alias oshc='oshcc -fopenmp -g -std=c99 -Wall -DUSE_OPENSHMEM '
# oshrun  --verbose -np 2 -display-allocation --debug --debugger /usr/bin/gdb ./graph500_shmem_one_sided 10
export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1

alias debug2='oshrun --verbose -np 2 --mca sshmem mmap xterm -e gdb ./graph500_shmem_one_sided '
alias myrun='oshrun --verbose -np 1 --mca sshmem mmap ./graph500_shmem_one_sided 14 14 '
alias myrun2='oshrun --verbose -np 2 --mca sshmem mmap ./graph500_shmem_one_sided 14 14 '
alias myrun4='oshrun --verbose -np 4 --mca sshmem mmap ./graph500_shmem_one_sided 14 14 '
alias simple4='oshrun --verbose -np 4 --mca sshmem mmap ./graph500_mpi_simple 14 14 '
alias simple2='oshrun --verbose -np 2 --mca sshmem mmap ./graph500_mpi_simple 14 14 '
alias myrunv='oshrun --np 1 --mca sshmem mmap valgrind --leak-check=full --show-leak-kinds=all  ./graph500_shmem_one_sided 12 12 '
