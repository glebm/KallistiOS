/* KallistiOS ##version##

   dc/pvr.h
   Copyright (C) 2002 Megan Potter
   Copyright (C) 2014 Lawrence Sebald
   Copyright (C) 2023 Ruslan Rostovtsev
   Copyright (C) 2024 Falco Girgis

   Low-level PVR 3D interface for the DC
*/

/** \file    dc/pvr.h
    \brief   Low-level PVR (3D hardware) interface.
    \ingroup pvr

    This file provides support for using the PVR 3D hardware in the Dreamcast.
    Note that this does not handle any sort of perspective transformations or
    anything of the like. This is just a very thin wrapper around the actual
    hardware support.

    This file is used for pretty much everything related to the PVR, from memory
    management to actual primitive rendering.

    \todo 
        - Auxiliary accumulation buffer
        - RTT texture sizes
        - Multipass rendering
        - pvr_poly_ctx_t::specular2
            - specular enabled within modifier?
        - Missing fog mode
        - Global "Intensity" light mode?
        - PVR_MIPBIAS_0 
            - Does it exist? What happens if set to 0?

    \author Megan Potter
    \author Roger Cattermole
    \author Paul Boese
    \author Brian Paul
    \author Lawrence Sebald
    \author Benoit Miller
    \author Ruslan Rostovtsev
    \author Falco Girgis
*/

#ifndef __DC_PVR_H
#define __DC_PVR_H

#include <stdint.h>
#include <stdbool.h>

#include <sys/cdefs.h>
__BEGIN_DECLS

#include <arch/memory.h>
#include <arch/types.h>
#include <arch/cache.h>
#include <dc/sq.h>

#include <dc/pvr/pvr_regs.h>
#include <dc/pvr/pvr_mem.h>
#include <dc/pvr/pvr_texture.h>
#include <dc/pvr/pvr_fog.h>
#include <dc/pvr/pvr_palette.h>

/** \defgroup pvr   PowerVR API
    \brief          Low-level PowerVR GPU Driver.
    \ingroup        video
*/

/** \brief   PVR Primitive List Type
    \ingroup pvr_lists

    Each primitive processed by the PVR is submitted to one of the hardware
    primitive lists. There is a list for each primitive type. 
    
    This type is an identifier for the different kinds of primitive lists.
*/
typedef enum pvr_list {
    PVR_LIST_OP_POLY = 0,  /**< \brief Opaque polygon list */
    PVR_LIST_OP_MOD  = 1,  /**< \brief Opaque modifier list */
    PVR_LIST_TR_POLY = 2,  /**< \brief Translucent polygon list */
    PVR_LIST_TR_MOD  = 3,  /**< \brief Translucent modifier list*/
    PVR_LIST_PT_POLY = 4,  /**< \brief Punch-thru polygon list */
    PVR_LIST_COUNT   = 5   /**< \brief Number of list types */
} pvr_list_t;

/** \defgroup pvr_geometry Geometry
    \brief                 PVR API for managing scene geometry
    \ingroup               pvr
*/

/** \defgroup pvr_primitives Primitives
    \brief                   Polygon and sprite management
    \ingroup                 pvr_geometry
*/

/** \defgroup pvr_ctx Contexts
    \brief            User-friendly intermittent primitive representation
    \ingroup          pvr_primitives

    PVR Contexts are user-friendly, intermittent representations of primitive
    headers. Their members are filled in with the properties of the desired
    primitives, then they are compiled into headers which can be directly
    submitted to the PVR.

    \warning
    It is highly recommended that you cache the primitive headers that get
    compiled from these intermediate context types, so that you are not
    wasting time dynamically regenerating headers from contexts each frame.
*/

/** \defgroup pvr_ctx_attrib Attributes
    \brief                   PVR primitive context attributes
    \ingroup                 pvr_ctx
*/

/** \brief                          PowerVR primitive context shading modes
    \ingroup                        pvr_ctx_attrib

    Each polygon can define how it wants to be shaded, be it with flat or
    Gouraud shading using these constants in the appropriate place in its
    pvr_poly_cxt_t.
*/
typedef enum __packed pvr_shade {
    PVR_SHADE_FLAT   = 0, /**< \brief Use flat shading */
    PVR_SHADE_GOURAD = 1  /**< \brief Use Gouraud shading */
} pvr_shade_t;

/** \defgroup pvr_ctx_depth     Depth
    \brief                      Depth attributes for PVR polygon contexts
    \ingroup                    pvr_ctx_attrib
*/

/** \brief                      PowerVR depth comparison modes
    \ingroup                    pvr_ctx_depth

    These set the depth function used for comparisons.
*/
typedef enum __packed pvr_depth {
    PVR_DEPTHCMP_NEVER    = 0,   /**< \brief Never pass */
    PVR_DEPTHCMP_LESS     = 1,   /**< \brief Less than */
    PVR_DEPTHCMP_EQUAL    = 2,   /**< \brief Equal to */
    PVR_DEPTHCMP_LEQUAL   = 3,   /**< \brief Less than or equal to */
    PVR_DEPTHCMP_GREATER  = 4,   /**< \brief Greater than */
    PVR_DEPTHCMP_NOTEQUAL = 5,   /**< \brief Not equal to */
    PVR_DEPTHCMP_GEQUAL   = 6,   /**< \brief Greater than or equal to */
    PVR_DEPTHCMP_ALWAYS   = 7    /**< \brief Always pass */
} pvr_depth_t;

/** \brief      PowerVR primitive context culling modes
    \ingroup    pvr_ctx_attrib

    These culling modes can be set by polygons to determine when they are
    culled. They work pretty much as you'd expect them to if you've ever used
    any 3D hardware before.
*/
typedef enum __packed pvr_cull {
    PVR_CULLING_NONE  = 0,   /**< \brief Disable culling */
    PVR_CULLING_SMALL = 1,   /**< \brief Cull if small */
    PVR_CULLING_CCW   = 2,   /**< \brief Cull if counterclockwise */
    PVR_CULLING_CW    = 3    /**< \brief Cull if clockwise */
} pvr_cull_t;

/** \defgroup pvr_depth_switch      Write Toggle
    \brief                          Enable or Disable Depth Writes.
    \ingroup                        pvr_ctx_depth
    @{
*/
#define PVR_DEPTHWRITE_ENABLE   0   /**< \brief Update the Z value */
#define PVR_DEPTHWRITE_DISABLE  1   /**< \brief Do not update the Z value */
/** @} */

/** \defgroup pvr_ctx_texture Texture
    \brief                    Texture attributes for PVR polygon contexts
    \ingroup                  pvr_ctx_attrib
*/

/** \defgroup pvr_txr_switch        Toggle
    \brief                          Enable or Disable Texturing on Polygons.
    \ingroup                        pvr_ctx_texture
    
    @{
*/
#define PVR_TEXTURE_DISABLE     0   /**< \brief Disable texturing */
#define PVR_TEXTURE_ENABLE      1   /**< \brief Enable texturing */
/** @} */

/** \defgroup pvr_blend             Blending
    \brief                          Blending attributes for PVR primitive contexts
    \ingroup                        pvr_ctx_attrib
*/

/** \brief                          Blending modes for PowerVR primitive contexts
    \ingroup                        pvr_blend

    These are all the blending modes that can be done with regard to alpha
    blending on the PVR.
*/
typedef enum __packed pvr_blend {
    PVR_BLEND_ZERO         = 0,   /**< \brief None of this color */
    PVR_BLEND_ONE          = 1,   /**< \brief All of this color */
    PVR_BLEND_DESTCOLOR    = 2,   /**< \brief Destination color */
    PVR_BLEND_INVDESTCOLOR = 3,   /**< \brief Inverse of destination color */
    PVR_BLEND_SRCALPHA     = 4,   /**< \brief Blend with source alpha */
    PVR_BLEND_INVSRCALPHA  = 5,   /**< \brief Blend with inverse source alpha */
    PVR_BLEND_DESTALPHA    = 6,   /**< \brief Blend with destination alpha */
    PVR_BLEND_INVDESTALPHA = 7    /**< \brief Blend with inverse destination alpha */
} pvr_blend_t;

/** \defgroup pvr_blend_switch      Blending Toggle
    \brief                          Enable or Disable Blending.
    \ingroup                        pvr_blend
    
    @{
*/
#define PVR_BLEND_DISABLE       0   /**< \brief Disable blending */
#define PVR_BLEND_ENABLE        1   /**< \brief Enable blending */
/** @} */

/** \brief                          PowerVR primitive context fog modes
    \ingroup                        pvr_ctx_attrib

    Each polygon can decide what fog type is used with regard to it using these
    constants in its pvr_poly_cxt_t.
*/
typedef enum __packed pvr_fog {
    PVR_FOG_TABLE   = 0,   /**< \brief Table fog */
    PVR_FOG_VERTEX  = 1,   /**< \brief Vertex fog */
    PVR_FOG_DISABLE = 2,   /**< \brief Disable fog */
    PVR_FOG_TABLE2  = 3    /**< \brief Table fog mode 2 */
} pvr_fog_t;

/** \brief                          PowerVR primitive context clipping modes
    \ingroup                        pvr_ctx_attrib

    These control how primitives are clipped against the user clipping area.
*/
typedef enum __packed pvr_clip {
    PVR_USERCLIP_DISABLE = 0,   /**< \brief Disable clipping */
    PVR_USERCLIP_INSIDE  = 2,   /**< \brief Enable clipping inside area */
    PVR_USERCLIP_OUTSIDE = 3    /**< \brief Enable clipping outside area */
} pvr_clip_t;

/** \defgroup pvr_ctx_color     Color
    \brief                      Color attributes for PowerVR primitive contexts
    \ingroup                    pvr_ctx_attrib
*/

/** \defgroup pvr_colclamp_switch   Clamping Toggle
    \brief                          Enable or Disable Color Clamping
    \ingroup                        pvr_ctx_color

    Enabling color clamping will clamp colors between the minimum and maximum
    values before any sort of fog processing.

    @{
*/
#define PVR_CLRCLAMP_DISABLE    0   /**< \brief Disable color clamping */
#define PVR_CLRCLAMP_ENABLE     1   /**< \brief Enable color clamping */
/** @} */

