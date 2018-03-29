# setup for UCCS backend for OpenSHMEM

module unload mpt

export MODULEPATH='/ccs/openshmem/openshmem-dev/modulefiles:'"$LD_LIBRARY_PATH"
# module load openshmem-gasnet
# module load openshmem-uccs-ext
module load openshmem-uccs

# use oshcc, run with either oshrun for GASNET or orterun for UCCS
# or wrapper at /ccs/openshmem/openshmem-dev/bin/allrun
# wrapper support -n/-np and -hostfile/--hostfile
# remaining options passed to oshrun/orterun/mpirun

export PATH=$PATH:/ccs/openshmem/openshmem-dev/bin/

#module load /ccs/openshmem/openshmem/modulefiles/openshmem
export GASNET_PSHM_NODES=1
export SHMEM_SYMMETRIC_HEAP_SIZE=1024M



./configure \
   CC=oshcc \
   CFLAGS=" -g " 


#   CFLAGS=" -O3 " 

