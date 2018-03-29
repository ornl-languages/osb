#!/bin/bash
# File name: debug_1node.sh
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

source $MODULESHOME/init/bash
module load mpt
PBS_JOBID=$$

rundir=debug_1node_$PBS_JOBID
mkdir $rundir
cd $rundir


export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1



# ----------------
unset SKIP_VALIDATION
export SHORT_VALIDATION=1
# ----------------

export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000

# mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_shmem_one_sided  26 16 >& mpt_s26.p24
# mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_simple  26 16 >& mpt_mpi_s26.p24


touch bfs_output.txt simple_graph.txt
rm bfs_output.txt simple_graph.txt
mpirun   bazooka02 16  ../graph500_mpi_simple  20 16 >& mpt_simple.out
touch bfs_output.txt
fsync bfs_output.txt
mv bfs_output.txt  bfs_simple.txt
fsync bfs_simple.txt
sync
sleep 1

 
touch bfs_output.txt mpi_graph.txt
rm bfs_output.txt mpi_graph.txt
mpirun   bazooka02 16  ../graph500_mpi_one_sided  20 16 >& mpt_mpi.out 
touch bfs_output.txt
fsync bfs_output.txt
mv bfs_output.txt bfs_mpi.txt
touch bfs_output.txt
fsync bfs_mpi.txt
sync
sleep 1

touch bfs_output.txt shmem_graph.txt
rm bfs_output.txt shmem_graph.txt
mpirun   bazooka02 16  ../graph500_shmem_one_sided  20 16 >& mpt_shmem.out 
touch bfs_output.txt
fsync bfs_output.txt
mv bfs_output.txt bfs_shmem.txt
fsync bfs_shmem.txt
sync
sleep 1

export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
# mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_shmem_one_sided  20 16 >& mpt_s20.p16
#  mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_simple  20 16 >& mpt_mpi_s20.p16