/** \defgroup pvr_offset_switch     Offset Toggle
    \brief                          Enable or Disable Offset Color
    \ingroup                        pvr_ctx_color

    Enabling offset color calculation allows for "specular" like effects on a
    per-vertex basis, by providing an additive color in the calculation of the
    final pixel colors. In vertex types with a "oargb" parameter, that's what it
    is for.

    \note
    This must be enabled for bumpmap polygons in order to allow you to
    specify the parameters in the oargb field of the vertices.

    @{
*/
#define PVR_SPECULAR_DISABLE    0   /**< \brief Disable offset colors */
#define PVR_SPECULAR_ENABLE     1   /**< \brief Enable offset colors */
/** @} */

/** \defgroup pvr_alpha_switch      Alpha Toggle
    \brief                          Enable or Disable Alpha Blending
    \ingroup                        pvr_blend

    This causes the alpha value in the vertex color to be paid attention to. It
    really only makes sense to enable this for translucent or punch-thru polys.

    @{
*/
#define PVR_ALPHA_DISABLE       0   /**< \brief Disable alpha blending */
#define PVR_ALPHA_ENABLE        1   /**< \brief Enable alpha blending */
/** @} */

/** \defgroup pvr_txralpha_switch   Alpha Toggle
    \brief                          Enable or Disable Texture Alpha Blending
    \ingroup                        pvr_ctx_texture

    This causes the alpha value in the texel color to be paid attention to. It
    really only makes sense to enable this for translucent or punch-thru polys.

    @{
*/
#define PVR_TXRALPHA_ENABLE     0   /**< \brief Enable alpha blending */
#define PVR_TXRALPHA_DISABLE    1   /**< \brief Disable alpha blending */
/** @} */

/** \brief     Texture U/V flipping mode for the PVR
    \ingroup   pvr_ctx_texture

    These flags determine what happens when U/V coordinate values exceed 1.0.
    In any of the flipped cases, the specified coordinate value will flip around
    after 1.0, essentially mirroring the image. So, if you displayed an image
    with a U coordinate of 0.0 on the left hand side and 2.0 on the right hand
    side with U flipping turned on, you'd have an image that was displayed twice
    as if mirrored across the middle. This mirroring behavior happens at every
    unit boundary (so at 2.0 it returns to normal, at 3.0 it flips, etc).

    The default case is to disable mirroring. In addition, clamping of the U/V
    coordinates by PVR_UVCLAMP_U, PVR_UVCLAMP_V, or PVR_UVCLAMP_UV will disable
    the mirroring behavior.
*/
typedef enum __packed pvr_uv_flip {
    PVR_UVFLIP_NONE = 0,  /**< \brief No flipped coordinates */
    PVR_UVFLIP_V    = 1,  /**< \brief Flip V only */
    PVR_UVFLIP_U    = 2,  /**< \brief Flip U only */
    PVR_UVFLIP_UV   = 3   /**< \brief Flip U and V */
} pvr_uv_flip_t;

/** \brief   Enable or disable clamping of U/V on the PVR
    \ingroup pvr_ctx_texture

    These flags determine whether clamping will be applied to U/V coordinate
    values that exceed 1.0. If enabled, these modes will explicitly override the
    flip/mirroring modes (PVR_UVFLIP_U, PVR_UVFLIP_V, and PVR_UVFLIP_UV), and
    will instead ensure that the coordinate(s) in question never exceed 1.0.
*/
typedef enum __packed pvr_uv_clamp {
    PVR_UVCLAMP_NONE = 0,  /**< \brief Disable clamping */
    PVR_UVCLAMP_V    = 1,  /**< \brief Clamp V only */
    PVR_UVCLAMP_U    = 2,  /**< \brief Clamp U only */
    PVR_UVCLAMP_UV   = 3   /**< \brief Clamp U and V */
} pvr_uv_clamp_t;

/** \brief    PowerVR texture sampling modes
    \ingroup  pvr_ctx_texture
*/
typedef enum __packed pvr_filter {
    PVR_FILTER_NONE       = 0,   /**< \brief No filtering (point sample) */
    PVR_FILTER_NEAREST    = 0,   /**< \brief No filtering (point sample) */
    PVR_FILTER_BILINEAR   = 2,   /**< \brief Bilinear interpolation */
    PVR_FILTER_TRILINEAR1 = 4,   /**< \brief Trilinear interpolation pass 1 */
    PVR_FILTER_TRILINEAR2 = 6    /**< \brief Trilinear interpolation pass 2 */
} pvr_filter_t;

/** \brief    Mipmap bias modes for PowerVR primitive contexts
    \ingroup  pvr_ctx_texture
*/
typedef enum __packed pvr_mip_bias {
    PVR_MIPBIAS_NORMAL = 4,    /* txr_mipmap_bias */
    PVR_MIPBIAS_0_25   = 1,
    PVR_MIPBIAS_0_50   = 2,
    PVR_MIPBIAS_0_75   = 3,
    PVR_MIPBIAS_1_00   = PVR_MIPBIAS_NORMAL,
    PVR_MIPBIAS_1_25   = 5,
    PVR_MIPBIAS_1_50   = 6,
    PVR_MIPBIAS_1_75   = 7,
    PVR_MIPBIAS_2_00   = 8,
    PVR_MIPBIAS_2_25   = 9,
    PVR_MIPBIAS_2_50   = 10,
    PVR_MIPBIAS_2_75   = 11,
    PVR_MIPBIAS_3_00   = 12,
    PVR_MIPBIAS_3_25   = 13,
    PVR_MIPBIAS_3_50   = 14,
    PVR_MIPBIAS_3_75   = 15
} pvr_mip_bias_t;

/** \brief     PowerVR texture color calculation modes
    \ingroup   pvr_ctx_texture
*/
typedef enum __packed pvr_txr_env {
    PVR_TXRENV_REPLACE       = 0,   /**< \brief C = Ct, A = At */
    PVR_TXRENV_MODULATE      = 1,   /**< \brief C = Cs * Ct, A = At */
    PVR_TXRENV_DECAL         = 2,   /**< \brief C = (Cs * At) + (Cs * (1-At)), A = As */
    PVR_TXRENV_MODULATEALPHA = 3    /**< \brief C = Cs * Ct, A = As * At */
} pvr_txr_env_t;

/** \defgroup pvr_mip_switch        Mipmap Toggle
    \brief                          Enable or Disable Mipmap Processing
    \ingroup                        pvr_ctx_texture

    @{
*/
#define PVR_MIPMAP_DISABLE      0   /**< \brief Disable mipmap processing */
#define PVR_MIPMAP_ENABLE       1   /**< \brief Enable mipmap processing */
/** @} */

/** \defgroup pvr_txr_fmts          Formats
    \brief                          PowerVR texture formats
    \ingroup                        pvr_txr_mgmt

    These are the texture formats that the PVR supports. Note that some of
    these, you can OR together with other values.

    @{
*/
#define PVR_TXRFMT_NONE         0           /**< \brief No texture */
#define PVR_TXRFMT_VQ_DISABLE   (0 << 30)   /**< \brief Not VQ encoded */
#define PVR_TXRFMT_VQ_ENABLE    (1 << 30)   /**< \brief VQ encoded */
#define PVR_TXRFMT_ARGB1555     (0 << 27)   /**< \brief 16-bit ARGB1555 */
#define PVR_TXRFMT_RGB565       (1 << 27)   /**< \brief 16-bit RGB565 */
#define PVR_TXRFMT_ARGB4444     (2 << 27)   /**< \brief 16-bit ARGB4444 */
#define PVR_TXRFMT_YUV422       (3 << 27)   /**< \brief YUV422 format */
#define PVR_TXRFMT_BUMP         (4 << 27)   /**< \brief Bumpmap format */
#define PVR_TXRFMT_PAL4BPP      (5 << 27)   /**< \brief 4BPP paletted format */
#define PVR_TXRFMT_PAL8BPP      (6 << 27)   /**< \brief 8BPP paletted format */
#define PVR_TXRFMT_TWIDDLED     (0 << 26)   /**< \brief Texture is twiddled */
#define PVR_TXRFMT_NONTWIDDLED  (1 << 26)   /**< \brief Texture is not twiddled */
#define PVR_TXRFMT_NOSTRIDE     (0 << 21)   /**< \brief Texture is not strided */
#define PVR_TXRFMT_STRIDE       (1 << 21)   /**< \brief Texture is strided */

/* OR one of these into your texture format if you need it. Note that
   these coincide with the twiddled/stride bits, so you can't have a
   non-twiddled/strided texture that's paletted! */

/** \brief   8BPP palette selector

    \param  x               The palette index */
#define PVR_TXRFMT_8BPP_PAL(x)  ((x) << 25)

/** \brief   4BPP palette selector

    \param  x               The palette index */
#define PVR_TXRFMT_4BPP_PAL(x)  ((x) << 21)
/** @} */

/** \brief                          Color formats for PowerVR vertices
    \ingroup                        pvr_ctx_color

    These control how colors are represented in polygon data.
*/
typedef enum __packed pvr_color {
    PVR_CLRFMT_ARGBPACKED     = 0,  /**< \brief 32-bit integer ARGB */
    PVR_CLRFMT_4FLOATS        = 1,  /**< \brief 4 floating point values */
    PVR_CLRFMT_INTENSITY      = 2,  /**< \brief Intensity color */
    PVR_CLRFMT_INTENSITY_PREV = 3   /**< \brief Use last intensity */
} pvr_color_t;

/** \brief                          U/V data format for PVR textures
    \ingroup                        pvr_ctx_texture
*/
typedef enum __packed pvr_uv {
    PVR_UVFMT_32BIT = 0,  /**< \brief 32-bit floating point U/V */
    PVR_UVFMT_16BIT = 1   /**< \brief 16-bit floating point U/V */
} pvr_uv_t;
/** @} */

