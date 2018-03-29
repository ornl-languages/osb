The NAS Parallel Benchmarks (NPB) are a small set of programs designed to help evaluate the performance of parallel supercomputers.
The benchmarks are derived from computational fluid dynamics (CFD) applications and consist of five kernels and three pseudo-applications in the original "pencil-and-paper" specification (NPB 1).
The benchmark suite has been extended to include new benchmarks for unstructured adaptive mesh, parallel I/O, multi-zone applications, and computational grids.
Problem sizes in NPB are predefined and indicated as different classes.
Reference implementations of NPB are available in commonly-used programming models like MPI and OpenMP (NPB 2 and NPB 3).

Features of the 1.0a version of the OpenSHMEM implementation:
    - based on the MPI implementation of NPB 3.2
        - Fortran: BT, EP, MG, SP
        - C: IS
    - separate directories for the C and Fortran versions
    - easily configurable to use various OpenSHMEM implementations using the samples provided in
        - config/NAS.samples/make.def.{uhosh,sgi,cray}
