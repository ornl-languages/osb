export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
export TMPFILE='/tmp/tmpfile'
export REUSEFILE=0
export VERBOSE=1

make clean
make make-edgelist
make CC=oshcc make-edgelist-shmem

alias run4='oshrun --verbose -np 4 --mca sshmem mmap ./make-edgelist-shmem -o /tmp/file4 -V -s 14 -e 16 '
alias run2='oshrun --verbose -np 2 --mca sshmem mmap ./make-edgelist-shmem -o /tmp/file2 -V -s 14 -e 16 '
alias run1='oshrun --verbose -np 1 --mca sshmem mmap ./make-edgelist-shmem -o /tmp/file1 -V -s 14 -e 16 '

