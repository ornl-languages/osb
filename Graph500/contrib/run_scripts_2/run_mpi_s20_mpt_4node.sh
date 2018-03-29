#!/bin/bash
# File name: run_mpi_s20_mpt_4node.pbs
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

module load mpt
PBS_JOBID=$$

mkdir mpi_s20_mpt_4node_$PBS_JOBID
cd mpi_s20_mpt_4node_$PBS_JOBID


export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1

export XT_SYMMETRIC_HEAP_SIZE=1G
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka02,bazooka03,bazooka04,bazooka05  24  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p24

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  16  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p16

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  12  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  8  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p8

export XT_SYMMETRIC_HEAP_SIZE=8G
export SHMEM_SYMMETRIC_HEAP_SIZE=8000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  4  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p4

export XT_SYMMETRIC_HEAP_SIZE=16G
export SHMEM_SYMMETRIC_HEAP_SIZE=16000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  2  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p2

export XT_SYMMETRIC_HEAP_SIZE=24G
export SHMEM_SYMMETRIC_HEAP_SIZE=24000000000
mpirun   bazooka02,bazooka03,bazooka04,bazooka05  1  ../graph500_mpi_one_sided  20 16 >& mpt_mpi_s20.p1
