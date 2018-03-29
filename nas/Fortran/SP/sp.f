!-------------------------------------------------------------------------!
!                                                                         !
!        N  A  S     P A R A L L E L     B E N C H M A R K S  3.2         !
!                                                                         !
!                                   S P                                   !
!                                                                         !
!-------------------------------------------------------------------------!
!                                                                         !
!    This benchmark is part of the NAS Parallel Benchmark 3.2 suite.      !
!    It is described in NAS Technical Reports 95-020 and 02-007           !
!                                                                         !
!    Permission to use, copy, distribute and modify this software         !
!    for any purpose with or without fee is hereby granted.  We           !
!    request, however, that all derived work reference the NAS            !
!    Parallel Benchmarks 3.2. This software is provided "as is"           !
!    without express or implied warranty.                                 !
!                                                                         !
!    Information on NPB 3.2, including the technical report, the          !
!    original specifications, source code, results and information        !
!    on how to submit new results, is available at:                       !
!                                                                         !
!           http://www.nas.nasa.gov/Software/NPB/                     !
!                                                                         !
!    Send comments or suggestions to  npb@nas.nasa.gov                    !
!                                                                         !
!          NAS Parallel Benchmarks Group                                  !
!          NASA Ames Research Center                                      !
!          Mail Stop: T27A-1                                              !
!          Moffett Field, CA   94035-1000                                 !
!                                                                         !
!          E-mail:  npb@nas.nasa.gov                                      !
!          Fax:     (650) 604-3957                                        !
!                                                                         !
!-------------------------------------------------------------------------!


c---------------------------------------------------------------------
c
c Authors: R. F. Van der Wijngaart
c          W. Saphir
c---------------------------------------------------------------------


c---------------------------------------------------------------------------
c
c  Copyright (c) 2011 - 2014
c    University of Houston System and Oak Ridge National Laboratory.
c
c  All rights reserved.
c
c  Redistribution and use in source and binary forms, with or without
c  modification, are permitted provided that the following conditions
c  are met:
c
c  o Redistributions of source code must retain the above copyright notice,
c    this list of conditions and the following disclaimer.
c
c  o Redistributions in binary form must reproduce the above copyright
c    notice, this list of conditions and the following disclaimer in the
c    documentation and/or other materials provided with the distribution.
c
c  o Neither the name of the University of Houston System, Oak Ridge
c    National Laboratory nor the names of its contributors may be used to
c    endorse or promote products derived from this software without specific
c    prior written permission.
c
c  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
c  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
c  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
c  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
c  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
c  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
c  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
c  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
c  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
c  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
c  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
c
c--------------------------------------------------------------------------

c---------------------------------------------------------------------
       program MPSP
c---------------------------------------------------------------------

       include  'header.h'
       include 'mpp/shmem.fh'

       integer          i, step, c, fstatus
       integer, save::niter

       external timer_read
       double precision mflops, timer_read
       double precision, save::t
       double precision, save::tmax

       logical          verified
       character        class
       double precision, save:: t1(t_last+2)
       double precision, save:: tsum(t_last+2)
       double precision, save:: tming(t_last+2)
       double precision, save:: tmaxg(t_last+2)
       character        t_recs(t_last+2)*8

       data t_recs/'total', 'rhs', 'xsolve', 'ysolve', 'zsolve', 
     >             'bpack', 'exch', 'xcomm', 'ycomm', 'zcomm',
     >             ' totcomp', ' totcomm'/

      integer, dimension(SHMEM_REDUCE_SYNC_SIZE), save :: psync
      integer, dimension(SHMEM_BCAST_SYNC_SIZE), save :: psync1
      integer, dimension(SHMEM_BCAST_SYNC_SIZE), save :: psync2
      double precision, dimension(SHMEM_REDUCE_MIN_WRKDATA_SIZE),
     > save :: pwrk

       call setup_rma

       psync = SHMEM_SYNC_VALUE
       psync1 = SHMEM_SYNC_VALUE
       psync2 = SHMEM_SYNC_VALUE
       call shmem_barrier_all()

       if (.not. active) goto 999

c---------------------------------------------------------------------
c      Root node reads input file (if it exists) else takes
c      defaults from parameters
c---------------------------------------------------------------------
       if (node .eq. root) then

          write(*, 1000)

          open (unit=2,file='timer.flag',status='old',iostat=fstatus)
          timeron = .false.
          if (fstatus .eq. 0) then
             timeron = .true.
             close(2)
          endif

          open (unit=2,file='inputsp.data',status='old', iostat=fstatus)