/** \defgroup pvr_ctx_modvol        Modifier Volumes
    \brief                          PowerVR modifier volume polygon context attributes
    \ingroup                        pvr_ctx_attrib
*/

/** \defgroup pvr_mod_switch        Toggle
    \brief                          Enable or Disable Modifier Effects
    \ingroup                        pvr_ctx_modvol
    @{
*/
#define PVR_MODIFIER_DISABLE    0   /**< \brief Disable modifier effects */
#define PVR_MODIFIER_ENABLE     1   /**< \brief Enable modifier effects */
/** @} */

/** \defgroup pvr_mod_types         Types
    \brief                          Modifier volume types for PowerVR primitive contexts
    \ingroup                        pvr_ctx_modvol
    @{
*/
#define PVR_MODIFIER_CHEAP_SHADOW   0
#define PVR_MODIFIER_NORMAL         1
/** @} */

/** \brief    Modifier volume modes for PowerVR primitive contexts
    \ingroup  pvr_ctx_modvol

    All triangles in a single modifier volume should be of the other poly type,
    except for the last one. That should be either of the other two types,
    depending on whether you want an inclusion or exclusion volume.
*/
typedef enum __packed pvr_mod {
    PVR_MODIFIER_OTHER_POLY        = 0,  /**< \brief Not the last polygon in the volume */
    PVR_MODIFIER_INCLUDE_LAST_POLY = 1,  /**< \brief Last polygon, inclusion volume */
    PVR_MODIFIER_EXCLUDE_LAST_POLY = 2   /**< \brief Last polygon, exclusion volume */
} pvr_mod_t;

/** \brief   PVR polygon context.
    \ingroup pvr_ctx

    Human-readable intermediate representation of a PVR polygon header. This
    structure should be filled in depending on the desired polygon properties
    then compiled into a pvr_poly_hdr_t, which can be directly submitted.

    \sa pvr_sprite_ctx_t, pvr_poly_hdr_t
*/
typedef struct __packed pvr_poly_cxt {
    pvr_list_t        list_type;     /**< List type to submit */
    struct __packed {
        bool          alpha;         /**< Enable alpha outside modifier */
        bool          shading;       /**< Enable gourad shading */
        pvr_fog_t     fog_type;      /**< Fog type outside modifier */
        pvr_cull_t    culling;       /**< Culling mode */
        bool          color_clamp;   /**< Enable color clamping outside of modifer */
        pvr_clip_t    clip_mode;     /**< Clipping mode */
        pvr_mod_t     modifier_mode; /**< Modifier mode */
        bool          specular;      /**< Enable offset color outside of modifier */
        bool          alpha2;        /**< Enable alpha inside modifier */
        pvr_fog_t     fog_type2;     /**< Fog type inside modifier */
        bool          color_clamp2;  /**< Enable color clamping inside modifier */
    } gen;                           /**< General parameters */
    struct __packed {
        pvr_blend_t    src;          /**< Source blending mode outside modifier */
        pvr_blend_t    dst;          /**< Dest blending mode outside modifier */
        bool           src_enable;   /**< Source blending enable outside modifier */
        bool           dst_enable;   /**< Dest blending enable outside modifier */
        pvr_blend_t    src2;         /**< Source blending mode inside modifier */
        pvr_blend_t    dst2;         /**< Dest blending mode inside modifier */
        bool           src_enable2;  /**< Source blending enable inside modifier */
        bool           dst_enable2;  /**< Dest blending enable inside modifier */
    } blend;                         /**< Blending parameters */
    struct __packed {
        pvr_color_t    color;        /**< Color format in vertex */
        pvr_uv_t       uv;           /**< U/V data format in vertex */
        bool           modifier;     /**< Enable or disable modifier effect */
    } fmt;                           /**< Format control */
    struct __packed {
        pvr_depth_t    comparison;   /**< Depth comparison mode */
        bool           write;        /**< Enable depth writes */
    } depth;                         /**< Depth comparison/write modes */
    struct __packed {
        bool           enable;       /**< Enable/disable texturing */
        pvr_filter_t   filter;       /**< Filtering mode */
        bool           mipmap;       /**< Enable/disable mipmaps */
        pvr_mip_bias_t mipmap_bias;  /**< Mipmap bias */
        pvr_uv_flip_t  uv_flip;      /**< Enable/disable U/V flipping */
        pvr_uv_clamp_t uv_clamp;     /**< Enable/disable U/V clamping */
        bool           alpha;        /**< Enable/disable texture alpha */
        pvr_txr_env_t  env;          /**< Texture color contribution */
        size_t         width;        /**< Texture width (requires a power of 2) */
        size_t         height;       /**< Texture height (requires a power of 2) */
        int            format;       /**< Texture format */
        pvr_ptr_t      base;         /**< Texture pointer */
    } txr,                           /**< Texturing params outside modifier */
      txr2;                          /**< Texturing params inside modifier */
} pvr_poly_cxt_t;

/** \brief   PVR sprite context.
    \ingroup pvr_ctx

    You should use this more human readable format for specifying your sprite
    contexts, and then compile them into sprite headers when you are ready to
    start using them.
*/
typedef struct pvr_sprite_ctx {
    int     list_type;          /**< \brief Primitive list
                                     \see   pvr_lists */
    struct {
        int     alpha;          /**< \brief Enable or disable alpha
                                     \see   pvr_alpha_switch */
        int     fog_type;       /**< \brief Fog type
                                     \see   pvr_fog_types */
        int     culling;        /**< \brief Culling mode
                                     \see   pvr_cull_modes */
        int     color_clamp;    /**< \brief Color clamp enable/disable
                                     \see   pvr_colclamp_switch */
        int     clip_mode;      /**< \brief Clipping mode
                                     \see   pvr_clip_modes */
        int     specular;       /**< \brief Offset color enable/disable
                                     \see   pvr_offset_switch */
    } gen;                      /**< \brief General parameters */
    struct {
        int     src;            /**< \brief Source blending mode
                                     \see   pvr_blend_modes */
        int     dst;            /**< \brief Dest blending mode
                                     \see   pvr_blend_modes */
        int     src_enable;     /**< \brief Source blending enable
                                     \see   pvr_blend_switch */
        int     dst_enable;     /**< \brief Dest blending enable
                                     \see   pvr_blend_switch */
    } blend;
    struct {
        int     comparison;     /**< \brief Depth comparison mode
                                     \see pvr_depth_modes */
        int     write;          /**< \brief Enable or disable depth writes
                                     \see pvr_depth_switch */
    } depth;                    /**< \brief Depth comparison/write modes */
    struct {
        int     enable;         /**< \brief Enable/disable texturing
                                     \see   pvr_txr_switch */
        int     filter;         /**< \brief Filtering mode
                                     \see   pvr_filter_modes */
        int     mipmap;         /**< \brief Enable/disable mipmaps
                                     \see   pvr_mip_switch */
        int     mipmap_bias;    /**< \brief Mipmap bias
                                     \see   pvr_mip_bias */
        int     uv_flip;        /**< \brief Enable/disable U/V flipping
                                     \see   pvr_uv_flip */
        int     uv_clamp;       /**< \brief Enable/disable U/V clamping
                                     \see   pvr_uv_clamp */
        int     alpha;          /**< \brief Enable/disable texture alpha
                                     \see   pvr_txralpha_switch */
        int     env;            /**< \brief Texture color contribution
                                     \see   pvr_txrenv_modes */
        int     width;          /**< \brief Texture width (requires a power of 2) */
        int     height;         /**< \brief Texture height (requires a power of 2) */
        int     format;         /**< \brief Texture format
                                     \see   pvr_txr_fmts */
        pvr_ptr_t base;         /**< \brief Texture pointer */
    } txr;                      /**< \brief Texturing params */
} pvr_sprite_cxt_t;

/** \defgroup pvr_primitives_headers Headers
    \brief                           Compiled headers for polygons and sprites
    \ingroup pvr_primitives

    @{
*/

/** \brief   PVR polygon header.

    This is the hardware equivalent of a rendering context; you'll create one of
    these from your pvr_poly_cxt_t and use it for submission to the hardware.
*/
typedef struct pvr_poly_hdr {
    uint32_t cmd;                /**< \brief TA command */
    uint32_t mode1;              /**< \brief Parameter word 1 */
    uint32_t mode2;              /**< \brief Parameter word 2 */
    uint32_t mode3;              /**< \brief Parameter word 3 */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
    uint32_t d3;                 /**< \brief Dummy value */
    uint32_t d4;                 /**< \brief Dummy value */
} pvr_poly_hdr_t;

/** \brief   PVR polygon header with intensity color.

    This is the equivalent of pvr_poly_hdr_t, but for use with intensity color.
*/
typedef struct pvr_poly_ic_hdr {
    uint32_t cmd;                /**< \brief TA command */
    uint32_t mode1;              /**< \brief Parameter word 1 */
    uint32_t mode2;              /**< \brief Parameter word 2 */
    uint32_t mode3;              /**< \brief Parameter word 3 */
    float   a;                   /**< \brief Face color alpha component */
    float   r;                   /**< \brief Face color red component */
    float   g;                   /**< \brief Face color green component */
    float   b;                   /**< \brief Face color blue component */
} pvr_poly_ic_hdr_t;

/** \brief   PVR polygon header to be used with modifier volumes.

    This is the equivalent of a pvr_poly_hdr_t for use when a polygon is to be
    used with modifier volumes.
*/
typedef struct pvr_poly_mod_hdr {
    uint32_t cmd;                /**< \brief TA command */
    uint32_t mode1;              /**< \brief Parameter word 1 */
    uint32_t mode2_0;            /**< \brief Parameter word 2 (outside volume) */
    uint32_t mode3_0;            /**< \brief Parameter word 3 (outside volume) */
    uint32_t mode2_1;            /**< \brief Parameter word 2 (inside volume) */
    uint32_t mode3_1;            /**< \brief Parameter word 3 (inside volume) */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
} pvr_poly_mod_hdr_t;

