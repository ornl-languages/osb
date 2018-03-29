source ../setup_modules_cray.sh

module unload craype-hugepages2M 
module unload craype-hugepages4M 
module unload craype-hugepages8M 
module unload craype-hugepages16M 
module unload craype-hugepages64M 

make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_4K
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_4K


module load craype-hugepages2M
make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_2M
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_2M
module unload craype-hugepages2M



module load craype-hugepages4M
make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_4M
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_4M
module unload craype-hugepages4M


module load craype-hugepages8M
make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_8M
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_8M
module unload craype-hugepages8M

module load craype-hugepages16M
make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_16M
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_16M
module unload craype-hugepages16M

module load craype-hugepages64M
make clean; make graph500_shmem_one_sided;
mv graph500_shmem_one_sided graph500_shmem_one_sided_64M
make clean; make graph500_mpi_one_sided
mv graph500_mpi_one_sided graph500_mpi_one_sided_64M
module unload craype-hugepages64M

