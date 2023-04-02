/* KallistiOS ##version##

   perfcntr.c
   Copyright (c)2003 Andy Barajas
*/

#include <arch/perfcntr.h>

/* Quick access macros */
#define PMCR_CTRL(o)  ( *((volatile uint16*)(0xff000084) + (o << 1)) )
#define PMCTR_HIGH(o) ( *((volatile uint32*)(0xff100004) + (o << 1)) )
#define PMCTR_LOW(o)  ( *((volatile uint32*)(0xff100008) + (o << 1)) )

#define PMCR_CLR        0x2000
#define PMCR_PMST       0x4000
#define PMCR_PMENABLE   0x8000
#define PMCR_RUN        0xc000
#define PMCR_PMM_MASK   0x003f

#define PMCR_CLOCK_TYPE_SHIFT 8

/* Get a counter's current configuration */
uint16 perf_cntr_get_config(int which) {
    return PMCR_CTRL(which);
}

/* Start a performance counter */
int perf_cntr_start(int which, int mode, int count_type) {
    perf_cntr_clear(which);
    PMCR_CTRL(which) = PMCR_RUN | mode | (count_type << PMCR_CLOCK_TYPE_SHIFT);
    
    return 0;
}

/* Stop a performance counter */
int perf_cntr_stop(int which) {
    PMCR_CTRL(which) &= ~(PMCR_PMM_MASK | PMCR_PMENABLE);

    return 0;
}

/* Clears a performance counter.  Has to stop it first. */
int perf_cntr_clear(int which) {
    perf_cntr_stop(which);
    PMCR_CTRL(which) |= PMCR_CLR;

    return 0;
}

/* Returns the count value of a counter */
uint64 perf_cntr_count(int which) {
    return (uint64)(PMCTR_HIGH(which) & 0xffff) << 32 | PMCTR_LOW(which);
}