/** \brief   PVR polygon header specifically for sprites.

    This is the equivalent of a pvr_poly_hdr_t for use when a quad/sprite is to
    be rendered. Note that the color data is here, not in the vertices.
*/
typedef struct pvr_sprite_hdr {
    uint32_t cmd;                /**< \brief TA command */
    uint32_t mode1;              /**< \brief Parameter word 1 */
    uint32_t mode2;              /**< \brief Parameter word 2 */
    uint32_t mode3;              /**< \brief Parameter word 3 */
    uint32_t argb;               /**< \brief Sprite face color */
    uint32_t oargb;              /**< \brief Sprite offset color */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
} pvr_sprite_hdr_t;

/** \brief   Modifier volume header.

    This is the header that should be submitted when dealing with setting a
    modifier volume.
*/
typedef struct pvr_mod_hdr {
    uint32_t cmd;                /**< \brief TA command */
    uint32_t mode1;              /**< \brief Parameter word 1 */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
    uint32_t d3;                 /**< \brief Dummy value */
    uint32_t d4;                 /**< \brief Dummy value */
    uint32_t d5;                 /**< \brief Dummy value */
    uint32_t d6;                 /**< \brief Dummy value */
} pvr_mod_hdr_t;

/** @} */

/** \defgroup pvr_vertex_types  Vertices
    \brief                      PowerVR vertex types
    \ingroup                    pvr_geometry

    @{
*/

/** \brief   Generic PVR vertex type.

    The PVR chip itself supports many more vertex types, but this is the main
    one that can be used with both textured and non-textured polygons, and is
    fairly fast.
*/
typedef struct pvr_vertex {
    uint32_t flags;              /**< \brief TA command (vertex flags) */
    float   x;                   /**< \brief X coordinate */
    float   y;                   /**< \brief Y coordinate */
    float   z;                   /**< \brief Z coordinate */
    float   u;                   /**< \brief Texture U coordinate */
    float   v;                   /**< \brief Texture V coordinate */
    uint32_t argb;               /**< \brief Vertex color */
    uint32_t oargb;              /**< \brief Vertex offset color */
} pvr_vertex_t;

/** \brief   PVR vertex type: Non-textured, packed color, affected by modifier
             volume.

    This vertex type has two copies of colors. The second color is used when
    enclosed within a modifier volume.
*/
typedef struct pvr_vertex_pcm {
    uint32_t flags;              /**< \brief TA command (vertex flags) */
    float   x;                   /**< \brief X coordinate */
    float   y;                   /**< \brief Y coordinate */
    float   z;                   /**< \brief Z coordinate */
    uint32_t argb0;              /**< \brief Vertex color (outside volume) */
    uint32_t argb1;              /**< \brief Vertex color (inside volume) */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
} pvr_vertex_pcm_t;

/** \brief   PVR vertex type: Textured, packed color, affected by modifier volume.

    Note that this vertex type has two copies of colors, offset colors, and
    texture coords. The second set of texture coords, colors, and offset colors
    are used when enclosed within a modifier volume.
*/
typedef struct pvr_vertex_tpcm {
    uint32_t flags;              /**< \brief TA command (vertex flags) */
    float   x;                  /**< \brief X coordinate */
    float   y;                  /**< \brief Y coordinate */
    float   z;                  /**< \brief Z coordinate */
    float   u0;                 /**< \brief Texture U coordinate (outside) */
    float   v0;                 /**< \brief Texture V coordinate (outside) */
    uint32_t argb0;              /**< \brief Vertex color (outside) */
    uint32_t oargb0;             /**< \brief Vertex offset color (outside) */
    float   u1;                 /**< \brief Texture U coordinate (inside) */
    float   v1;                 /**< \brief Texture V coordinate (inside) */
    uint32_t argb1;              /**< \brief Vertex color (inside) */
    uint32_t oargb1;             /**< \brief Vertex offset color (inside) */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
    uint32_t d3;                 /**< \brief Dummy value */
    uint32_t d4;                 /**< \brief Dummy value */
} pvr_vertex_tpcm_t;

/** \brief   PVR vertex type: Textured sprite.

    This vertex type is to be used with the sprite polygon header and the sprite
    related commands to draw textured sprites. Note that there is no fourth Z
    coordinate. I suppose it just gets interpolated?

    The U/V coordinates in here are in the 16-bit per coordinate form. Also,
    like the fourth Z value, there is no fourth U or V, so it must get
    interpolated from the others.
*/
typedef struct pvr_sprite_txr {
    uint32_t flags;               /**< \brief TA command (vertex flags) */
    float   ax;                   /**< \brief First X coordinate */
    float   ay;                   /**< \brief First Y coordinate */
    float   az;                   /**< \brief First Z coordinate */
    float   bx;                   /**< \brief Second X coordinate */
    float   by;                   /**< \brief Second Y coordinate */
    float   bz;                   /**< \brief Second Z coordinate */
    float   cx;                   /**< \brief Third X coordinate */
    float   cy;                   /**< \brief Third Y coordinate */
    float   cz;                   /**< \brief Third Z coordinate */
    float   dx;                   /**< \brief Fourth X coordinate */
    float   dy;                   /**< \brief Fourth Y coordinate */
    uint32_t dummy;               /**< \brief Dummy value */
    uint32_t auv;                 /**< \brief First U/V texture coordinates */
    uint32_t buv;                 /**< \brief Second U/V texture coordinates */
    uint32_t cuv;                 /**< \brief Third U/V texture coordinates */
} pvr_sprite_txr_t;

/** \brief   PVR vertex type: Untextured sprite.

    This vertex type is to be used with the sprite polygon header and the sprite
    related commands to draw untextured sprites (aka, quads).
*/
typedef struct pvr_sprite_col {
    uint32_t flags;              /**< \brief TA command (vertex flags) */
    float   ax;                  /**< \brief First X coordinate */
    float   ay;                  /**< \brief First Y coordinate */
    float   az;                  /**< \brief First Z coordinate */
    float   bx;                  /**< \brief Second X coordinate */
    float   by;                  /**< \brief Second Y coordinate */
    float   bz;                  /**< \brief Second Z coordinate */
    float   cx;                  /**< \brief Third X coordinate */
    float   cy;                  /**< \brief Third Y coordinate */
    float   cz;                  /**< \brief Third Z coordinate */
    float   dx;                  /**< \brief Fourth X coordinate */
    float   dy;                  /**< \brief Fourth Y coordinate */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
    uint32_t d3;                 /**< \brief Dummy value */
    uint32_t d4;                 /**< \brief Dummy value */
} pvr_sprite_col_t;

/** \brief   PVR vertex type: Modifier volume.

    This vertex type is to be used with the modifier volume header to specify
    triangular modifier areas.
*/
typedef struct pvr_modifier_vol {
    uint32_t flags;              /**< \brief TA command (vertex flags) */
    float   ax;                  /**< \brief First X coordinate */
    float   ay;                  /**< \brief First Y coordinate */
    float   az;                  /**< \brief First Z coordinate */
    float   bx;                  /**< \brief Second X coordinate */
    float   by;                  /**< \brief Second Y coordinate */
    float   bz;                  /**< \brief Second Z coordinate */
    float   cx;                  /**< \brief Third X coordinate */
    float   cy;                  /**< \brief Third Y coordinate */
    float   cz;                  /**< \brief Third Z coordinate */
    uint32_t d1;                 /**< \brief Dummy value */
    uint32_t d2;                 /**< \brief Dummy value */
    uint32_t d3;                 /**< \brief Dummy value */
    uint32_t d4;                 /**< \brief Dummy value */
    uint32_t d5;                 /**< \brief Dummy value */
    uint32_t d6;                 /**< \brief Dummy value */
} pvr_modifier_vol_t;

/** \brief   Pack four floating point color values into a 32-bit integer form.

    All of the color values should be between 0 and 1.

    \param  a               Alpha value
    \param  r               Red value
    \param  g               Green value
    \param  b               Blue value
    \return                 The packed color value
*/
#define PVR_PACK_COLOR(a, r, g, b) ( \
                                     ( ((uint8)( a * 255 ) ) << 24 ) | \
                                     ( ((uint8)( r * 255 ) ) << 16 ) | \
                                     ( ((uint8)( g * 255 ) ) << 8 ) | \
                                     ( ((uint8)( b * 255 ) ) << 0 ) )

/** \brief   Pack two floating point coordinates into one 32-bit value,
             truncating them to 16-bits each.

    \param  u               First coordinate to pack
    \param  v               Second coordinate to pack
    \return                 The packed coordinates
*/
static inline uint32_t PVR_PACK_16BIT_UV(float u, float v) {
    union {
        float f;
        uint32_t i;
    } u2, v2;

    u2.f = u;
    v2.f = v;

    return (u2.i & 0xFFFF0000) | (v2.i >> 16);
}

/** @} */

/** \defgroup pvr_commands          TA Command Values
    \brief                          Command values for submitting data to the TA
    \ingroup                        pvr_primitives_headers

    These are are appropriate values for TA commands. Use whatever goes with the
    primitive type you're using.

    @{
*/
#define PVR_CMD_POLYHDR     0x80840000  /**< \brief PVR polygon header.
Striplength set to 2 */
#define PVR_CMD_VERTEX      0xe0000000  /**< \brief PVR vertex data */
#define PVR_CMD_VERTEX_EOL  0xf0000000  /**< \brief PVR vertex, end of strip */
#define PVR_CMD_USERCLIP    0x20000000  /**< \brief PVR user clipping area */
#define PVR_CMD_MODIFIER    0x80000000  /**< \brief PVR modifier volume */
#define PVR_CMD_SPRITE      0xA0000000  /**< \brief PVR sprite header */
/** @} */

/** \defgroup pvr_bitmasks          Constants and Masks
    \brief                          Polygon header constants and masks
    \ingroup                        pvr_primitives_headers

    Note that thanks to the arrangement of constants, this is mainly a matter of
    bit shifting to compile headers...

    @{
*/
#define PVR_TA_CMD_TYPE_SHIFT       24
#define PVR_TA_CMD_TYPE_MASK        (7 << PVR_TA_CMD_TYPE_SHIFT)

