/* KallistiOS ##version##

   include/kos/init.h
   Copyright (C) 2001 Megan Potter
   Copyright (C) 2023 Lawrence Sebald

*/

/** \file   kos/init.h
    \brief  Initialization-related flags and macros.

    This file provides initialization-related flags and macros that can be used
    to set up various subsystems of KOS on startup. Only flags that are
    architecture-independent are specified here, however this file also includes
    the architecture-specific file to bring in those flags as well.

    \author Lawrence Sebald
    \author Megan Potter
    \see    arch/init_flags.h
*/

#ifndef __KOS_INIT_H
#define __KOS_INIT_H

#include <kos/cdefs.h>
__BEGIN_DECLS

#include <arch/types.h>
#include <arch/init_flags.h>

#define KOS_INIT_FLAG_DECL(prefix) \
    int (*prefix##_init_weak)(void) __attribute__((weak)); \
    void (*prefix##_shutdown_weak)(void) __attribute__((weak))

#define KOS_INIT_FLAG_INIT(prefix) \
    if(prefix##_init_weak) (*prefix##_init_weak)()

#define KOS_INIT_FLAG_SHUTDOWN(prefix) \
    if(prefix##_shutdown_weak) (*prefix##_shutdown_weak)()

/** \cond */
#define KOS_INIT_FLAG(flags, flag, prefix) \
    extern int prefix##_init(void); \
    int (*prefix##_init_weak)(void) = ((flags) & flag) ? prefix##_init : NULL; \
    extern void prefix##_shutdown(void); \
    void (*prefix##_shutdown_weak)(void) = ((flags) & flag) ? prefix##_shutdown : NULL
/** \endcond */

/** \brief  Use this macro to determine the level of initialization you'd like
            in your program by default.

    The defaults will be fine for most things, and will be used if you do not
    specify any init flags yourself.

    \param  flags           Parts of KOS to init.
    \see    kos_initflags
    \see    dreamcast_initflags
*/
#define KOS_INIT_FLAGS(flags) \
    uint32 __kos_init_flags = (flags); \
    extern void arch_init_net(void); \
    void (*init_net_weak)(void) = ((flags) & INIT_NET) ? arch_init_net : NULL; \
    extern void net_shutdown(void); \
    void (*net_shutdown_weak)(void) = ((flags) & INIT_NET) ? net_shutdown : NULL; \
    extern int export_init(void); \
    int (*export_init_weak)(void) = ((flags) & INIT_EXPORT) ? export_init : NULL; \
    KOS_INIT_FLAG(flags, INIT_NET, bba_la); \
    KOS_INIT_FLAG(flags, INIT_CONTROLLER, cont); \
    KOS_INIT_FLAG(flags, INIT_KEYBOARD, kbd); \
    KOS_INIT_FLAG(flags, INIT_MOUSE, mouse); \
    KOS_INIT_FLAG(flags, INIT_LIGHTGUN, lightgun); \
    KOS_INIT_FLAG(flags, INIT_VMU, vmu); \
    KOS_INIT_FLAG(flags, INIT_PURUPURU, purupuru); \
    KOS_INIT_FLAG(flags, INIT_SIP, sip); \
    KOS_INIT_FLAG(flags, INIT_DREAMEYE, dreameye) 

/** \brief  The init flags. Do not modify this directly! */
extern uint32 __kos_init_flags;

/** \brief  Define a romdisk for your program, if you'd like one.
    \param  rd                  Pointer to the romdisk image in your code.
*/
#define KOS_INIT_ROMDISK(rd)    void * __kos_romdisk = (rd)

/** \brief  Built-in romdisk. Do not modify this directly! */
extern void * __kos_romdisk;

/** \brief  State that you don't want a romdisk. */
#define KOS_INIT_ROMDISK_NONE   NULL

/** \brief  Register a single function to be called very early in the boot
            process, before the BSS section is cleared.

    \param  func            The function to register. The prototype should be
                            void func(void)
*/
#define KOS_INIT_EARLY(func) void (*__kos_init_early_fn)(void) = (func)

/** \defgroup kos_initflags     Available flags for initialization

    These are the architecture-independent flags that can be specified with
    KOS_INIT_FLAGS.

    \see    dreamcast_initflags
    @{
*/
/** \brief  Default init flags (IRQs on, preemption enabled). */
#define INIT_DEFAULT \
    (INIT_IRQ | INIT_THD_PREEMPT)

#define INIT_NONE           0x00000000  /**< \brief Don't init optional things */
#define INIT_IRQ            0x00000001  /**< \brief Enable IRQs at startup */
/* Preemptive mode is the only mode now. Keeping define for compatability. */
#define INIT_THD_PREEMPT    0x00000002  /**< \brief Enable thread preemption */
#define INIT_NET            0x00000004  /**< \brief Enable built-in networking */
#define INIT_MALLOCSTATS    0x00000008  /**< \brief Enable malloc statistics */
#define INIT_QUIET          0x00000010  /**< \brief Disable dbgio */
#define INIT_EXPORT         0x00000020  /**< \brief Export kernel symbols */
#define INIT_CONTROLLER     0x00000040  /**< \brief Enable Controller maple driver */
#define INIT_KEYBOARD       0x00000080  /**< \brief Enable Keyboard maple driver */
#define INIT_MOUSE          0x00000100  /**< \brief Enable Mouse maple driver */
#define INIT_LIGHTGUN       0x00000200  /**< \brief Enable Lightgun maple driver */
#define INIT_VMU            0x00000400  /**< \brief Enable VMU maple driver */
#define INIT_PURUPURU       0x00000800  /**< \brief Enable Puru Puru maple driver */
#define INIT_SIP            0x00001000  /**< \brief Enable Sound input maple driver */
#define INIT_DREAMEYE       0x00002000  /**< \brief Enable DreamEye maple driver */
#define INIT_ALL            0x0000ffef  /**< \brief Initialize all KOS subsystems */

/** @} */

__END_DECLS

#endif /* !__KOS_INIT_H */
