oshcc -o tortoall -std=c99 -Wall tortoall.c
export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1


echo "np = 4 "
oshrun --verbose -np 4 --mca sshmem mmap ./tortoall
echo "np = 5 "
oshrun --verbose -np 5 --mca sshmem mmap ./tortoall

