source setup_cray_shmem.sh
touch graph500_shmem_one_sided
rm  graph500_shmem_one_sided
make clean; make graph500_shmem_one_sided
cp graph500_shmem_one_sided /tmp

source setup_cray_mpich.sh
touch graph500_mpi_one_sided
rm graph500_mpi_one_sided
make clean; make graph500_mpi_one_sided
cp graph500_mpi_one_sided /tmp

source setup_cray_mpich.sh
touch graph500_mpi_simple
rm graph500_mpi_simple
make clean; make graph500_mpi_simple
cp graph500_mpi_simple /tmp


cp /tmp/graph500_mpi_simple .
cp /tmp/graph500_shmem_one_sided .
cp /tmp/graph500_mpi_one_sided .

