#   This file is part of SHOMS.
#
#   Copyright (C) 2014-2018, UT-Battelle, LLC.
#
#   This product includes software produced by UT-Battelle, LLC under Contract No.
#   DE-AC05-00OR22725 with the Department of Energy.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the New BSD 3-clause software license (LICENSE).
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#   LICENSE for more details.
#
#   For more information please contact the SHOMS developers at:
#   bakermb@ornl.gov

SHMEM12 = 1

OSHCC=oshcc
OSHLD=$(OSHCC)
CC=gcc
CPPFLAGS=-Isrc/include/ #-DDEBUG #-DNDEBUG
CFLAGS=-g -std=c99 -Os -Wall
LDFLAGS=
LIBS=-lm

OBJECTS=src/main.o src/test_list.o src/tests.o src/orbtimer.o src/process_parameters.o

ifneq ($(SHMEM12), 0)
	CFLAGS += -DUSE_SHMEM12
endif

.c.o:
	$(OSHCC) -c $(CPPFLAGS) $(CFLAGS) $(OSHFLAGS) -o $@ $<

all: shoms

bin:
	mkdir bin

src/main.o: src/include/shoms.h
src/test_list.o: src/include/shoms.h
src/tests.o: src/include/shoms.h
src/process_parameters.o: src/include/shoms.h

shoms: $(OBJECTS) bin
	$(OSHLD) $(LDFLAGS) -o bin/shoms $(OBJECTS) $(LIBS)

clean:
	rm -f $(OBJECTS) bin/shoms
