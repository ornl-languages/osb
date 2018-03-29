#!/bin/bash
# File name: run_mpi_s26_mpt_8node.pbs
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

module load mpt
PBS_JOBID=$$

mkdir mpi_s26_mpt_8node_$PBS_JOBID
cd mpi_s26_mpt_8node_$PBS_JOBID


export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1
# export SKIP_VALIDATION=1

export XT_SYMMETRIC_HEAP_SIZE=1G
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p24

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p16

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p8

export XT_SYMMETRIC_HEAP_SIZE=8G
export SHMEM_SYMMETRIC_HEAP_SIZE=8000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  4  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p4

export XT_SYMMETRIC_HEAP_SIZE=16G
export SHMEM_SYMMETRIC_HEAP_SIZE=16000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  2  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p2

export XT_SYMMETRIC_HEAP_SIZE=24G
export SHMEM_SYMMETRIC_HEAP_SIZE=24000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05,bazooka06,bazooka07,bazooka08,bazooka09  1  ../graph500_mpi_one_sided  26 16 >& mpt_mpi_s26.p1
