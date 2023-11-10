/* KallistiOS ##version##

   arch/dreamcast/include/arch/init_flags.h
   Copyright (C) 2001 Megan Potter
   Copyright (C) 2023 Lawrence Sebald
   Copyright (C) 2023 Falco Girgis

*/

/** \file   arch/init_flags.h
    \brief  Dreamcast-specific initialization-related flags and macros.

    This file provides initialization-related flags that are specific to the
    Dreamcast architecture.

    \author Lawrence Sebald
    \author Megan Potter
    \author Falco Girgis

    \see    kos/init.h
*/

#ifndef __ARCH_INIT_FLAGS_H
#define __ARCH_INIT_FLAGS_H

#include <kos/cdefs.h>
__BEGIN_DECLS

#include <kos/init_base.h>

#define KOS_INIT_FLAGS_ARCH(flags) \
    KOS_INIT_FLAG_EXPORT(flags, INIT_CONTROLLER, int, cont_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_CONTROLLER, void, cont_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_KEYBOARD, int, kbd_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_KEYBOARD, void, kbd_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_MOUSE, int, mouse_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_MOUSE, void, mouse_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_LIGHTGUN, int, lightgun_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_LIGHTGUN, void, lightgun_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_VMU, int, vmu_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_VMU, void, vmu_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_VMU, int, vmu_fs_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_VMU, void, vmu_fs_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_PURUPURU, int, purupuru_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_PURUPURU, void, purupuru_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_SIP, int, sip_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_SIP, void, sip_shutdown); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_DREAMEYE, int, dreameye_init); \
    KOS_INIT_FLAG_EXPORT(flags, INIT_DREAMEYE, void, dreameye_shutdown)


/** \defgroup dreamcast_initflags   Dreamcast-specific initialization flags.

    These are the Dreamcast-specific flags that can be specified with
    KOS_INIT_FLAGS.

    \see    kos_initflags
    @{
*/

#define INIT_DEFAULT_ARCH   (INIT_CONTROLLER | INIT_VMU)

#define INIT_CONTROLLER     0x00001000  /**< \brief Enable Controller maple driver */
#define INIT_KEYBOARD       0x00002000  /**< \brief Enable Keyboard maple driver */
#define INIT_MOUSE          0x00004000  /**< \brief Enable Mouse maple driver */
#define INIT_LIGHTGUN       0x00008000  /**< \brief Enable Lightgun maple driver */
#define INIT_VMU            0x00010000  /**< \brief Enable VMU maple driver */
#define INIT_PURUPURU       0x00020000  /**< \brief Enable Puru Puru maple driver */
#define INIT_SIP            0x00040000  /**< \brief Enable Sound input maple driver */
#define INIT_DREAMEYE       0x00080000  /**< \brief Enable DreamEye maple driver */
#define INIT_MAPLE_ALL      0x000ff000  /**< \brief Enable all Maple drivers */

#define INIT_OCRAM          0x10000000 /**< \brief Use half of the dcache as RAM */
#define INIT_NO_DCLOAD      0x20000000 /**< \brief Disable dcload */

/** @} */

__END_DECLS

#endif /* !__ARCH_INIT_FLAGS_H */
