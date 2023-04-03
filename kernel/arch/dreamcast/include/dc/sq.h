/* KallistiOS ##version##

   kernel/arch/dreamcast/include/dc/sq.h
   (C)2000-2001 Andrew Kieschnick
*/

/** \file    dc/sq.h
    \ingroup store_queues
    \brief   Functions to access the SH4 Store Queues.

    \author Andrew Kieschnick
*/

/** \defgroup  store_queues Store Queues
    \brief  SH4 CPU Peripheral for burst memory transactions.

    The store queues are a way to do efficient burst transfers from the CPU to
    external memory. They can be used in a variety of ways, such as to transfer
    a texture to PVR memory. The transfers are in units of 32-bytes, and the
    destinations must be 32-byte aligned.

    \note
    Mastery over knowing when and how to utilize the store queues is 
    important when trying to push the limits of the Dreamcast, specifically
    when transferring chunks of data between regions of memory. It is often
    the case that the DMA is faster for transactions which are consistently 
    large; however, the store queues tend to have better performance and 
    have less configuration overhead when bursting smaller chunks of data. 
*/

#ifndef __DC_SQ_H
#define __DC_SQ_H

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>

/** \brief   Store Queue 0 access register 
    \ingroup store_queues
*/
#define QACR0 (*(volatile unsigned int *)(void *)0xff000038)

/** \brief   Store Queue 1 access register 
    \ingroup store_queues  
*/
#define QACR1 (*(volatile unsigned int *)(void *)0xff00003c)

/** \brief   Clear a block of memory.
    \ingroup store_queues

    This function is similar to calling memset() with a value to set of 0, but
    uses the store queues to do its work.

    \warning
    The dest pointer must be a 32-byte aligned with n being a multiple of 32!

    \param  dest            The address to begin clearing at (32-byte aligned).
    \param  n               The number of bytes to clear (multiple of 32).
*/
void sq_clr(void *dest, int n);

/** \brief   Copy a block of memory.
    \ingroup store_queues

    This function is similar to memcpy4(), but uses the store queues to do its
    work.

    \warning
    The dest pointer must be at least 32-byte aligned, the src pointer
    must be at least 4-byte aligned, and n must be a multiple of 32!

    \param  dest            The address to copy to (32-byte aligned).
    \param  src             The address to copy from (32-bit (4-byte) aligned).
    \param  n               The number of bytes to copy (multiple of 32).
    \return                 The original value of dest.

    \sa sq_cpy64()
*/
void * sq_cpy(void *dest, const void *src, int n);

/** \brief   Copy a block of memory.
    \ingroup store_queues

    This function is similar to sq_cpy(), but it operates on 64-bit 
    rather than 32-bit words, using the store queues to copy memory.

    \warning
    The dest pointer must be at least 64-byte aligned, the src pointer
    must be at least 8-byte aligned, and n must be a multiple of 64.

    \param  dest            The address to copy to (64-byte aligned).
    \param  src             The address to copy from (64-bit (8-byte) aligned).
    \param  n               The number of bytes to copy (multiple of 64).
    \return                 The original value of dest.

    \sa sq_cpy()
*/
void * sq_cpy64(void *dest, const void *src, int n);

/** \brief   Set a block of memory to an 8-bit value.
    \ingroup store_queues

    This function is similar to calling memset(), but uses the store queues to
    do its work.

    \warning
    s must be at least 32-byte aligned, n must be a multiple of 32, and only
    the low 8-bits are used from c.

    \param  s               The address to begin setting at (32-byte aligned).
    \param  c               The value to set (in the low 8-bits).
    \param  n               The number of bytes to set (multiple of 32).
    \return                 The original value of dest.

    \sa sq_set16(), sq_set32()
*/
void * sq_set(void *s, uint32 c, int n);

/** \brief   Set a block of memory to a 16-bit value.
    \ingroup store_queues

    This function is similar to calling memset2(), but uses the store queues to
    do its work.

    \warning
    s must be at least 32-byte aligned, n must be a multiple of 32, and only
    the low 16-bits are used from c.

    \param  s               The address to begin setting at (32-byte aligned).
    \param  c               The value to set (in the low 16-bits).
    \param  n               The number of bytes to set (multiple of 32).
    \return                 The original value of dest.

    \sa sq_set(), sq_set32()
*/
void * sq_set16(void *s, uint32 c, int n);

/** \brief   Set a block of memory to a 32-bit value.
    \ingroup store_queues

    This function is similar to calling memset4(), but uses the store queues to
    do its work.

    \warning
    s must be at least 32-byte aligned and n must be a multiple of 32.

    \param  s               The address to begin setting at (32-byte aligned).
    \param  c               The value to set (all 32-bits).
    \param  n               The number of bytes to set (multiple of 32).
    \return                 The original value of dest.

    \sa sq_set(), sq_set16()
*/
void * sq_set32(void *s, uint32 c, int n);

__END_DECLS

#endif

