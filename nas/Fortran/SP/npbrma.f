      module npbrma

c---------------------------------------------------------------------
c The following include file is generated automatically by the
c "setparams" utility. It defines 
c      maxcells:      the square root of the maximum number of processors
c      problem_size:  12, 64, 102, 162 (for class T, A, B, C)
c      dt_default:    default time step for this problem size if no
c                     config file
c      niter_default: default number of iterations for this problem size
c---------------------------------------------------------------------

      include 'npbparams.h'
      include 'mpif.h'

      integer, parameter :: maxcelldim = (problem_size/maxcells)+1,
     >      BUF_SIZE=maxcelldim*maxcelldim*(maxcells-1)*60+1,
     >      BUF_SIZZ=(2*maxcells-2)/maxcells*maxcelldim*maxcelldim*22+1

      integer :: node, no_nodes, total_nodes, root, dp_type,
     >      comm_setup, comm_solve
      logical :: active

      double precision :: in_buffer(BUF_SIZE)
      integer :: buffoff(3), buff_id

      double precision, save :: out_buffer(BUF_SIZE+BUF_SIZZ*3)

      integer :: win, group, pred_group(3), succ_group(3)
      integer(kind=mpi_address_kind) disp

      end module npbrma
