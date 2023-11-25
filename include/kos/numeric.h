/* KallistiOS ##version##

   kos/numeric.h
   Copyright (C) 2023 Falco Girgis

*/

/** \file   kos/numeric.h
    \brief  Numeric Algorithms, Conversions, and Utilities

    This file contains the definitions of KOS' name manager. A "name" is a
    generic identifier for some kind of module. These modules may include
    services provided by the kernel (such as VFS handlers).

    \author Falco Girgis
*/

#ifndef __KOS_NUMERIC_H
#define __KOS_NUMERIC_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <stdint.h>
#include <stddef.h>

static inline uint8_t bcd_from_dec(uint8_t dec) {
   return (((dec / 10) << 4) | (dec % 10));
}

static inline uint8_t bcd_to_dec(uint8_t bcd) {
   return (((bcd >> 4) * 10) + (bcd & 0xf));
}

/* Utility function for aligning an address or offset. */
static inline size_t align_to(size_t address, size_t alignment) {
    return (address + (alignment - 1)) & ~(alignment - 1);
}


__END_DECLS

#endif /* __KOS_NUMERIC_H */