static u64 np = 1;
static u64 me = 0;
static u64 sz = 512 * 1024 * 1024;
static void *base, *bump, *end;

void shmem_barrier_all();
void shmem_int_sum_to_all(int* target, const int* source, int nreduce, 
                          int PE_start, int logPE_stride, int PE_size, 
                          int* pWrk, long* pSync);
void shmem_longlong_sum_to_all(s64* target, const s64* source, int nreduce,
                              int PE_start, int logPE_stride, int PE_size,
                              long long* pWrk, long* pSync);
uint64_t shmem_longlong_g(const uint64_t* addr, int pe);
void shmem_longlong_p(uint64_t* addr, uint64_t value, int pe);
void shmem_longlong_inc(s64* target, int pe);
void shmem_quiet();
void shmem_free(void* ptr);
void shmem_finalize();

// Initialize SHMEM with mmap
void shmem_init() {
	base = mmap((void *)(((u64)1)<<36), sz, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (base == MAP_FAILED) {
		perror("Failed to mmap shared memory region");
		exit(1);
	}
	bump = base;
}

// Simple bump allocator for SHMEM memory
void* shmem_malloc(size_t size) {
	void *ret = bump;
	bump += ((u64)size + 7) & ~7;
	return ret;
}

int shmem_n_pes() {
	return np;
}

int shmem_my_pe() {
	return me;
}

void shmem_barrier_all() {
}

void shmem_int_sum_to_all(int* target, const int* source, int nreduce, int PE_start, int logPE_stride, int PE_size, int* pWrk, long* pSync) {
}

void shmem_longlong_sum_to_all(s64* target, const s64* source, int nreduce, int PE_start, int logPE_stride, int PE_size, long long* pWrk, long* pSync) {
}

uint64_t shmem_longlong_g(const uint64_t* addr, int pe) {
    return *addr;
}

void shmem_longlong_p(uint64_t* addr, uint64_t value, int pe) {
    *addr = value;
}

void shmem_longlong_inc(s64* target, int pe) {
	(*target)++;
}

void shmem_quiet() {
}

void shmem_free(void* ptr) {
}

void shmem_finalize() {
}

