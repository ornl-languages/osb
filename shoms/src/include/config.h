/*
   This file is part of SHOMS.

   Copyright (C) 2014-2105, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SHOMS developers at:
   bakermb@ornl.gov

*/

#define SHOMS_VERSION_STRING "0.01"
#define MINSIZE 8
#define MAXSIZE 16777216
#define WARMUP_RUN_COUNT 10

//Note: Current OpenShmem implmentations only support 1 thread offically.
#define NUMCORES 1
#define USE_UINT64_INDEX

#define TIMER_X86_64
