/* KallistiOS ##version##

   kos/opts.h
   Copyright (C) 2014 Lawrence Sebald
*/

/** \file    kos/opts.h
    \brief   Compile-time options regarding debugging and other topics.
    \ingroup debugging_options

    This file is meant to be a kind of Grand Central Station for all of the
    various compile-time options that can be set when building KOS. Each of the
    various compile-time macros that control things like additional debugging
    checks and such should be documented in this particular file. In addition,
    groups of related macros are provided here to make it easier to set these
    options at compile time.

    Basically, this is here to make it easier to customize your copy of KOS. In
    the past, you would have had to search through all the various files to find
    potential options, which was both annoying and error-prone. In addition,
    often various debugging flags would end up being set in the main repository
    as people debugged individual pieces of code, leaving them set for everyone
    else, even if they aren't necessarily useful/helpful all the time (KM_DBG,
    I'm looking at you).

    \author Lawrence Sebald
*/

#ifndef __KOS_OPTS_H
#define __KOS_OPTS_H

#include <sys/cdefs.h>
__BEGIN_DECLS

/** \defgroup debugging_options Options
    \brief                      Compile-time options for debugging KOS
    \ingroup                    debugging

   Various debug options. Uncomment the `#define` line to enable the specific
   option described. 

   @{
*/

/* Enable debugging in fs_vmu. */
/* #define VMUFS_DEBUG 1 */


/* Enable to allow extra debugging checks in the malloc code itself. This
   sometimes catches corrupted blocks. Recommended during debugging phases. */
/* #define MALLOC_DEBUG 1 */

/* Enable this define if you want costly malloc debugging (buffer sentinel
   checking, block leak checking, etc). Recommended during debugging phases, but
   you should probably take it out before you start your final testing. */
/* #define KM_DBG 1 */

/* Enable this define if you want REALLY verbose malloc debugging (print every
   time a block is allocated or freed). Only enable this if you are having some
   serious issues. */
/* #define KM_DBG_VERBOSE 1 */


/* The following three macros are similar to the ones above, but for the PVR
   memory pool malloc. */
/* #define PVR_MALLOC_DEBUG 1 */
/* #define PVR_KM_DBG 1 */
/* #define PVR_KM_DBG_VERBOSE 1 */

/* Enable this define to enable PVR error interrupts and to have the interrupt
   handler print them when they occur.  */
/* #define PVR_RENDER_DBG */

/* Aggregate debugging levels. It's probably best to enable these with your
   KOS_CFLAGS when compiling KOS itself, but they're all documented here and
   can be enabled here, if you really want to. */

/* Enable all recommended options for normal debugging use. This enables the
   first level of malloc debugging for the main pool, as well as the internal
   dlmalloc debugging code for the main pool. This is essentially the set that
   was normally enabled in the repository tree. */
/* #define KOS_DEBUG 1 */

/* Enable verbose debugging support. Basically, if your code is crashing and the
   stuff in the first level doesn't help, then use this one instead. This adds
   in verbose malloc debugging in the main pool, as well as basic PVR malloc
   debugging. */
/* #define KOS_DEBUG 2 */

/* Enable verbose debugging support, plus additional debugging options that
   normally wouldn't be needed. Once again, try lighter levels before this one,
   as there's not much reason this one should ever be needed. This level
   includes the internal debugging stuff in fs_vmu, verbose debugging in the PVR
   malloc, as well as everything in level 2. */
/* #define KOS_DEBUG 3 */

#ifdef NDEBUG
#define KOS_DEBUG 0
#define DBG_LOGLEVEL = DBG_DEAD
#endif

#ifndef KOS_DEBUG 
#define KOS_DEBUG 0
#ifndef DBG_LOGLEVEL
    #define DBG_LOGLEVEL = DBG_WARNING
#endif
#endif

#if KOS_DEBUG >= 1
#define MALLOC_DEBUG 1
#define KM_DBG 1
#ifndef DBG_LOGLEVEL
    #define DBG_LOGLEVEL = DBG_DEBUG
#endif
#endif

#if KOS_DEBUG >= 2
#define KM_DBG_VERBOSE 1
#define PVR_MALLOC_DEBUG 1
#define PVR_KM_DBG 1
#ifndef DBG_LOGLEVEL
    #define DBG_LOGLEVEL = DBG_DEBUG
#endif
#endif

#if KOS_DEBUG >= 3
#define PVR_KM_DBG_VERBOSE 1
#define VMUFS_DEBUG 1
#ifndef DBG_LOGLEVEL
    #define DBG_LOGLEVEL = DBG_KDEBUG
#endif
#define DBG_DYNLOG 1
#endif

/** \defgroup   dbglog_levels   Log Levels
    \brief                      dbglog severity levels
    \ingroup                    logging

    This is the list of levels that are allowed to be passed into the dbglog()
    function, representing different levels of importance.

    @{
*/
#define DBG_DEAD        0       /**< \brief The system is dead */
#define DBG_CRITICAL    1       /**< \brief A critical error message */
#define DBG_ERROR       2       /**< \brief A normal error message */
#define DBG_WARNING     3       /**< \brief Potential problem */
#define DBG_NOTICE      4       /**< \brief Normal but significant */
#define DBG_INFO        5       /**< \brief Informational messages */
#define DBG_DEBUG       6       /**< \brief User debug messages */
#define DBG_KDEBUG      7       /**< \brief Kernel debug messages */
/** @} */

/** \brief  The maximum number of cd files that can be open at a time. */
#ifndef FS_CD_MAX_FILES
#define FS_CD_MAX_FILES 8
#endif

/** \brief  The maximum number of romdisk files that can be open at a time. */
#ifndef FS_ROMDISK_MAX_FILES
#define FS_ROMDISK_MAX_FILES 16
#endif

/** \brief  The maximum number of ramdisk files that can be open at a time. */
#ifndef FS_RAMDISK_MAX_FILES
#define FS_RAMDISK_MAX_FILES 8
#endif

/** @} */

__END_DECLS

#endif /* !__KOS_OPTS_H */
