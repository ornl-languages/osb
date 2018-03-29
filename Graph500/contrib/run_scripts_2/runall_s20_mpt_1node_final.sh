#!/bin/bash
# File name: run_mpi_s20_mpt_1node.pbs
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

source $MODULESHOME/init/bash
module load mpt
PBS_JOBID=$$

rundir=runall_s20_mpt_1node_final_$PBS_JOBID
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
export SKIP_VALIDATION=1
unset SHORT_VALIDATION
# ----------------

export XT_SYMMETRIC_HEAP_SIZE=1G
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka04  24  ../graph500_shmem_one_sided  20 16 >& mpt_s20.p24
mpirun   bazooka04  24  ../graph500_mpi_simple  20 16 >& mpt_simple_s20.p24
mpirun   bazooka04  24  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20.p24


export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka04  16  ../graph500_shmem_one_sided  20 16 >& mpt_s20.p16
mpirun   bazooka04  16  ../graph500_mpi_simple  20 16 >& mpt_simple_s20.p16
mpirun   bazooka04  16  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20.p16

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka04  12  ../graph500_shmem_one_sided  20 16 >& mpt_s20.p12
mpirun   bazooka04  12  ../graph500_mpi_simple  20 16 >& mpt_simple_s20.p12
mpirun   bazooka04  12  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka04  8  ../graph500_shmem_one_sided  20 16 >& mpt_s20.p8
mpirun   bazooka04  8  ../graph500_mpi_simple  20 16 >& mpt_simple_s20.p8
mpirun   bazooka04  8  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20.p8



# ----------------
unset SKIP_VALIDATION
export SHORT_VALIDATION=1
# ----------------


export XT_SYMMETRIC_HEAP_SIZE=1G
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka04  24  ../graph500_shmem_one_sided  20 16 >& mpt_s20_teps.p24
mpirun   bazooka04  24  ../graph500_mpi_simple  20 16 >& mpt_simple_s20_teps.p24
mpirun   bazooka04  24  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20_teps.p24

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka04  16  ../graph500_shmem_one_sided  20 16 >& mpt_s20_teps.p16
mpirun   bazooka04  16  ../graph500_mpi_simple  20 16 >& mpt_simple_s20_teps.p16
mpirun   bazooka04  16  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20_teps.p16

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka04  12  ../graph500_shmem_one_sided  20 16 >& mpt_s20_teps.p12
mpirun   bazooka04  12  ../graph500_mpi_simple  20 16 >& mpt_simple_s20_teps.p12
mpirun   bazooka04  12  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20_teps.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka04  8  ../graph500_shmem_one_sided  20 16 >& mpt_s20_teps.p8
mpirun   bazooka04  8  ../graph500_mpi_simple  20 16 >& mpt_simple_s20_teps.p8
mpirun   bazooka04  8  ../graph500_mpi_one_sided  20 16 >& mpt_one_sided_s20_teps.p8

