
c---------------------------------------------------------------------
c---------------------------------------------------------------------

      subroutine setup_rma

c---------------------------------------------------------------------
c---------------------------------------------------------------------

c---------------------------------------------------------------------
c set up RMA stuff
c---------------------------------------------------------------------

      use npbrma
      implicit none
      include 'mpp/shmem.fh'

      integer num_pes, my_pe

      integer nc, color

      call start_pes(0)
      total_nodes = num_pes()
      no_nodes = num_pes()
      node = my_pe()

      if (.not. convertdouble) then
         dp_type = MPI_DOUBLE_PRECISION
      else
         dp_type = MPI_REAL
      endif

c---------------------------------------------------------------------
c     compute square root; add small number to allow for roundoff
c---------------------------------------------------------------------
      nc = dint(dsqrt(dble(total_nodes) + 0.00001d0))

c---------------------------------------------------------------------
c We handle a non-square number of nodes by making the excess nodes
c inactive. However, we can never handle more cells than were compiled
c in. 
c---------------------------------------------------------------------

      if (nc .gt. maxcells) nc = maxcells

      if (node .gt. nc*nc) then
         active = .false.
         color = 1
      else
         active = .true.
         color = 0
      end if
      
      if (.not. active) return


c---------------------------------------------------------------------
c     let node 0 be the root for the group (there is only one)
c---------------------------------------------------------------------
      root = 0
      if (no_nodes .ne. total_nodes) then
         if (node .eq. root) write(*,1000) total_nodes, no_nodes
1000     format(' Total number of nodes',I5,
     &          ' does not match with squared number of nodes',I5)
      endif
      if (.not.active) return

c---------------------------------------------------------------------
c     create window for RMA
c---------------------------------------------------------------------
      buffoff(1) = BUF_SIZE
      buffoff(2) = buffoff(1) + BUF_SIZZ
      buffoff(3) = buffoff(2) + BUF_SIZZ
      buff_id = 1


      return
      end