#define PVR_TA_CMD_USERCLIP_SHIFT   16
#define PVR_TA_CMD_USERCLIP_MASK    (3 << PVR_TA_CMD_USERCLIP_SHIFT)

#define PVR_TA_CMD_CLRFMT_SHIFT     4
#define PVR_TA_CMD_CLRFMT_MASK      (7 << PVR_TA_CMD_CLRFMT_SHIFT)

#define PVR_TA_CMD_SPECULAR_SHIFT   2
#define PVR_TA_CMD_SPECULAR_MASK    (1 << PVR_TA_CMD_SPECULAR_SHIFT)

#define PVR_TA_CMD_SHADE_SHIFT      1
#define PVR_TA_CMD_SHADE_MASK       (1 << PVR_TA_CMD_SHADE_SHIFT)

#define PVR_TA_CMD_UVFMT_SHIFT      0
#define PVR_TA_CMD_UVFMT_MASK       (1 << PVR_TA_CMD_UVFMT_SHIFT)

#define PVR_TA_CMD_MODIFIER_SHIFT   7
#define PVR_TA_CMD_MODIFIER_MASK    (1 <<  PVR_TA_CMD_MODIFIER_SHIFT)

#define PVR_TA_CMD_MODIFIERMODE_SHIFT   6
#define PVR_TA_CMD_MODIFIERMODE_MASK    (1 <<  PVR_TA_CMD_MODIFIERMODE_SHIFT)

#define PVR_TA_PM1_DEPTHCMP_SHIFT   29
#define PVR_TA_PM1_DEPTHCMP_MASK    (7 << PVR_TA_PM1_DEPTHCMP_SHIFT)

#define PVR_TA_PM1_CULLING_SHIFT    27
#define PVR_TA_PM1_CULLING_MASK     (3 << PVR_TA_PM1_CULLING_SHIFT)

#define PVR_TA_PM1_DEPTHWRITE_SHIFT 26
#define PVR_TA_PM1_DEPTHWRITE_MASK  (1 << PVR_TA_PM1_DEPTHWRITE_SHIFT)

#define PVR_TA_PM1_TXRENABLE_SHIFT  25
#define PVR_TA_PM1_TXRENABLE_MASK   (1 << PVR_TA_PM1_TXRENABLE_SHIFT)

#define PVR_TA_PM1_MODIFIERINST_SHIFT   29
#define PVR_TA_PM1_MODIFIERINST_MASK    (3 <<  PVR_TA_PM1_MODIFIERINST_SHIFT)

#define PVR_TA_PM2_SRCBLEND_SHIFT   29
#define PVR_TA_PM2_SRCBLEND_MASK    (7 << PVR_TA_PM2_SRCBLEND_SHIFT)

#define PVR_TA_PM2_DSTBLEND_SHIFT   26
#define PVR_TA_PM2_DSTBLEND_MASK    (7 << PVR_TA_PM2_DSTBLEND_SHIFT)

#define PVR_TA_PM2_SRCENABLE_SHIFT  25
#define PVR_TA_PM2_SRCENABLE_MASK   (1 << PVR_TA_PM2_SRCENABLE_SHIFT)

#define PVR_TA_PM2_DSTENABLE_SHIFT  24
#define PVR_TA_PM2_DSTENABLE_MASK   (1 << PVR_TA_PM2_DSTENABLE_SHIFT)

#define PVR_TA_PM2_FOG_SHIFT        22
#define PVR_TA_PM2_FOG_MASK     (3 << PVR_TA_PM2_FOG_SHIFT)

#define PVR_TA_PM2_CLAMP_SHIFT      21
#define PVR_TA_PM2_CLAMP_MASK       (1 << PVR_TA_PM2_CLAMP_SHIFT)

#define PVR_TA_PM2_ALPHA_SHIFT      20
#define PVR_TA_PM2_ALPHA_MASK       (1 << PVR_TA_PM2_ALPHA_SHIFT)

#define PVR_TA_PM2_TXRALPHA_SHIFT   19
#define PVR_TA_PM2_TXRALPHA_MASK    (1 << PVR_TA_PM2_TXRALPHA_SHIFT)

#define PVR_TA_PM2_UVFLIP_SHIFT     17
#define PVR_TA_PM2_UVFLIP_MASK      (3 << PVR_TA_PM2_UVFLIP_SHIFT)

#define PVR_TA_PM2_UVCLAMP_SHIFT    15
#define PVR_TA_PM2_UVCLAMP_MASK     (3 << PVR_TA_PM2_UVCLAMP_SHIFT)

#define PVR_TA_PM2_FILTER_SHIFT     12
#define PVR_TA_PM2_FILTER_MASK      (7 << PVR_TA_PM2_FILTER_SHIFT)

#define PVR_TA_PM2_MIPBIAS_SHIFT    8
#define PVR_TA_PM2_MIPBIAS_MASK     (15 << PVR_TA_PM2_MIPBIAS_SHIFT)

#define PVR_TA_PM2_TXRENV_SHIFT     6
#define PVR_TA_PM2_TXRENV_MASK      (3 << PVR_TA_PM2_TXRENV_SHIFT)

#define PVR_TA_PM2_USIZE_SHIFT      3
#define PVR_TA_PM2_USIZE_MASK       (7 << PVR_TA_PM2_USIZE_SHIFT)

#define PVR_TA_PM2_VSIZE_SHIFT      0
#define PVR_TA_PM2_VSIZE_MASK       (7 << PVR_TA_PM2_VSIZE_SHIFT)

#define PVR_TA_PM3_MIPMAP_SHIFT     31
#define PVR_TA_PM3_MIPMAP_MASK      (1 << PVR_TA_PM3_MIPMAP_SHIFT)

#define PVR_TA_PM3_TXRFMT_SHIFT     0
#define PVR_TA_PM3_TXRFMT_MASK      0xffffffff
/** @} */


/* Initialization ****************************************************/
/** \defgroup pvr_init  Initialization 
    \brief              Driver initialization and shutdown
    \ingroup            pvr

    Initialization and shutdown: stuff you should only ever have to do
    once in your program. 
*/

/** \defgroup pvr_binsizes          Primitive Bin Sizes
    \brief                          Available sizes for primitive bins
    \ingroup                        pvr_init
    @{
*/
#define PVR_BINSIZE_0   0   /**< \brief 0-length (disables the list) */
#define PVR_BINSIZE_8   8   /**< \brief 8-word (32-byte) length */
#define PVR_BINSIZE_16  16  /**< \brief 16-word (64-byte) length */
#define PVR_BINSIZE_32  32  /**< \brief 32-word (128-byte) length */
/** @} */

/** \brief   PVR initialization structure
    \ingroup pvr_init

    This structure defines how the PVR initializes various parts of the system,
    including the primitive bin sizes, the vertex buffer size, and whether
    vertex DMA will be enabled.

    You essentially fill one of these in, and pass it to pvr_init().
*/
typedef struct pvr_init_params {
    /** \brief  Bin sizes.

        The bins go in the following order: opaque polygons, opaque modifiers,
        translucent polygons, translucent modifiers, punch-thrus
    */
    size_t     opb_sizes[PVR_LIST_COUNT];

    /** \brief  Vertex buffer size (should be a nice round number) */
    size_t     vertex_buf_size;

    /** \brief  Enable vertex DMA?

        Set to non-zero if we want to enable vertex DMA mode. Note that if this
        is set, then _all_ enabled lists need to have a vertex buffer assigned,
        even if you never use that list for anything.
    */
    bool     dma_enabled;

    /** \brief  Enable horizontal scaling?

        Set to non-zero if horizontal scaling is to be enabled. By enabling this
        setting and stretching your image to double the native screen width, you
        can get horizontal full-screen anti-aliasing. */
    bool     fsaa_enabled;

    /** \brief  Disable translucent polygon autosort?

        Set to non-zero to disable translucent polygon autosorting. By enabling
        this setting, the PVR acts more like a traditional Z-buffered system
        when rendering translucent polygons, meaning you must pre-sort them
        yourself if you want them to appear in the right order. */
    bool     autosort_disabled;


    /** \brief  OPB Overflow Count.

        Preallocates this many extra OPBs (sets of tile bins), allowing the PVR
        to use the extra space when there's too much geometry in the first OPB.
    
        Increasing this value can eliminate artifacts where pieces of geometry
        flicker in and out of existence along the tile boundaries. */

    size_t     opb_overflow_count;

} pvr_init_params_t;

/** \brief   Initialize the PVR chip to ready status.
    \ingroup pvr_init

    This function enables the specified lists and uses the specified parameters.
    Note that bins and vertex buffers come from the texture memory pool, so only
    allocate what you actually need. Expects that a 2D mode was initialized
    already using the vid_* API.

    \param  params          The set of parameters to initialize with
    \retval 0               On success
    \retval -1              If the PVR has already been initialized or the video
                            mode active is not suitable for 3D
*/
int pvr_init(const pvr_init_params_t *params);

/** \brief   Simple PVR initialization.
    \ingroup pvr_init

    This simpler function initializes the PVR using 16/16 for the opaque
    and translucent lists' bin sizes, and 0's for everything else. It sets 512KB
    of vertex buffer. This is equivalent to the old ta_init_defaults() for now.

    \retval 0               On success
    \retval -1              If the PVR has already been initialized or the video
                            mode active is not suitable for 3D
*/
int pvr_init_defaults(void);

/** \brief   Shut down the PVR chip from ready status.
    \ingroup pvr_init

    This essentially leaves the video system in 2D mode as it was before the
    init.

    \retval 0               On success
    \retval -1              If the PVR has not been initialized
*/
int pvr_shutdown(void);


/* Misc parameters ***************************************************/

/** \defgroup pvr_global Global State
    \brief               PowerVR functionality which is managed globally
    \ingroup             pvr

    These are miscellaneous parameters you can set which affect the
    rendering process.
*/

