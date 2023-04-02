/* KallistiOS ##version##

   arch/dreamcast/include/perfcnter.h
   (c)2023 Andy Barajas, Falco Girgis

*/

/** \file   arch/perfcnter.h
    \brief  Hardware performance counter API

    This file contains the API for driving the hardware 
    performance counter peripherals.

    \author Andy Barajas
*/

#ifndef __ARCH_PERFCNTER_H
#define __ARCH_PERFCNTER_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/** \defgroup   perf_counters Performance Counters
    The performance counter API exposes the SH4's hardware profiling registers, 
    which consist of two different sets of independently operable 64-bit 
    counters.  
    \sa timers
*/

/** \defgroup perf_counters_sources Performance Counter Sources
 *  \brief The two sets of performance counter registers 
 *  \ingroup perf_counters
 * @{
 */

 /** \brief  SH4 Performance Counter.
  * 
    \note 
    By default this peformance counter is used to 
    enable the ns timer and to increase the precision
    of applicable standard C/C++ timing functions.

    \sa timers
*/  
#define PRFC0   0

/** \brief  SH4 Performance Counter.
   
    A counter that is not used by KOS.
*/
#define PRFC1   1
/** }@
 */

/** \defgroup perf_counters_types Performance Counter Cycle Count Type
 *  \brief Counter type for configuring the performance counters
 *  \ingroup perf_counters
 * @{
 */

/** \brief  CPU Cycles Count Type.

    Count cycles. At 5 ns increments, a 48-bit cycle counter can 
    run continuously for 16.33 days.
*/
#define PMCR_COUNT_CPU_CYCLES 0

/** \brief  Ratio Cycles Count Type.
    \ingroup perf_counters

    CPU/bus ratio mode where cycles (where T = C x B / 24 and T is time, 
    C is count, and B is time of one bus cycle).
*/
#define PMCR_COUNT_RATIO_CYCLES 1
/** }@
 */

/** \defgroup   perf_counters_modes Performance Counter Modes
 * 
    This is the list of modes that are allowed to be passed into the perf_cntr_start()
    function, representing different things you want to count.
    \ingroup perf_counters
    @{
*/
/*                MODE DEFINITION                  VALUE   MEASURMENT TYPE & NOTES */
#define PMCR_INIT_NO_MODE                           0x00 /**< \brief None; Just here to be complete */
#define PMCR_OPERAND_READ_ACCESS_MODE               0x01 /**< \brief Quantity; With cache */
#define PMCR_OPERAND_WRITE_ACCESS_MODE              0x02 /**< \brief Quantity; With cache */
#define PMCR_UTLB_MISS_MODE                         0x03 /**< \brief Quantity */
#define PMCR_OPERAND_CACHE_READ_MISS_MODE           0x04 /**< \brief Quantity */
#define PMCR_OPERAND_CACHE_WRITE_MISS_MODE          0x05 /**< \brief Quantity */
#define PMCR_INSTRUCTION_FETCH_MODE                 0x06 /**< \brief Quantity; With cache */
#define PMCR_INSTRUCTION_TLB_MISS_MODE              0x07 /**< \brief Quantity */
#define PMCR_INSTRUCTION_CACHE_MISS_MODE            0x08 /**< \brief Quantity */
#define PMCR_ALL_OPERAND_ACCESS_MODE                0x09 /**< \brief Quantity */
#define PMCR_ALL_INSTRUCTION_FETCH_MODE             0x0a /**< \brief Quantity */
#define PMCR_ON_CHIP_RAM_OPERAND_ACCESS_MODE        0x0b /**< \brief Quantity */
/* No 0x0c */
#define PMCR_ON_CHIP_IO_ACCESS_MODE                 0x0d /**< \brief Quantity */
#define PMCR_OPERAND_ACCESS_MODE                    0x0e /**< \brief Quantity; With cache, counts both reads and writes */
#define PMCR_OPERAND_CACHE_MISS_MODE                0x0f /**< \brief Quantity */
#define PMCR_BRANCH_ISSUED_MODE                     0x10 /**< \brief Quantity; Not the same as branch taken! */
#define PMCR_BRANCH_TAKEN_MODE                      0x11 /**< \brief Quantity */
#define PMCR_SUBROUTINE_ISSUED_MODE                 0x12 /**< \brief Quantity; Issued a BSR, BSRF, JSR, JSR/N */
#define PMCR_INSTRUCTION_ISSUED_MODE                0x13 /**< \brief Quantity */
#define PMCR_PARALLEL_INSTRUCTION_ISSUED_MODE       0x14 /**< \brief Quantity */
#define PMCR_FPU_INSTRUCTION_ISSUED_MODE            0x15 /**< \brief Quantity */
#define PMCR_INTERRUPT_COUNTER_MODE                 0x16 /**< \brief Quantity */
#define PMCR_NMI_COUNTER_MODE                       0x17 /**< \brief Quantity */
#define PMCR_TRAPA_INSTRUCTION_COUNTER_MODE         0x18 /**< \brief Quantity */
#define PMCR_UBC_A_MATCH_MODE                       0x19 /**< \brief Quantity */
#define PMCR_UBC_B_MATCH_MODE                       0x1a /**< \brief Quantity */
/* No 0x1b-0x20 */
#define PMCR_INSTRUCTION_CACHE_FILL_MODE            0x21 /**< \brief Cycles */
#define PMCR_OPERAND_CACHE_FILL_MODE                0x22 /**< \brief Cycles */
#define PMCR_ELAPSED_TIME_MODE                      0x23 /**< \brief Cycles; For 200MHz CPU: 5ns per count in 1 cycle = 1 count mode, or around 417.715ps per count (increments by 12) in CPU/bus ratio mode */
#define PMCR_PIPELINE_FREEZE_BY_ICACHE_MISS_MODE    0x24 /**< \brief Cycles */
#define PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE    0x25 /**< \brief Cycles */
/* No 0x26 */
#define PMCR_PIPELINE_FREEZE_BY_BRANCH_MODE         0x27 /**< \brief Cycles */
#define PMCR_PIPELINE_FREEZE_BY_CPU_REGISTER_MODE   0x28 /**< \brief Cycles */
#define PMCR_PIPELINE_FREEZE_BY_FPU_MODE            0x29 /**< \brief Cycles */
/** @} */

