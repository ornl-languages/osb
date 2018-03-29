#!/bin/bash
# File name: run_s20_mpt.sh
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

#cd $PROJWORK/csc040/dazevedo/graph500_openshmem/mpi
cd /ccs/home/efdazedo/WORK/Graph/Graph500/graph500_openshmem/mpi

PBS_JOBID=$$
mkdir s20_mpt_$PBS_JOBID
cd s20_mpt_$PBS_JOBID


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

mpirun -np 1   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p1
mpirun -np 2   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p2
mpirun -np 4   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p4
mpirun -np 8   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p8
mpirun -np 12   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p12
mpirun -np 16   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p16
mpirun -np 24   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p24
# mpirun -np 32   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p32
# mpirun -np 48   ../graph500_shmem_one_sided 20 16 >& mpt_s20.p48