c
          if (fstatus .eq. 0) then
            write(*,233) 
 233        format(' Reading from input file inputsp.data')
            read (2,*) niter
            read (2,*) dt
            read (2,*) grid_points(1), grid_points(2), grid_points(3)
            close(2)
          else
            write(*,234) 
            niter = niter_default
            dt    = dt_default
            grid_points(1) = problem_size
            grid_points(2) = problem_size
            grid_points(3) = problem_size
          endif
 234      format(' No input file inputsp.data. Using compiled defaults')

          write(*, 1001) grid_points(1), grid_points(2), grid_points(3)
          write(*, 1002) niter, dt
          if (no_nodes .ne. total_nodes) write(*, 1004) total_nodes
          if (no_nodes .ne. maxcells*maxcells) 
     >        write(*, 1005) maxcells*maxcells
          write(*, 1003) no_nodes

 1000 format(//,' NAS Parallel Benchmarks 3.3 -- SP Benchmark',/)
 1001     format(' Size: ', i4, 'x', i4, 'x', i4)
 1002     format(' Iterations: ', i4, '    dt: ', F11.7)
 1004     format(' Total number of processes: ', i5)
 1005     format(' WARNING: compiled for ', i5, ' processes ')
 1003     format(' Number of active processes: ', i5, /)

       endif

c ...  broadcast input parameters
      call shmem_broadcast4(niter, niter, 1, 0, 0, 0, no_nodes, psync1)

      call shmem_broadcast8(dt, dt, 1, 0, 0, 0, no_nodes, psync2)

      call shmem_broadcast4(grid_points, grid_points, 3, 0, 0, 0,
     >                      no_nodes, psync1)

      call shmem_broadcast4(timeron,timeron,1,0,0,0,no_nodes,psync2)

      call make_set

       do  c = 1, ncells
          if ( (cell_size(1,c) .gt. IMAX) .or.
     >         (cell_size(2,c) .gt. JMAX) .or.
     >         (cell_size(3,c) .gt. KMAX) ) then
             print *,node, c, (cell_size(i,c),i=1,3)
             print *,' Problem size too big for compiled array sizes'
             goto 999
          endif
       end do

       do  i = 1, t_last
          call timer_clear(i)
       end do

       call set_constants

       call initialize

       call lhsinit

       call exact_rhs

       call compute_buffer_size(5)

c---------------------------------------------------------------------
c      do one time step to touch all code, and reinitialize
c---------------------------------------------------------------------
       call adi
       call initialize

c---------------------------------------------------------------------
c      Synchronize before placing time stamp
c---------------------------------------------------------------------
       do  i = 1, t_last
          call timer_clear(i)
       end do
       call shmem_barrier_all()

       call timer_clear(1)
       call timer_start(1)

       do  step = 1, niter

          if (node .eq. root) then
             if (mod(step, 20) .eq. 0 .or. 
     >           step .eq. 1) then
                write(*, 200) step
 200            format(' Time step ', i4)
              endif
          endif

          call adi

       end do

       call timer_stop(1)
       t = timer_read(1)
       
       call verify(niter, class, verified)

       call shmem_real8_max_to_all(tmax,t,1,0,0,no_nodes,pwrk,psync)

       if( node .eq. root ) then
          if( tmax .ne. 0. ) then
             mflops = (881.174*float( problem_size )**3
     >                -4683.91*float( problem_size )**2
     >                +11484.5*float( problem_size )
     >                -19272.4) * float( niter ) / (tmax*1000000.0d0)
          else
             mflops = 0.0
          endif

         call print_results('SP', class, grid_points(1), 
     >     grid_points(2), grid_points(3), niter, maxcells*maxcells, 
     >     total_nodes, tmax, mflops, '          floating point', 
     >     verified, npbversion,compiletime, 
     >     cs1, cs2, cs3, cs8, cs9, cs4, cs5, cs6, '(none)')
       endif

       if (.not.timeron) goto 999

       do i = 1, t_last
          t1(i) = timer_read(i)
       end do
       t1(t_xsolve) = t1(t_xsolve) - t1(t_xcomm)
       t1(t_ysolve) = t1(t_ysolve) - t1(t_ycomm)
       t1(t_zsolve) = t1(t_zsolve) - t1(t_zcomm)
       t1(t_last+2) = t1(t_xcomm)+t1(t_ycomm)+t1(t_zcomm)+t1(t_exch)
       t1(t_last+1) = t1(t_total)  - t1(t_last+2)

      call shmem_real8_sum_to_all(tsum,t1,t_last+2,0,0,
     >                            no_nodes,pwrk,psync)
      call shmem_real8_min_to_all(tming,t1,t_last+2,0,0,
     >                            no_nodes,pwrk,psync)

      call shmem_real8_max_to_all(tmaxg,t1,t_last+2,0,0,
     >                            no_nodes,pwrk,psync)

       if (node .eq. root) then
          write(*, 800) total_nodes
          do i = 1, t_last+2
             tsum(i) = tsum(i) / total_nodes
             write(*, 810) i, t_recs(i), tming(i), tmaxg(i), tsum(i)
          end do
       endif
 800   format(' nprocs =', i6, 11x, 'minimum', 5x, 'maximum', 
     >        5x, 'average')
 810   format(' timer ', i2, '(', A8, ') :', 3(2x,f10.4))

 999   continue
       call shmem_barrier_all()

       end
