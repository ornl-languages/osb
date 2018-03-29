#!/bin/bash
# File name: run_s26_mpt_4node.sh
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

#cd $PROJWORK/csc040/dazevedo/graph500_openshmem/mpi
cd /ccs/home/efdazedo/WORK/Graph/Graph500/graph500_openshmem/mpi

PBS_JOBID=$$
mkdir s26_mpt_4node_$PBS_JOBID
cd s26_mpt_4node_$PBS_JOBID


export OMP_NUM_THREADS=1
export XT_SYMMETRIC_HEAP_SIZE=6G

export SHMEM_SYMMETRIC_HEAP_SIZE=6000000000
export OMP_NUM_THREADS=1
export OMP_STACKSIZE=4M
unset TMPFILE
unset REUSEFILE
# export TMPFILE='/tmp/tmpfile'
# export REUSEFILE=0
export VERBOSE=1

mpirun  bazooka02,bazooka03,bazooka04,bazooka05  1   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p1
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  2   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p2
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  4   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p4
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  8   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p8
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  12   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p12
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  16   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p16
mpirun  bazooka02,bazooka03,bazooka04,bazooka05  24   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p24
# mpirun -np 32   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p32
# mpirun -np 48   ../graph500_shmem_one_sided 26 16 >& mpt_s26.p48
