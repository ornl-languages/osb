#ifndef GET_CLOCK_H
#define GET_CLOCK_H

#if defined (__x86_64__) || defined(__i386__)
/* Note: only x86 CPUs which have rdtsc instruction are supported. */
typedef unsigned long long cycles_t;
static inline cycles_t get_cycles()
{
	unsigned low, high;
	unsigned long long val;
	asm volatile ("rdtsc" : "=a" (low), "=d" (high));
	val = high;
	val = (val << 32) | low;
	return val;
}
#elif defined(__PPC__) || defined(__PPC64__)
/* Note: only PPC CPUs which have mftb instruction are supported. */
/* PPC64 has mftb */
typedef unsigned long cycles_t;
static inline cycles_t get_cycles()
{
	cycles_t ret;

	asm volatile ("mftb %0" : "=r" (ret) : );
	return ret;
}
#elif defined(__ia64__)
/* Itanium2 and up has ar.itc (Itanium1 has errata) */
typedef unsigned long cycles_t;
static inline cycles_t get_cycles()
{
	cycles_t ret;

	asm volatile ("mov %0=ar.itc" : "=r" (ret));
	return ret;
}

#else
#warning get_cycles not implemented for this architecture: attempt asm/timex.h
#include <asm/timex.h>
#endif

extern double get_cpu_mhz(int);

struct report_options {
    int unsorted;
    int histogram;
    int cycles;   /* report delta's in cycles, not microsec's */
};

static inline cycles_t get_median(int n, cycles_t delta[])
{
    if ((n - 1) % 2)
        return (delta[n / 2] + delta[n / 2 - 1]) / 2;
    else
        return delta[n / 2];
}

static int cycles_compare(const void * aptr, const void * bptr)
{
    const cycles_t *a = aptr;
    const cycles_t *b = bptr;
    if (*a < *b) return -1;
    if (*a > *b) return 1;
    return 0;
}

static void print_report(struct report_options * options,
             unsigned int iters, cycles_t *tstamp)
{
    double cycles_to_units;
    cycles_t median;
    unsigned int i,j;
    const char* units;
    cycles_t *delta = malloc((iters) * sizeof *delta);

    if (!delta) {
        perror("malloc");
        return;
    }

    for (i = 0,j=0; i < iters; i++,j+=2)
        delta[i] = tstamp[j + 1] - tstamp[j];


    if (options->cycles) {
        cycles_to_units = 1;
        units = "cycles";
    } else {
        cycles_to_units = get_cpu_mhz(0);
        units = "usec";
    }

    if (options->unsorted) {
        printf("#, %s\n", units);
        for(i = 0; i < iters; ++i)
            printf("%d, %g\n", i + 1, delta[i] / cycles_to_units / 2);
    }

    qsort(delta, iters, sizeof *delta, cycles_compare);

    if (options->histogram) {
        printf("#, %s\n", units);
        for(i = 0; i < iters; ++i)
            printf("%d, %g\n", i + 1, delta[i] / cycles_to_units / 2);
    }

    median = get_median(iters, delta);

    printf("Latency typical: %g %s\n", median / cycles_to_units / 2, units);
    printf("Latency best   : %g %s\n", delta[0] / cycles_to_units / 2, units);
    printf("Latency worst  : %g %s\n", delta[iters - 1] / cycles_to_units / 2, units);

    free(delta);
}

#endif
