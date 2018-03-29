
module unload PrgEnv-intel
module unload PrgEnv-gnu
module unload PrgEnv-pgi
module unload PrgEnv-cray

module unload cray-mpich2
module unload cray-shem
module unload perftools
module unload cray-tpsl

module load PrgEnv-cray
module unload cray-mpich2

#module load cray-shmem/7.2.0
module load cray-shmem
module load perftools
module load cray-tpsl

export SHMEM_SYMMETRIC_HEAP_SIZE=1000000000
export XT_SYMMETRIC_HEAP_SIZE=1G

export SHORT_VALIDATION=1


