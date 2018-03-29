module unload PrgEnv-intel
module unload PrgEnv-gnu
module unload PrgEnv-pgi
module unload PrgEnv-cray
module unload cray-shmem

module unload perftools
module unload cray-tpsl

module load PrgEnv-cray
module unload cray-mpich2

module load cray-shmem
# module load perftools
module load cray-tpsl

export XT_SYMMETRIC_HEAP_SIZE=1000M
export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000

export SHORT_VALIDATION=1

