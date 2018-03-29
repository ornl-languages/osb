#!/bin/bash
# File name: run_huge_page_mpi.pbs
#PBS -A csc040
#PBS -l walltime=1:00:00
#PBS -l nodes=16

cd $PROJWORK/csc040/dazevedo/graph500_openshmem/mpi

source $MODULESHOME/init/bash

source ../setup_modules_cray.sh



mkdir hugepage_$PBS_JOBID
cd hugepage_$PBS_JOBID

export HUGETLB_VERBOSE=3

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

module unload craype-hugepages2M 
module unload craype-hugepages4M 
module unload craype-hugepages8M 
module unload craype-hugepages16M 

#aprun -n 256 -d1  ../graph500_shmem_one_sided_4K 14 16 >& cray_s14.p256.4K
aprun -n 256 -d 1 ../graph500_mpi_one_sided_4K  14 16 >& cray_mpi_s14.p256.4K




module unload craype-hugepages2M 
module load craype-hugepages2M
#aprun -n 256 -d1  ../graph500_shmem_one_sided_2M 14 16 >& cray_s14.p256.2M
aprun -n 256 -d 1 ../graph500_mpi_one_sided_2M  14 16 >& cray_mpi_s14.p256.2M
module unload craype-hugepages2M 


module unload craype-hugepages4M 
module load craype-hugepages4M
#aprun -n 256 -d1  ../graph500_shmem_one_sided_4M 14 16 >& cray_s14.p256.4M
aprun -n 256 -d 1 ../graph500_mpi_one_sided_4M  14 16 >& cray_mpi_s14.p256.4M
module unload craype-hugepages4M 


module unload craype-hugepages8M 
module load craype-hugepages8M
#aprun -n 256 -d1  ../graph500_shmem_one_sided_8M 14 16 >& cray_s14.p256.8M
aprun -n 256 -d 1 ../graph500_mpi_one_sided_8M  14 16 >& cray_mpi_s14.p256.8M
module unload craype-hugepages8M 


module unload craype-hugepages16M 
module load craype-hugepages16M
#aprun -n 256 -d1  ../graph500_shmem_one_sided_16M 14 16 >& cray_s14.p256.16M
aprun -n 256 -d 1 ../graph500_mpi_one_sided_16M  14 16 >& cray_mpi_s14.p256.16M
module unload craype-hugepages16M 





