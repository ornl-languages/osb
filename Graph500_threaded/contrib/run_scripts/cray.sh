module unload PrgEnv-intel
module unload PrgEnv-gnu
module unload PrgEnv-pgi
module unload PrgEnv-cray
module unload cray-shmem

module load PrgEnv-cray
module load cray-shmem

export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export XT_SYMMETRIC_HEAP_SIZE=$SHMEM_SYMMETRIC_HEAP_SIZE

export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
export TMPFILE='/tmp/tmpfile'
export REUSEFILE=0
export VERBOSE=1

make clean
make CC=cc make-edgelist
make CC=cc make-edgelist-shmem

alias run16='aprun -n 16  ./make-edgelist-shmem -o ./file16 -V -s 14 -e 14 '
alias run8='aprun -n 8  ./make-edgelist-shmem -o ./file8 -V -s 14 -e 14 '
alias run4='aprun -n 4  ./make-edgelist-shmem -o ./file4 -V -s 14 -e 14 '
alias run2='aprun -n 2 ./make-edgelist-shmem -o ./file2 -V -s 14 -e 14 '
alias run1='aprun -n 1 ./make-edgelist-shmem -o ./file1 -V -s 14 -e 14 '

