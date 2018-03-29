

export MODULEPATH=${MODULEPATH}:/ccs/openshmem/openshmem-dev/modulefiles:/ccs/proj/csc040/software/rhel6-x86_64/modulefiles
module unload openshmem
module unload openshmem-uccs
module load mpt
#module load openshmem-uccs
module load autoconf/2.65
module load automake/1.14.1
CC=cc  CFLAGS="-DUSE_STATIC_ARRAY=1 -DMPFR_DEFAULT_PRECISION=256 -DUSE_GSL=1 -O3 -I/opt/sgi/mpt/mpt-2.03/include -fPIC -Wall -std=c99  -lgslcblas -lgsl -lm"  \
./configure --prefix=/tmp/foo --with-gmp=/ccs/proj/csc040/software/rhel6-x86_64/gmp-5.1.3 --disable-shared --with-mpfr=/ccs/proj/csc040/software/sles11-x86_64/mpfr-3.1.2 --enable-comms=openshmem --enable-comms=openshmem
make clean
make all
make install
