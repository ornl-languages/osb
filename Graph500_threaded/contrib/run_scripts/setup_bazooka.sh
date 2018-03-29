export  SHMEM_SYMMETRIC_HEAP_SIZE=256M
module unload mpt/2.03
module unload openshmem

module load openmpi
module load gasnet
module load  openshmem/1.0e-gcc-ibv

# module load /ccs/openshmem/openshmem/modulefiles/openshmem

# export PATH=/ccs/openshmem/openshmem/bin:/ccs/openshmem/openshmem/gasnet/fast/bin:$PATH

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/ccs/proj/csc040/software/rhel6-x86_64/openshmem-1.0e-gcc-ibv/lib

./configure CC=oshcc CFLAGS="-std=gnu99 -O3 -I /ccs/proj/csc040/software/rhel6-x86_64/openshmem-1.0e-gcc-ibv/include "

