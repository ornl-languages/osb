
module unload PrgEnv-cray
module unload PrgEnv-intel
module unload PrgEnv-gnu
module unload PrgEnv-pgi


module load PrgEnv-cray
module unload cray-mpich2
module load cray-shmem
module load cray-tpsl





export OMP_NUM_THREADS=1
export XT_SYMMETRIC_HEAP_SIZE=1G

export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1


aprun -n 1 ./graph500_mpi_one_sided 16 16 >& cray_mpi_s16.p1
aprun -n 2 ./graph500_mpi_one_sided 16 16 >& cray_mpi_s16.p2
aprun -n 4 ./graph500_mpi_one_sided 16 16 >& cray_mpi_s16.p4
aprun -n 8 ./graph500_mpi_one_sided 16 16 >& cray_mpi_s16.p8
aprun -n 16 ./graph500_mpi_one_sided 16 16 >& cray_mpi_s16.p16
