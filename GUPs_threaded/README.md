 Copyright (c) 2011 - 2015 University of Houston System and UT-Battelle, LLC.
 Copyright (C) 2011-2018, UT-Battelle, LLC.
  All rights reserved.
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  o Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  o Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

  o Neither the name of the University of Houston System,
    UT-Battelle, LLC. nor the names of its contributors may be used to
    endorse or promote products derived from this software without specific
    prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# ---------------------------------------------------
This is a modified version of the Ramdom Access Benchmark initially developed 
by David Koester and Bob Lucas.

Basic requirements of the RandomAccess benchmark are:
 (1) size of the table T[ ] being a power of two and approximately half the global memory and with
 (2) look-ahead and storage constraints being followed. Specifically, attempts to improve memory 
 access characteristics by reordering the address stream -- so that all updates are "local" -- 
 violates the rules of the RandomAccess benchmark.

In the OpenSHMEM version the entire HPCC_Table is allocated via shmem_malloc and 
all update locations are ensured to be both random and remote.
For more information please contact the developers at: pophaless@ornl

To compile:
------------
* In the Makefile set CC to the correct compiler. Default is set to oshcc.
* Type 'make' in the main folder
* Executable produced is 'gups'

Sample results:
---------------
* On Intel® Xeon® E5 Cluster -2660 processors with 10 physical cores and hyper threading,
a Mellanox ConnectX-4 VPI adapter card, EDR IB (100 Gb/s) and 12 GB RAM.

oshrun -np 32 ./gups

Running on 32 processors
Total Main table size = 2^26 = 67108864 words
PE Main table size = (2^26)/32  = 2097152 words/PE MAX
Default number of updates (RECOMMENDED) = 268435456	and actually done = 268435456
Real time used = 33.717555 seconds
0.007961297 Billion(10^9) Updates    per second [GUP/s]
0.000248791 Billion(10^9) Updates/PE per second [GUP/s]
