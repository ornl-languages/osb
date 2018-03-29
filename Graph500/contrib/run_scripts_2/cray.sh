module unload PrgEnv-intel
module unload PrgEnv-gnu
module unload PrgEnv-pgi
module unload PrgEnv-cray
module unload cray-shmem

module unload git
module unload perftools
module unload cray-shmem

module load PrgEnv-cray
#module load perftools
module load cray-shmem
module load git

make CC=cc -f Makefile.cray clean
make CC=cc -f Makefile.cray  graph500_shmem_one_sided 
make CC=cc -f Makefile.cray  graph500_mpi_simple 
make CC=cc -f Makefile.cray  graph500_mpi_one_sided 



alias oshc='cc -h omp  -DUSE_OPENSHMEM '
# oshrun  --verbose -np 2 -display-allocation --debug --debugger /usr/bin/gdb ./graph500_shmem_one_sided 10
export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export XT_SYMMETRIC_HEAP_SIZE=$SHMEM_SYMMETRIC_HEAP_SIZE
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4000000
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1

alias myrun='aprun -n 1  ./graph500_shmem_one_sided 14 14 '
alias myrun2='aprun -n 2  ./graph500_shmem_one_sided 14 14 '
alias myrun4='aprun -n 4  ./graph500_shmem_one_sided 14 14 '
alias myrun8='aprun -n 8  ./graph500_shmem_one_sided 14 14 '
alias myrun16='aprun -n 16  ./graph500_shmem_one_sided 14 14 '