/** \brief 5ns per count in 1 cycle = 1 count mode(PMCR_COUNT_CPU_CYCLES) 
    \ingroup perf_counters
*/
#define PMCR_NS_PER_CYCLE      5

/** \brief  Get a performance counter's settings.
    \ingroup perf_counters

    This function returns a performance counter's settings.

    \param  which           The performance counter (i.e, \ref PRFC0 or PRFC1).
    \retval 0               On success.
*/
uint16 perf_cntr_get_config(int which);

/** \brief  Start a performance counter.
    \ingroup perf_counters

    This function starts a performance counter

    \param  which           The counter to start (i.e, \ref PRFC0 or PRFC1).
    \param  mode            Use one of the 33 modes listed above.
    \param  count_type      PMCR_COUNT_CPU_CYCLES or PMCR_COUNT_RATIO_CYCLES.
    \retval 0               On success.
*/
int perf_cntr_start(int which, int mode, int count_type);

/** \brief  Stop a performance counter.
    \ingroup perf_counters

    This function stops a performance counter that was started with perf_cntr_start().
    Stopping a counter retains its count. To clear the count use perf_cntr_clear().

    \param  which           The counter to stop (i.e, \ref PRFC0 or PRFC1).
    \retval 0               On success.
*/
int perf_cntr_stop(int which);

/** \brief  Clear a performance counter.
    \ingroup perf_counters

    This function clears a performance counter. It resets its count to zero.
    This function stops the counter before clearing it because you cant clear 
    a running counter.

    \param  which           The counter to clear (i.e, \ref PRFC0 or PRFC1).
    \retval 0               On success.
*/
int perf_cntr_clear(int which);

/** \brief  Obtain the count of a performance counter.
    \ingroup perf_counters

    This function simply returns the count of the counter.

    \param  which           The counter to read (i.e, \ref PRFC0 or PRFC1).
    \return                 The counter's count.
*/
uint64 perf_cntr_count(int which);

__END_DECLS

#endif  /* __ARCH_PERFCNTR_H */