/** \brief   Set the background plane color.
    \ingroup pvr_global

    This function sets the color of the area of the screen not covered by any
    other polygons.

    \param  r               Red component of the color to set
    \param  g               Green component of the color to set
    \param  b               Blue component of the color to set
*/
void pvr_set_bg_color(float r, float g, float b);

/** \brief   Set cheap shadow parameters.
    \ingroup pvr_global

    This function sets up the PVR cheap shadow parameters for use. You can only
    specify one scale value per frame, so the effect that you can get from this
    is somewhat limited, but if you want simple shadows, this is the easiest way
    to do it.

    Polygons affected by a shadow modifier volume will effectively multiply
    their final color by the scale value set here when shadows are enabled and
    the polygon is inside the modifier (or outside for exclusion volumes).

    \param  enable          Set to true to enable cheap shadow mode.
    \param  scale_value     Floating point value (between 0 and 1) representing
                            how colors of polygons affected by and inside the
                            volume will be modified by the shadow volume.
*/
void pvr_set_shadow_scale(bool enable, float scale_value);

/** \brief   Set Z clipping depth.
    \ingroup pvr_global

    This function sets the Z clipping depth. The default value for this is
    0.0001.

    \param  zc              The new value to set the z clip parameter to.
*/
void pvr_set_zclip(float zc);

/** \brief   Retrieve the current VBlank count.
    \ingroup pvr_stats

    This function retrieves the number of VBlank interrupts that have occurred
    since the PVR was initialized.

    \return                 The number of VBlanks since init
*/
size_t pvr_get_vbl_count(void);

/** \defgroup pvr_stats         Profiling
    \brief                      Rendering stats and metrics for profiling
    \ingroup                    pvr
*/

/** \brief   PVR statistics structure.
    \ingroup pvr_stats

    This structure is used to hold various statistics about the operation of the
    PVR since initialization.
*/
typedef struct pvr_stats {
    uint64_t frame_last_time;     /**< \brief Ready-to-Ready length for the last frame in nanoseconds */
    uint64_t reg_last_time;       /**< \brief Registration time for the last frame in nanoseconds */
    uint64_t rnd_last_time;       /**< \brief Rendering time for the last frame in nanoseconds */
    uint64_t buf_last_time;       /**< \brief DMA buffer file time for the last frame in nanoseconds */
    size_t   frame_count;         /**< \brief Total number of rendered/viewed frames */
    size_t   vbl_count;           /**< \brief VBlank count */
    size_t   vtx_buffer_used;     /**< \brief Number of bytes used in the vertex buffer for the last frame */
    size_t   vtx_buffer_used_max; /**< \brief Number of bytes used in the vertex buffer for the largest frame */
    float    frame_rate;          /**< \brief Current frame rate (per second) */
    uint32_t enabled_list_mask;   /**< \brief Which lists are enabled? */
} pvr_stats_t;

/** \brief   Get the current statistics from the PVR.
    \ingroup pvr_stats

    This function fills in the pvr_stats_t structure passed in with the current
    statistics of the system.

    \param  stat            The statistics structure to fill in. Must not be
                            NULL
    \retval 0               On success
    \retval -1              If the PVR is not initialized
*/
int pvr_get_stats(pvr_stats_t *stat);


/* Scene rendering ***************************************************/
/** \defgroup   pvr_scene_mgmt  Scene Submission
    \brief                      PowerVR API for submitting scene geometry
    \ingroup                    pvr

    This API is used to submit triangle strips to the PVR via the TA
    interface in the chip.

    An important side note about the PVR is that all primitive types
    must be submitted grouped together. If you have 10 polygons for each
    list type, then the PVR must receive them via the TA by list type,
    with a list delimiter in between.

    So there are two modes you can use here. The first mode allows you to
    submit data directly to the TA. Your data will be forwarded to the
    chip for processing as it is fed to the PVR module. If your data
    is easily sorted into the primitive types, then this is the fastest
    mode for submitting data.

    The second mode allows you to submit data via main-RAM vertex buffers,
    which will be queued until the proper primitive type is active. In this
    case, each piece of data is copied into the vertex buffer while the
    wrong list is activated, and when the proper list becomes activated,
    the data is all sent at once. Ideally this would be via DMA, right
    now it is by store queues. This has the advantage of allowing you to
    send data in any order and have the PVR functions resolve how it should
    get sent to the hardware, but it is slower.

    The nice thing is that any combination of these modes can be used. You
    can assign a vertex buffer for any list, and it will be used to hold the
    incoming vertex data until the proper list has come up. Or if the proper
    list is already up, the data will be submitted directly. So if most of
    your polygons are opaque, and you only have a couple of translucents,
    you can set a small buffer to gather translucent data and then it will
    get sent when you do a pvr_end_scene().

    Thanks to Mikael Kalms for the idea for this API.

    \note
    Another somewhat subtle point that bears mentioning is that in the normal
    case (interrupts enabled) an interrupt handler will automatically take
    care of starting a frame rendering (after scene_finish()) and also
    flipping pages when appropriate. 
*/

/** \defgroup  pvr_vertex_dma   Vertex DMA
    \brief                      Use the DMA to transfer inactive lists to the PVR
    \ingroup                    pvr_scene_mgmt
*/

/** \brief   Is vertex DMA enabled?
    \ingroup pvr_vertex_dma
    
    \return  true if vertex DMA was enabled at init time
*/
bool pvr_vertex_dma_enabled(void);

/** \brief   Setup a vertex buffer for one of the list types.
    \ingroup pvr_list_mgmt

    If the specified list type already has a vertex buffer, it will be replaced
    by the new one. 

    \note
    Each buffer should actually be twice as long as what you will need to hold
    two frames worth of data).

    \warning
    You should generally not try to do this at any time besides before a frame
    is begun, or Bad Things May Happen.

    \param  list            The primitive list to set the buffer for.
    \param  buffer          The location of the buffer in main RAM. This must be
                            aligned to a 32-byte boundary.
    \param  len             The length of the buffer. This must be a multiple of
                            64, and must be at least 128 (even if you're not
                            using the list).
    
    \return                 The old buffer location (if any)
*/
void *pvr_set_vertbuf(pvr_list_t list, void *buffer, size_t len);

/** \brief   Retrieve a pointer to the current output location in the DMA buffer
             for the requested list.
    \ingroup pvr_vertex_dma

    Vertex DMA must globally be enabled for this to work. Data may be added to
    this buffer by the user program directly; however, make sure to call
    pvr_vertbuf_written() to notify the system of any such changes.

    \param  list            The primitive list to get the buffer for.
    
    \return                 The tail of that list's buffer.
*/
void *pvr_vertbuf_tail(pvr_list_t list);

/** \brief   Notify the PVR system that data have been written into the output
             buffer for the given list.
    \ingroup pvr_vertex_dma

    This should always be done after writing data directly to these buffers or
    it will get overwritten by other data.

    \param  list            The primitive list that was modified.
    \param  amt             Number of bytes written. Must be a multiple of 32.
*/
void pvr_vertbuf_written(pvr_list_t list, size_t amt);

/** \brief   Set the translucent polygon sort mode for the next frame.
    \ingroup pvr_scene_mgmt

    This function sets the translucent polygon sort mode for the next frame of
    output, potentially switching between autosort and presort mode.

    For most programs, you'll probably want to set this at initialization time
    (with the autosort_disabled field in the pvr_init_params_t structure) and
    not mess with it per-frame. It is recommended that if you do use this
    function to change the mode that you should set it each frame to ensure that
    the mode is set properly.

    \param  presort         Set to 1 to set the presort mode for translucent
                            polygons, set to 0 to use autosort mode.
*/
void pvr_set_presort_mode(bool presort);

/** \brief   Begin collecting data for a frame of 3D output to the off-screen
             frame buffer.
    \ingroup pvr_scene_mgmt

    You must call this function (or pvr_scene_begin_txr()) for ever frame of
    output.
*/
void pvr_scene_begin(void);

/** \brief   Begin collecting data for a frame of 3D output to the specified
             texture.
    \ingroup pvr_scene_mgmt

    This function currently only supports outputting at the same size as the
    actual screen. Thus, make sure rx and ry are at least large enough for that.
    For a 640x480 output, rx will generally be 1024 on input and ry 512, as
    these are the smallest values that are powers of two and will hold the full
    screen sized output.

    \param  txr             The texture to render to.
    \param  rx              Width of the texture buffer (in pixels).
    \param  ry              Height of the texture buffer (in pixels).
*/
void pvr_scene_begin_txr(pvr_ptr_t txr, size_t *rx, size_t *ry);


/** \defgroup pvr_list_mgmt Polygon Lists
    \brief                  PVR API for managing list submission
    \ingroup                pvr_scene_mgmt
*/

/** \brief   Begin collecting data for the given list type.
    \ingroup pvr_list_mgmt

    Lists do not have to be submitted in any particular order, but all types of
    a list must be submitted at once (unless vertex DMA mode is enabled).

    Note that there is no need to call this function in DMA mode unless you want
    to make use of pvr_prim() for compatibility. This function will
    automatically call pvr_list_finish() if a list is already opened before
    opening the new list.

    \param  list            The list to open.
    \retval 0               On success.
    \retval -1              If the specified list has already been closed.
*/
int pvr_list_begin(pvr_list_t list);            

/** \brief   End collecting data for the current list type.
    \ingroup pvr_list_mgmt

    Lists can never be opened again within a single frame once they have been
    closed. Thus submitting a primitive that belongs in a closed list is
    considered an error. Closing a list that is already closed is also an error.

    Note that if you open a list but do not submit any primitives, a blank one
    will be submitted to satisfy the hardware. If vertex DMA mode is enabled,
    then this simply sets the current list pointer to no list, and none of the
    above restrictions apply.

    \retval 0               On success.
    \retval -1              On error.
*/
int pvr_list_finish(void);

