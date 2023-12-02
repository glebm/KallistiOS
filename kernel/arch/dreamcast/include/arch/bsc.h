/* KallistiOS ##version##

   arch/dreamcast/include/bsc.h
   Copyright (c) 2023 Falco Girgis

*/

/** \file   arch/bsc.h
    \brief  Bus State Controller API

    \sa timer.h
    \sa wdt.h 
    \sa rtc.h

    \author Falco Girgis
*/

#ifndef __ARCH_BSC_H
#define __ARCH_BSC_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stdint.h>

/** \defgroup bsc    Bus State Controller (BSC)
    \brief           Driver for the SH4's BSC Peripheral

   \sa timers
   \sa wdt
*/   

typedef void (*bsc_callback_t)(void*);

void bsc_set_isrs(uint8_t priority, 
                  bsc_callback_t comp_match_callback, void *comp_match_data,
                  bsc_callback_t overflow_callback, void *overflow_data);

__END_DECLS

#endif  /* __ARCH_BSC_H */
