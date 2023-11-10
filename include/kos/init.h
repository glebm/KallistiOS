/* KallistiOS ##version##

   include/kos/init.h
   Copyright (C) 2001 Megan Potter
   Copyright (C) 2023 Lawrence Sebald
   Copyright (C) 2023 Paul Cercueil
   Copyright (C) 2023 Falco Girgis

*/

/** \file   kos/init.h
    \brief  Initialization-related flags and macros.

    This file provides initialization-related flags and macros that can be used
    to set up various subsystems of KOS on startup. Only flags that are
    architecture-independent are specified here, however this file also includes
    the architecture-specific file to bring in those flags as well.

    \sa     arch/init_flags.h
    \sa     kos/init_base.h 

    \author Megan Potter
    \author Lawrence Sebald
    \author Paul Cercueil
    \author Falco Girgis
*/

#ifndef __KOS_INIT_H
#define __KOS_INIT_H

#include <kos/cdefs.h>
__BEGIN_DECLS

#include <arch/init_flags.h>
#include <stdint.h>

/** \brief  Exports and initializes the given KOS subsystems.

    KOS_INIT_FLAGS() provides a mechanism through which various components
    of KOS can be enabled and initialized depending on whether their flag
    has been included within the list.

    \note
    When no KOS_INIT_FLAGS() have been explicitly provided, the default
    flags used by KOS are equivalent to KOS_INIT_FLAGS(INIT_DEFAULT), which
    should enable all of the Maple peripherals.

    \param  flags           Parts of KOS to init.
 */
 #define KOS_INIT_FLAGS(flags) \
     const uint32_t __kos_init_flags = (flags); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_NET, void, arch_init_net); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_NET, void, net_shutdown); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_NET, void, bba_la_shutdown); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_FS_ROMDISK, int, fs_romdisk_init); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_FS_ROMDISK, int, fs_romdisk_shutdown); \
     KOS_INIT_FLAG_EXPORT(flags, INIT_EXPORT, int, export_init); \
     KOS_INIT_FLAGS_ARCH(flags)

 /** \cond The init flags. Do not modify this directly! */
 extern const uint32_t __kos_init_flags;
 /** \endcond **/

/** \brief  Legacy Romdisk initialization
    \deprecated
    This has been deprecated and should no longer be used
*/
/** \brief  Deprecated and not useful anymore. */
#define KOS_INIT_ROMDISK(rd) \
    static void *__old_romdisk __attribute__((unused)) = (rd)

/** \brief  State that you don't want a romdisk. */
#define KOS_INIT_ROMDISK_NONE   NULL

/** \cond Built-in romdisk. Do not modify this directly! */
extern void * __kos_romdisk;
/** \endcond */

/** \name Init Flags
    \brief Architecture-independent KOS initialization flags

    @{
*/ 
 /** \brief  Default init flags (IRQs on, preemption enabled, romdisk fs) */
 #define INIT_DEFAULT    (INIT_IRQ | INIT_THD_PREEMPT | INIT_FS_ROMDISK | \
                          INIT_DEFAULT_ARCH)
 
 #define INIT_NONE        0x00000000  /**< \brief Don't init optional things */
 #define INIT_IRQ         0x00000001  /**< \brief Enable IRQs at startup */
 /* Preemptive mode is the only mode now. Keeping define for compatability. */
 #define INIT_THD_PREEMPT 0x00000002  /**< \deprecated Already default mode */
 #define INIT_NET         0x00000004  /**< \brief Enable built-in networking */
 #define INIT_MALLOCSTATS 0x00000008  /**< \brief Enable malloc statistics */
 #define INIT_QUIET       0x00000010  /**< \brief Disable dbgio */
 #define INIT_EXPORT      0x00000020  /**< \brief Export kernel symbols */
 #define INIT_FS_ROMDISK  0x00000040  /**< \brief Enable support for romdisks */
 /** @} */

 __END_DECLS
#endif /* !__KOS_INIT_H */
