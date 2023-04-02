/* KallistiOS ##version##

   arch/dreamcast/include/timer.h
   (c)2000-2001 Megan Potter

*/

/** \file   arch/timer.h
    \brief  Low-level timer functionality.

    This file contains functions for interacting with the timer sources on the
    SH4. Many of these functions may interfere with thread operation or other
    such things, and should thus be used with caution. Basically, the only
    functionality that you might use in practice in here in normal programs is
    the gettime functions.

    \author Megan Potter
*/

#ifndef __ARCH_TIMER_H
#define __ARCH_TIMER_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>
#include <arch/irq.h>

/** \defgroup   timers Timers
 * This API exposes 3 of the 4 hardware timer peripherals.
 * 
 * \note 
 * These timers are used internally by KOS for various
 * tasks such as thread scheduling and implementing 
 * high-level timing functions and should not typically
 * be accessed directly.
 * 
 * \note The watchdog timer is not supported.
 * \sa perf_counters
*/

/** \defgroup   timer_sources Timer Sources
    The SH4 has 4 hardware timer peripherals. 
    The fourth is a watchdog timer, which is unsupported.
    \ingroup timers
    @{
*/
/** \brief  SH4 Timer 0.
 * 
    This timer is used for thread operation, and thus is off limits if you want
    that to work properly.
*/
#define TMU0    0

/** \brief  SH4 Timer 1.

    This timer is used for the timer_spin_sleep() function.
*/
#define TMU1    1

/** \brief  SH4 Timer 2.

    This timer is used by the various gettime functions in this header.
*/
#define TMU2    2

/** \brief  SH4 Watchdog Timer.

    KallistiOS does not currently support using this timer.
*/
#define WDT     3
/** }@ **/

/** \brief  Which timer does the thread system use?
 *  \ingroup timers 
 **/
#define TIMER_ID TMU0

/** \brief  Pre-initialize a timer, but do not start it.
    \ingroup timers

    This function sets up a timer for use, but does not start it.

    \param  which           The timer to set up (i.e, \ref TMU0).
    \param  speed           The number of ticks per second.
    \param  interrupts      Set to 1 to receive interrupts when the timer ticks.
    \retval 0               On success.
*/
int timer_prime(int which, uint32 speed, int interrupts);

/** \brief  Start a timer.
    \ingroup timers

    This function starts a timer that has been initialized with timer_prime(),
    starting raising interrupts if applicable.

    \param  which           The timer to start (i.e, \ref TMU0).
    \retval 0               On success.
*/
int timer_start(int which);

/** \brief  Stop a timer.
    \ingroup timers

    This function stops a timer that was started with timer_start(), and as a
    result stops interrupts coming in from the timer.

    \param  which           The timer to stop (i.e, \ref TMU0).
    \retval 0               On success.

    \sa timer_stop()
*/
int timer_stop(int which);

/** \brief  Obtain the count of a timer.   
    \ingroup timers

    This function simply returns the count of the timer.

    \param  which           The timer to inspect (i.e, \ref TMU0).
    \return                 The timer's count.
    
    \sa timer_start()
*/
uint32 timer_count(int which);

/** \brief  Clear the underflow bit of a timer.
    \ingroup timers

    This function clears the underflow bit of a timer if it was set.

    \param  which           The timer to inspect (i.e, \ref TMU0).
    \retval 0               If the underflow bit was clear (prior to calling).
    \retval 1               If the underflow bit was set (prior to calling).
*/
int timer_clear(int which);

/** \brief  Spin-loop sleep function.

    This function is meant as a very accurate delay function, even if threading
    and interrupts are disabled. It uses \ref TMU1 to sleep.

    \param  ms              The number of milliseconds to sleep.
*/
void timer_spin_sleep(int ms);

/** \brief  Enable high-priority timer interrupts.
    \ingroup timers

    This function enables interrupts on the specified timer.

    \param  which           The timer to enable interrupts on (i.e, \ref TMU0).

    \sa timer_disable_ints()
*/
void timer_enable_ints(int which);

/** \brief  Disable timer interrupts.
    \ingroup timers

    This function disables interrupts on the specified timer.

    \param  which           The timer to disable interrupts on (i.e, \ref TMU0).

    \sa timer_enable_ints()
*/
void timer_disable_ints(int which);

/** \brief  Check whether interrupts are enabled on a timer.
    \ingroup timers

    This function checks whether or not interrupts are enabled on the specified
    timer.

    \param  which           The timer to inspect (i.e, \ref TMU0).
    \retval 0               If interrupts are disabled on the timer.
    \retval 1               If interrupts are enabled on the timer.
*/
int timer_ints_enabled(int which);

/** \brief  Enable the millisecond timer.
    \ingroup timers

    This function enables the timer used for the gettime functions. This is on
    by default. These functions use \ref TMU2 to do their work.
    
    \sa timer_ms_disable()
*/
void timer_ms_enable(void);

