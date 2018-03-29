#!/bin/bash

WORK_SIZE=16

for i in 1 2 4 8 16 32 
do
    echo "threaded run (scale=${WORK_SIZE}) threads=${i}"
    date
    orterun -np 2 -x OMP_NUM_THREADS=${i} -x UCX_LOG_LEVEL=error -H turing6,turing7 --map-by node ./graph500_shmem_one_sided ${WORK_SIZE}
done
