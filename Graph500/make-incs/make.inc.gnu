# -*- Makefile -*-
# Copyright 2010, Georgia Institute of Technology, USA.
# See COPYING for license.
CFLAGS = -g -std=c99 -Wall  -Impi -Igenerator -I. 
LDLIBS = -lm -lrt
#CPPFLAGS = -DUSE_OPENSHMEM -UUSE_MMAP_LARGE -UUSE_MMAP_LARGE_EXT
CPPFLAGS = -DUSE_OPENSHMEM

OSHCC = oshcc

BUILD_OPENMP = No
BUILD_MPI = No
BUILD_OPENSHMEM = Yes