/** \brief  Disable the millisecond timer.
    \ingroup timers

    This function disables the timer used for the gettime functions. Generally,
    you will not want to do this, unless you have some need to use the timer
    \ref TMU2 for something else.

    \sa timer_ms_enable()
*/
void timer_ms_disable(void);

/** \brief  Get the current uptime of the system.
    \ingroup timers

    This function retrieves the number of seconds and milliseconds since KOS was
    started.

    \param  secs            A pointer to store the number of seconds since boot
                            into.
    \param  msecs           A pointer to store the number of milliseconds past
                            a second since boot.
    \sa timer_ms_gettime64()
*/
void timer_ms_gettime(uint32 *secs, uint32 *msecs);

/** \brief  Get the current uptime of the system (in milliseconds).
    \ingroup timers

    This function retrieves the number of milliseconds since KOS was started. It
    is equivalent to calling timer_ms_gettime() and combining the number of
    seconds and milliseconds into one 64-bit value.

    \return                 The number of milliseconds since KOS started.
    
    \sa timer_ms_gettime()
*/
uint64 timer_ms_gettime64(void);

/** \brief  Get the current uptime of the system.
    \ingroup timers

    This function retrieves the number of seconds and microseconds since KOS was
    started.
    \note 
    There is no precise microsecond timer, so this function will either utilize
    the nanosecond or millisecond timer, depending on what is available

    \param  secs            A pointer to store the number of seconds since boot
                            into.
    \param  msecs           A pointer to store the number of microseconds past
                            a second since boot.
    
    \sa timer_us_gettime64()
*/
void timer_us_gettime(uint32 *secs, uint32 *usecs);

/** \brief  Get the current uptime of the system (in microseconds).
    \ingroup timers

    This function retrieves the number of microseconds since KOS was started. It
    should be more precise, in theory, than timer_ms_gettime64(), but the exact
    amount of preciseness is undetermined.

    \return                 The number of microseconds since KOS started.

    \sa timer_us_gettime()
*/
uint64 timer_us_gettime64(void);

/** \brief  Enable the nanosecond timer.
    \ingroup timers

    This function enables the performance counter used for the timer_ns_gettime64() 
    function. This is on by default. The function uses \ref PRFC0 to do the work.

    \sa timer_ns_disable(), perf_counters
*/
void timer_ns_enable(void);

/** \brief  Disable the nanosecond timer.
    \ingroup timers

    This function disables the performance counter used for the timer_ns_gettime64() 
    function. Generally, you will not want to do this, unless you have some need to use 
    the counter \ref PRFC0 for something else.

    \sa timer_ns_enable(), perf_counters
*/
void timer_ns_disable(void);

/** \brief  Get the current uptime of the system.
    \ingroup timers

    This function retrieves the number of seconds and nanoseconds since KOS was
    started.

    \note
    When the ns timer is disabled, calling this function will still gracefully
    fall back to msec resolution.

    \param  secs            A pointer to store the number of seconds since boot
                            into.
    \param  nsecs           A pointer to store the number of nanoseconds past
                            a second since boot.

    \sa timer_ns_gettime64(), perf_counters
*/
void timer_ns_gettime(uint32 *secs, uint32 *nsecs);

/** \brief  Get the current uptime of the system (in nanoseconds).
    \ingroup timers
    \note
    If the ns timer is not currently enabled, this function will 
    gracefully fall back to using the msec timer with lower resolution.

    This function retrieves the number of nanoseconds since KOS was started.

    \return                 The number of nanoseconds since KOS started.

    \sa timer_ns_gettime(), perf_counters
*/
uint64 timer_ns_gettime64(void);

/** \brief  Primary timer callback type.
 *  \ingroup timers
 *  \sa timer_primary_set_callback()
 **/
typedef void (*timer_primary_callback_t)(irq_context_t *);

/** \brief  Set the primary timer callback.
    \ingroup timers

    This function sets the primary timer callback to the specified function
    pointer. Generally, you should not do this, as the threading system relies
    on the primary timer to work.

    \param  callback        The new timer callback (set to NULL to disable).
    \return                 The old timer callback.
*/
timer_primary_callback_t timer_primary_set_callback(timer_primary_callback_t callback);

/** \brief  Request a primary timer wakeup.
    \ingroup timers

    This function will wake the caller (by calling the primary timer callback)
    in approximately the number of milliseconds specified. You can only have one
    timer wakeup scheduled at a time. Any subsequently scheduled wakeups will
    replace any existing one.

    \param  millis          The number of milliseconds to schedule for.
*/
void timer_primary_wakeup(uint32 millis);

/** \cond */
/* Init function. Automatically called by KOS during initialization */
int timer_init(void);

/* Shutdown function. Automatically called by KOS during shutdown */
void timer_shutdown(void);
/** \endcond */

__END_DECLS

#endif  /* __ARCH_TIMER_H */