/** \brief   Submit a primitive of the current list type.
    \ingroup pvr_list_mgmt

    Note that any values submitted in this fashion will go directly to the
    hardware without any sort of buffering, and submitting a primitive of the
    wrong type will quite likely ruin your scene. Note that this also will not
    work if you haven't begun any list types (i.e., all data is queued). If DMA
    is enabled, the primitive will be appended to the end of the currently
    selected list's buffer.

    \param  data            The primitive to submit.
    \param  size            The length of the primitive, in bytes. Must be a
                            multiple of 32.
    
    \retval 0               On success.
    \retval -1              On error.
*/
int pvr_prim(void *data, size_t size);

/** \defgroup pvr_direct  Direct Rendering
    \brief                API for using direct rendering with the PVR
    \ingroup              pvr_scene_mgmt

    @{
*/

/** \brief   Direct Rendering state variable type. */
typedef uintptr_t pvr_dr_state_t;

/** \brief   Initialize a state variable for Direct Rendering.

    Store Queues are used.

    \param  vtx_buf_ptr     A variable of type pvr_dr_state_t to init.
*/
void pvr_dr_init(pvr_dr_state_t *vtx_buf_ptr);

/** \brief   Obtain the target address for Direct Rendering.

    \param  vtx_buf_ptr     State variable for Direct Rendering. Should be of
                            type pvr_dr_state_t, and must have been initialized
                            previously in the scene with pvr_dr_init().
    
    \return                 A write-only destination address where a primitive
                            should be written to get ready to submit it to the
                            TA in DR mode.
*/
#define pvr_dr_target(vtx_buf_ptr) \
    ({ (vtx_buf_ptr) ^= 32; \
        (pvr_vertex_t *)(MEM_AREA_SQ_BASE | (vtx_buf_ptr)); \
    })

/** \brief   Commit a primitive written into the Direct Rendering target address.

    \param  addr            The address returned by pvr_dr_target(), after you
                            have written the primitive to it.
*/
#define pvr_dr_commit(addr) sq_flush(addr)

/** \brief  Finish work with Direct Rendering.

    Called atomatically in pvr_scene_finish().
    Use it manually if you want to release Store Queues earlier.

*/
void pvr_dr_finish(void);

/** @} */

/** \brief   Submit a primitive of the given list type.
    \ingroup pvr_list_mgmt

    Data will be queued in a vertex buffer, thus one must be available for the
    list specified (will be asserted by the code).

    \param  list            The list to submit to.
    \param  data            The primitive to submit.
    \param  size            The size of the primitive in bytes. This must be a
                            multiple of 32.
    
    \retval 0               On success.
    \retval -1              On error.
*/
int pvr_list_prim(pvr_list_t list, void *data, size_t size);

/** \brief   Flush the buffered data of the given list type to the TA.
    \ingroup pvr_list_mgmt

    This function is currently not implemented, and calling it will result in an
    assertion failure. It is intended to be used later in a "hybrid" mode where
    both direct and DMA TA submission is possible.

    \param  list            The list to flush.
    
    \retval -1              On error (it is not possible to succeed).
*/
int pvr_list_flush(pvr_list_t list);

/** \brief   Call this after you have finished submitting all data for a frame.
    \ingroup pvr_scene_mgmt

    Once this has been called, you can not submit any more data until one of the
    pvr_scene_begin() or pvr_scene_begin_txr() functions is called again.

    \retval 0               On success.
    \retval -1              On error (no scene started).
*/
int pvr_scene_finish(void);

/** \brief   Block the caller until the PVR system is ready for another frame to
             be submitted.
    \ingroup pvr_scene_mgmt

    The PVR system allocates enough space for two frames: one in data collection
    mode, and another in rendering mode. If a frame is currently rendering, and
    another frame has already been closed, then the caller cannot do anything
    else until the rendering frame completes. Note also that the new frame
    cannot be activated except during a vertical blanking period, so this
    essentially waits until a rendered frame is complete and a vertical blank
    happens.

    \retval 0               On success. A new scene can be started now.
    \retval -1              On error. Something is probably very wrong...
*/
int pvr_wait_ready(void);

/** \brief   Check if the PVR system is ready for another frame to be submitted.
    \ingroup pvr_scene_mgmt

    \retval 0               If the PVR is ready for a new scene. You must call
                            pvr_wait_ready() afterwards, before starting a new
                            scene.
    \retval -1              If the PVR is not ready for a new scene yet.

    \sa pvr_is_ready()
*/
int pvr_check_ready(void);

/** \brief   Check whether the PVR is ready for another frame to be submitted.
    \ingroup pvr_scene_mgmt

    \note
    This function is equivalent to pvr_check_ready() with a sane return value.

    \retval false   The PVR is not yet raedy for another scene. You must call
                    pvr_wait_ready() afterwards, before starting a new scene.
    \retval true    The PVR is ready to begin a new scene.
 
*/
static inline bool pvr_is_ready(void) {
    return !pvr_check_ready();
}

/* Primitive handling ************************************************/

/** \defgroup pvr_primitives_compilation Compilation
    \brief                               API for compiling primitive contexts
                                         into headers
    \ingroup pvr_ctx
*/

/** \brief   Compile a polygon context into a polygon header.
    \ingroup pvr_primitives_compilation

    This function compiles a pvr_poly_cxt_t into the form needed by the hardware
    for rendering. This is for use with normal polygon headers.

    \param  dst             Where to store the compiled header.
    \param  src             The context to compile.
*/
void pvr_poly_compile(pvr_poly_hdr_t *dst, const pvr_poly_cxt_t *src);

/** \defgroup pvr_ctx_init     Initialization
    \brief                     Functions for initializing PVR polygon contexts
    \ingroup                   pvr_ctx
*/

/** \brief   Fill in a polygon context for non-textured polygons.
    \ingroup pvr_ctx_init

    This function fills in a pvr_poly_cxt_t with default parameters appropriate
    for rendering a non-textured polygon in the given list.

    \param  dst             Where to store the polygon context.
    \param  list            The primitive list to be used.
*/
void pvr_poly_cxt_col(pvr_poly_cxt_t *dst, pvr_list_t list);

/** \brief   Fill in a polygon context for a textured polygon.
    \ingroup pvr_ctx_init

    This function fills in a pvr_poly_cxt_t with default parameters appropriate
    for rendering a textured polygon in the given list.

    \param  dst             Where to store the polygon context.
    \param  list            The primitive list to be used.
    \param  textureformat   The format of the texture used.
    \param  tw              The width of the texture, in pixels.
    \param  th              The height of the texture, in pixels.
    \param  textureaddr     A pointer to the texture.
    \param  filtering       The type of filtering to use.

    \see    pvr_txr_fmts
    \see    pvr_filter_modes
*/
void pvr_poly_cxt_txr(pvr_poly_cxt_t *dst, pvr_list_t list,
                      pvr_txr_fmt_t textureformat, size_t tw, size_t th, pvr_ptr_t textureaddr,
                      pvr_filter_t filtering);

/** \brief   Compile a sprite context into a sprite header.
    \ingroup pvr_primitives_compilation

    This function compiles a pvr_sprite_cxt_t into the form needed by the
    hardware for rendering. This is for use with sprite headers.

    \param  dst             Where to store the compiled header.
    \param  src             The context to compile.
*/
void pvr_sprite_compile(pvr_sprite_hdr_t *dst,
                        const pvr_sprite_cxt_t *src);

/** \brief   Fill in a sprite context for non-textured sprites.
    \ingroup pvr_ctx_init

    This function fills in a pvr_sprite_cxt_t with default parameters
    appropriate for rendering a non-textured sprite in the given list.

    \param  dst             Where to store the sprite context.
    \param  list            The primitive list to be used.
*/
void pvr_sprite_cxt_col(pvr_sprite_cxt_t *dst, pvr_list_t list);

/** \brief   Fill in a sprite context for a textured sprite.
    \ingroup pvr_ctx_init

    This function fills in a pvr_sprite_cxt_t with default parameters
    appropriate for rendering a textured sprite in the given list.

    \param  dst             Where to store the sprite context.
    \param  list            The primitive list to be used.
    \param  textureformat   The format of the texture used.
    \param  tw              The width of the texture, in pixels.
    \param  th              The height of the texture, in pixels.
    \param  textureaddr     A pointer to the texture.
    \param  filtering       The type of filtering to use.

    \see    pvr_txr_fmts
    \see    pvr_filter_modes
*/
void pvr_sprite_cxt_txr(pvr_sprite_cxt_t *dst, pvr_list_t list,
                        pvr_txr_fmt_t textureformat, size_t tw, size_t th, pvr_ptr_t textureaddr,
                        pvr_filter_t filtering);

/** \brief   Create a modifier volume header.
    \ingroup pvr_primitives_compilation

    This function fills in a modifier volume header with the parameters
    specified. Note that unlike for polygons and sprites, there is no context
    step for modifiers.

    \param  dst             Where to store the modifier header.
    \param  list            The primitive list to be used.
    \param  mode            The mode for this modifier.
    \param  cull            The culling mode to use.

    \see    pvr_mod_modes
    \see    pvr_cull_modes
*/
void pvr_mod_compile(pvr_mod_hdr_t *dst, pvr_list_t list, pvr_mod_t mode,
                     pvr_cull_t cull);

/** \brief   Compile a polygon context into a polygon header that is affected by
             modifier volumes.
    \ingroup pvr_primitives_compilation

    This function works pretty similarly to pvr_poly_compile(), but compiles
    into the header type that is affected by a modifier volume. The context
    should have been created with either pvr_poly_cxt_col_mod() or
    pvr_poly_cxt_txr_mod().

    \param  dst             Where to store the compiled header.
    \param  src             The context to compile.
*/
void pvr_poly_mod_compile(pvr_poly_mod_hdr_t *dst, const pvr_poly_cxt_t *src);

/** \brief   Fill in a polygon context for non-textured polygons affected by a
             modifier volume.
    \ingroup pvr_ctx_init

    This function fills in a pvr_poly_cxt_t with default parameters appropriate
    for rendering a non-textured polygon in the given list that will be affected
    by modifier volumes.

    \param  dst             Where to store the polygon context.
    \param  list            The primitive list to be used.
*/
void pvr_poly_cxt_col_mod(pvr_poly_cxt_t *dst, pvr_list_t list);

