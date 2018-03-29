export  SHMEM_SYMMETRIC_HEAP_SIZE=256M
module unload mpt/2.03

module load gasnet

export PATH=/ccs/openshmem/openshmem/bin:$PATH
export PATH=/ccs/openshmem/openshmem/gasnet/fast/bin:$PATH

export LD_LIBRARY_PATH=/ccs/openshmem/openshmem/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/ccs/openshmem/openshmem/lib/modules:$LD_LIBRARY_PATH

./configure \
   CC=oshcc \
   CFLAGS=" -I/ccs/openshmem/openshmem/include -L/ccs/openshmem/openshmem/lib -L/ccs/openshmem/openshmem/gasnet/fast/lib"

