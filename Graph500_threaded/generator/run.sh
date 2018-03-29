make -f Makefile.shmem clean
make -f Makefile.shmem 
make -f Makefile.mpi clean
make -f Makefile.mpi 

# oshrun  --verbose -np 2 -display-allocation --debug --debugger /usr/bin/gdb ./graph500_shmem_one_sided 10
export SHMEM_SYMMETRIC_HEAP_SIZE=64000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
export TMPFILE='/tmp/tmpfile'
export REUSEFILE=0
# export TMPFILE=
# export REUSEFILE
export VERBOSE=1

alias myrun='oshrun --verbose -np 1 --mca sshmem mmap ./generator_test_shmem '
echo "" > mpi.out

echo "mpirun --mca sshmem mmap -np 1 ./generator_test_mpi "  >> mpi.out
mpirun --mca sshmem mmap -np 1 ./generator_test_mpi &>> mpi.out
echo "mpirun --mca sshmem mmap -np 4 ./generator_test_mpi "  >> mpi.out
mpirun --mca sshmem mmap -np 4 ./generator_test_mpi &>> mpi.out
echo "mpirun --mca sshmem mmap -np 8 ./generator_test_mpi "  >> mpi.out
mpirun --mca sshmem mmap -np 8 ./generator_test_mpi &>> mpi.out

echo "" > shmem.out
echo "oshrun --verbose -np 1 --mca sshmem mmap ./generator_test_shmem "  >> shmem.out
oshrun --verbose -np 1 --mca sshmem mmap ./generator_test_shmem  &>> shmem.out
echo "oshrun --verbose -np 4 --mca sshmem mmap ./generator_test_shmem "  >> shmem.out
oshrun --verbose -np 4 --mca sshmem mmap ./generator_test_shmem  &>> shmem.out
echo "oshrun --verbose -np 8 --mca sshmem mmap ./generator_test_shmem "  >> shmem.out
oshrun --verbose -np 8 --mca sshmem mmap ./generator_test_shmem  &>> shmem.out