/** \brief   Fill in a polygon context for a textured polygon affected by
             modifier volumes.
    \ingroup pvr_ctx_init

    This function fills in a pvr_poly_cxt_t with default parameters appropriate
    for rendering a textured polygon in the given list and being affected by
    modifier volumes.

    \param  dst             Where to store the polygon context.
    \param  list            The primitive list to be used.
    \param  textureformat   The format of the texture used (outside).
    \param  tw              The width of the texture, in pixels (outside).
    \param  th              The height of the texture, in pixels (outside).
    \param  textureaddr     A pointer to the texture (outside).
    \param  filtering       The type of filtering to use (outside).
    \param  textureformat2  The format of the texture used (inside).
    \param  tw2             The width of the texture, in pixels (inside).
    \param  th2             The height of the texture, in pixels (inside).
    \param  textureaddr2    A pointer to the texture (inside).
    \param  filtering2      The type of filtering to use (inside).

    \see    pvr_txr_fmts
    \see    pvr_filter_modes
*/
void pvr_poly_cxt_txr_mod(pvr_poly_cxt_t *dst, pvr_list_t list,
                          pvr_txr_fmt_t textureformat, size_t tw, size_t th,
                          pvr_ptr_t textureaddr, pvr_filter_t filtering,
                          pvr_txr_fmt_t textureformat2, size_t tw2, size_t th2,
                          pvr_ptr_t textureaddr2, pvr_filter_t filtering2);

/* PVR DMA ***********************************************************/
/** \defgroup pvr_dma   DMA
    \brief              PowerVR DMA driver
    \ingroup            pvr
*/

/** \brief   PVR DMA interrupt callback type.
    \ingroup pvr_dma

    Functions that act as callbacks when DMA completes should be of this type.
    These functions will be called inside an interrupt context, so don't try to
    use anything that might stall.

    \param  data            User data passed in to the pvr_dma_transfer()
                            function.
*/
typedef void (*pvr_dma_callback_t)(void *data);

/** \brief   Perform a DMA transfer to the PVR RAM over 64-bit TA bus.
    \ingroup pvr_dma

    This function copies a block of data to the PVR or its memory via DMA. There
    are all kinds of constraints that must be fulfilled to actually do this, so
    make sure to read all the fine print with the parameter list.

    If a callback is specified, it will be called in an interrupt context, so
    keep that in mind in writing the callback.

    \param  src             Where to copy from. Must be 32-byte aligned.
    \param  dest            Where to copy to. Must be 32-byte aligned.
    \param  count           The number of bytes to copy. Must be a multiple of
                            32.
    \param  type            The type of DMA transfer to do (see list of modes).
    \param  block           Non-zero if you want the function to block until the
                            DMA completes.
    \param  callback        A function to call upon completion of the DMA.
    \param  cbdata          Data to pass to the callback function.
    \retval 0               On success.
    \retval -1              On failure. Sets errno as appropriate.

    \par    Error Conditions:
    \em     EINPROGRESS - DMA already in progress \n
    \em     EFAULT - dest is not 32-byte aligned \n
    \em     EIO - I/O error

    \see    pvr_dma_modes
*/
int pvr_dma_transfer(void *src, uintptr_t dest, size_t count, int type,
                     int block, pvr_dma_callback_t callback, void *cbdata);

/** \defgroup pvr_dma_modes         Transfer Modes
    \brief                          Transfer modes with TA/PVR DMA and Store Queues
    \ingroup  pvr_dma

    @{
*/
#define PVR_DMA_VRAM64    0   /**< \brief Transfer to VRAM using TA bus */
#define PVR_DMA_VRAM32    1   /**< \brief Transfer to VRAM using TA bus */
#define PVR_DMA_TA        2   /**< \brief Transfer to the tile accelerator */
#define PVR_DMA_YUV       3   /**< \brief Transfer to the YUV converter (TA) */
#define PVR_DMA_VRAM32_SB 4   /**< \brief Transfer to/from VRAM using PVR i/f */
#define PVR_DMA_VRAM64_SB 5   /**< \brief Transfer to/from VRAM using PVR i/f */
/** @} */

/** \brief   Load a texture using TA DMA.
    \ingroup pvr_dma

    This is essentially a convenience wrapper for pvr_dma_transfer(), so all
    notes that apply to it also apply here.

    \param  src             Where to copy from. Must be 32-byte aligned.
    \param  dest            Where to copy to. Must be 32-byte aligned.
    \param  count           The number of bytes to copy. Must be a multiple of
                            32.
    \param  block           Non-zero if you want the function to block until the
                            DMA completes.
    \param  callback        A function to call upon completion of the DMA.
    \param  cbdata          Data to pass to the callback function.
    \retval 0               On success.
    \retval -1              On failure. Sets errno as appropriate.

    \par    Error Conditions:
    \em     EINPROGRESS - DMA already in progress \n
    \em     EFAULT - dest is not 32-byte aligned \n
    \em     EIO - I/O error
*/
int pvr_txr_load_dma(void *src, pvr_ptr_t dest, size_t count, int block,
                     pvr_dma_callback_t callback, void *cbdata);

/** \brief   Load vertex data to the TA using TA DMA.
    \ingroup pvr_dma

    This is essentially a convenience wrapper for pvr_dma_transfer(), so all
    notes that apply to it also apply here.

    \param  src             Where to copy from. Must be 32-byte aligned.
    \param  count           The number of bytes to copy. Must be a multiple of
                            32.
    \param  block           Non-zero if you want the function to block until the
                            DMA completes.
    \param  callback        A function to call upon completion of the DMA.
    \param  cbdata          Data to pass to the callback function.
    \retval 0               On success.
    \retval -1              On failure. Sets errno as appropriate.

    \par    Error Conditions:
    \em     EINPROGRESS - DMA already in progress \n
    \em     EFAULT - dest is not 32-byte aligned \n
    \em     EIO - I/O error
 */
int pvr_dma_load_ta(void *src, size_t count, int block,
                    pvr_dma_callback_t callback, void *cbdata);

/** \brief   Load yuv data to the YUV converter using TA DMA.
    \ingroup pvr_dma

    This is essentially a convenience wrapper for pvr_dma_transfer(), so all
    notes that apply to it also apply here.

    \param  src             Where to copy from. Must be 32-byte aligned.
    \param  count           The number of bytes to copy. Must be a multiple of
                            32.
    \param  block           Non-zero if you want the function to block until the
                            DMA completes.
    \param  callback        A function to call upon completion of the DMA.
    \param  cbdata          Data to pass to the callback function.
    \retval 0               On success.
    \retval -1              On failure. Sets errno as appropriate.

    \par    Error Conditions:
    \em     EINPROGRESS - DMA already in progress \n
    \em     EFAULT - dest is not 32-byte aligned \n
    \em     EIO - I/O error
*/
int pvr_dma_yuv_conv(void *src, size_t count, int block,
                     pvr_dma_callback_t callback, void *cbdata);

/** \brief   Is PVR DMA is inactive?
    \ingroup pvr_dma
    \return                 Non-zero if there is no PVR DMA active, thus a DMA
                            can begin or 0 if there is an active DMA.
*/
int pvr_dma_ready(void);

/** \brief   Initialize TA/PVR DMA. 
    \ingroup pvr_dma
 */
void pvr_dma_init(void);

/** \brief   Shut down TA/PVR DMA. 
    \ingroup pvr_dma
 */
void pvr_dma_shutdown(void);

/** \brief   Copy a block of memory to VRAM
    \ingroup store_queues

    This function is similar to sq_cpy(), but it has been
    optimized for writing to a destination residing within VRAM.

    \warning
    This function cannot be used at the same time as a PVR DMA transfer.

    The dest pointer must be at least 32-byte aligned and reside 
    in video memory, the src pointer must be at least 8-byte aligned, 
    and n must be a multiple of 32.

    \param  dest            The address to copy to (32-byte aligned).
    \param  src             The address to copy from (32-bit (8-byte) aligned).
    \param  n               The number of bytes to copy (multiple of 32).
    \param  type            The type of SQ/DMA transfer to do (see list of modes).
    \return                 The original value of dest.

    \sa pvr_sq_set32()
*/
void *pvr_sq_load(void *dest, const void *src, size_t n, int type);

/** \brief   Set a block of PVR memory to a 16-bit value.
    \ingroup store_queues

    This function is similar to sq_set16(), but it has been
    optimized for writing to a destination residing within VRAM.

    \warning
    This function cannot be used at the same time as a PVR DMA transfer.
    
    The dest pointer must be at least 32-byte aligned and reside in video 
    memory, n must be a multiple of 32 and only the low 16-bits are used 
    from c.

    \param  dest            The address to begin setting at (32-byte aligned).
    \param  c               The value to set (in the low 16-bits).
    \param  n               The number of bytes to set (multiple of 32).
    \param  type            The type of SQ/DMA transfer to do (see list of modes).
    \return                 The original value of dest.

    \sa pvr_sq_set32()
*/
void *pvr_sq_set16(void *dest, uint32_t c, size_t n, int type);

/** \brief   Set a block of PVR memory to a 32-bit value.
    \ingroup store_queues

    This function is similar to sq_set32(), but it has been
    optimized for writing to a destination residing within VRAM.

    \warning
    This function cannot be used at the same time as a PVR DMA transfer.

    The dest pointer must be at least 32-byte aligned and reside in video 
    memory, n must be a multiple of 32.

    \param  dest            The address to begin setting at (32-byte aligned).
    \param  c               The value to set.
    \param  n               The number of bytes to set (multiple of 32).
    \param  type            The type of SQ/DMA transfer to do (see list of modes).
    \return                 The original value of dest.

    \sa pvr_sq_set16
*/
void *pvr_sq_set32(void *dest, uint32_t c, size_t n, int type);

/*********************************************************************/


__END_DECLS

#endif
