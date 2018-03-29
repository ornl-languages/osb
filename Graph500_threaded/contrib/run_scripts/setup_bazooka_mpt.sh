
module unload mpt
module unload /ccs/openshmem/openshmem/modulefiles/openshmem

module load mpt
export GASNET_PSHM_NODES=1
export SHMEM_SYMMETRIC_HEAP_SIZE=1024M


