#!/bin/bash
# File name: run_mpi_s26_mpt_8node.pbs
#PBS -A csc040
#PBS -l walltime=4:00:00
#PBS -l nodes=16

source $MODULESHOME/init/bash
module load mpt
PBS_JOBID=$$

rundir=runall_s26_mpt_8node_$PBS_JOBID
mkdir $rundir
cd $rundir


export MPI_REQUEST_MAX=1048575
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

export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_shmem_one_sided  26 16 >& mpt_s26.p24
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_simple  26 16 >& mpt_simple_s26.p24
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26.p24

export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_shmem_one_sided  26 16 >& mpt_s26.p16
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_simple  26 16 >& mpt_simple_s26.p16
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26.p16

export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_shmem_one_sided  26 16 >& mpt_s26.p12
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_mpi_simple  26 16 >& mpt_simple_s26.p12
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_shmem_one_sided  26 16 >& mpt_s26.p8
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_mpi_simple  26 16 >& mpt_simple_s26.p8
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26.p8



# ----------------
unset SKIP_VALIDATION
export SHORT_VALIDATION=1
# ----------------


export XT_SYMMETRIC_HEAP_SIZE=1G
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_shmem_one_sided  26 16 >& mpt_s26_teps.p24
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_simple  26 16 >& mpt_simple_s26_teps.p24
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  24  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26_teps.p24

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_shmem_one_sided  26 16 >& mpt_s26_teps.p16
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_simple  26 16 >& mpt_simple_s26_teps.p16
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  16  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26_teps.p16

export XT_SYMMETRIC_HEAP_SIZE=2G
export SHMEM_SYMMETRIC_HEAP_SIZE=2000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_shmem_one_sided  26 16 >& mpt_s26_teps.p12
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_mpi_simple  26 16 >& mpt_simple_s26_teps.p12
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  12  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26_teps.p12

export XT_SYMMETRIC_HEAP_SIZE=4G
export SHMEM_SYMMETRIC_HEAP_SIZE=4000000000
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_shmem_one_sided  26 16 >& mpt_s26_teps.p8
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_mpi_simple  26 16 >& mpt_simple_s26_teps.p8
mpirun   bazooka02,bazooka03,bazooka10,bazooka11,bazooka06,bazooka07,bazooka08,bazooka09  8  ../graph500_mpi_one_sided  26 16 >& mpt_one_sided_s26_teps.p8

