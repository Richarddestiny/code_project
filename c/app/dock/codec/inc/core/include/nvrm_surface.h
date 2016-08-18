/*
 * Copyright (c) 2009-2014 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef INCLUDED_nvrm_surface_H
#define INCLUDED_nvrm_surface_H

/**
 * @file
 * <b> NVIDIA Driver Development Kit: Resource Manager Surface Management Interface</b>
 *
 * @b Description: This file declares the interface for RM Surface Management
 *    Services.
 *
 */


#if defined(__cplusplus)
extern "C"
{
#endif

#include "nvrm_memmgr.h"
#include "nvrm_init.h"
#include "nvrm_drf.h"

/**
 * @defgroup nvrm_surface RM Surface Management Services
 *
 * @ingroup nvddk_rm
 *
 * We distinguish between a "memory allocation" and a "surface". A memory
 * allocation, which comes from API like ::NvOsAlloc or
 * \c NvRmMemHandleAllocAttr, is an unstructured array of bytes that have no
 * particular semantic meaning -- they are just bytes in which data can be
 * stored. A surface, on the other hand, is a higher-level construct where the
 * bytes comprise an array of pixels with some particular color format and
 * layout in memory. Surfaces are always two-dimensional.
 *
 * The mapping between surfaces and memory allocations is many-to-one. A given
 * memory allocation can hold many surfaces. Usually they will be
 * nonoverlapping, but not always. For example, we might choose to interpret
 * the same bytes as either an RGB surface or a YUV surface for different
 * purposes. Or, we might have a mipmap chain where each mipmap level is a
 * separate surface with a different width and height, pointing to a different
 * part of a larger memory allocation.
 *
 * For this reason, it is useful to think of a surface as a view of memory,
 * rather than as a piece of memory itself. The underlying memory's lifetime
 * must exceed the surface's lifetime.
 *
 * The surface API provides a uniform way of describing a surface for other
 * APIs, regardless of its properties such as width, height, color format,
 * layout, and so on.
 *
 * The way the pixels are laid out in memory is defined by the "Layout", "Kind",
 * and "Block Height" parameters of a surface.
 *
 * The simplest layout is called "Pitch" (or "Pitchlinear"). In pitch format, a
 * pixel at location (x,y) lives at address y*pitch + x*bpp from the base of the
 * surface. Pitch layout is optimal for hardware engines that are optimized for
 * linear access patterns (typically at least CPU).
 *
 * The most efficient layout for many hardware engines (such as GPU and VIC) is
 * "blocklinear". The blocklinear layout is parametrizable with the block height
 * in GOBs (Groups Of Bytes), and "kind". The kind specifies how to interpret
 * the bytes within a GOB (for pitch surfaces, kind is always NvRmMemKind_Pitch).
 * For more information about GOBs, kinds and the blocklinear format, refer to
 * //hw/ar/doc/t124/gfd/pixel_memory_formats/T40 Pixel Memory Format GFD.doc.
 *
 * Because the pixel layout of a surface depends on many factors, it is a good
 * idea to avoid manipulating the raw pixel data using the CPU (except for
 * pitch surfaces). The surface API provides NvRmSurfaceRead,  NvRmSurfaceWrite
 * and NvRmSurfaceComputeOffset functions for doing CPU operations. These
 * functions work for all layout configurations, but they are not intended for
 * use by performance critical code.
 *
 * The surface API does not natively understand planar YUV surfaces. A planar
 * YUV surface is really three separate surfaces that you can operate on
 * independently, and that (usually) just happen to be adjacent in memory.
 * However the API does provide helpers for setting up arrays of surfaces,
 * typically used when setting up YUV surfaces.
 *
 * We use opaque handles to represent memory allocations. We do not, however,
 * use opaque handles to represent surfaces. The surface structure is fully
 * exposed to its clients. One important benefit is that surfaces can live on
 * the stack, rather than on the heap -- creating a "temporary" surface that
 * refers to an existing chunk of memory is extremely cheap and cannot fail.
 * Another benefit is that surfaces can be embedded directly in other heap data
 * structures, rather than having to store a pointer to the surface. Finally,
 * it allows code that operates on a surface to extract its properties (its
 * underlying storage, its width, its height, its color format, etc.) without
 * the overhead of a function call.
 *
 * In general, this design decision increases performance and simplifies the
 * code, at the cost of (1) having to recompile any time that the surface
 * structure definition changes and (2) not being able to protect users as
 * much from misusing the API. We believe (1) is not a major concern, but (2)
 * is slightly more worrisome, so take caution to use the API as documented.
 *
 * We allow a surface to be backed by two types of storage: either by a plain
 * pointer (which would typically come from something like ::NvOsAlloc), or
 * by an ::NvRmMemHandle. We do not attempt here to manage the lifetime of the
 * storage. That is entirely the responsibility of the client.
 *
 *
 * The two most important functions of this API are NvRmSurfaceSetup and
 * NvRmMultiplanarSurfaceSetup. They should be used for initializing NvRmSurfaces
 * for single-plane and YUV surfaces, respectively. A typical allocation code
 * looks like:
 *
 * <pre>
 *    NvRmSurface s;
 *    NvRmSurfaceSetup(&s, 400, 400, NvColorFormat_A8R8G8B8, NULL);
 * </pre>
 *
 * This will choose the default layout (depends on the chip), and initialize the
 * fields of NvRmSurface. The setup code (and the fields of NvRmSurface) tends
 * to change slightly for every new chip, so using NvRmSurfaceSetup is highly
 * encouraged.
 *
 * If you want finer grained control over how exactly the surface should be set
 * up, you can use the Attributes array for passing additional requirements (or
 * hints) to NvRmSurfaceSetup. For example, to set up a pitchlinear surface, do:
 *
 * <pre>
 *    NvRmSurface s;
 *    NvU32 attrs[] = { NvRmSurfaceAttribute_Layout, NvRmSurfaceLayout_Pitch,
 *                      NvRmSurfaceAttribute_None };
 *    NvRmSurfaceSetup(&s, 400, 400, NvColorFormat_A8R8G8B8, attrs);
 * </pre>
 *
 * Or to request a compressible surface, do:
 *
 * <pre>
 *    NvRmSurface s;
 *    NvU32 attrs[] = { NvRmSurfaceAttribute_Compression, NV_TRUE,
 *                      NvRmSurfaceAttribute_None };
 *    NvRmSurfaceSetup(&s, 400, 400, NvColorFormat_A8R8G8B8, attrs);
 * </pre>
 *
 * As stated above, the surface API is not responsible for memory allocation.
 * However, typically the next step after setting up a surface is to allocate
 * backing memory for it. Typical set up and allocation code looks like this:
 *
 * <pre>
 *    // Set up.
 *    NvRmSurface s;
 *    NvRmSurfaceSetup(&s, 400, 400, NvColorFormat_A8R8G8B8, NULL);
 *
 *    // Compute size and alignment.
 *    NvU32 size = NvRmSurfaceComputeSize(&s);
 *    NvU32 alignment = NvRmSurfaceComputeAlignment(NULL, &s);
 *
 *    // Allocate memory.
 *    NVRM_DEFINE_MEM_HANDLE_ATTR(HandleAttr);
 *    NVRM_MEM_HANDLE_SET_ATTR(HandleAttr,
 *                             alignment,
 *                             NvOsMemAttribute_WriteBack,
 *                             size,
 *                             NVRM_MEM_TAG_GRALLOC_MISC);
 *    NVRM_MEM_HANDLE_SET_KIND_ATTR(HandleAttr, s.Kind);
 *    e = NvRmMemHandleAllocAttr(Rm, &HandleAttr, &s.hMem);
 * </pre>
 * @{
 */

#include "nvcolor.h"
#include "nvrm_memmgr.h"

/** Defines the different display scan formats in the video planes. This
 * enumeration is built to support display formats progressive,
 * interlaced
 */
typedef enum
{
    /* Display scan formats */
    NvDisplayScanFormat_Progressive = 0,
    NvDisplayScanFormat_Interlaced,

    NvDisplayScanFormat_Force32 = 0x7FFFFFFF
} NvDisplayScanFormat;

/**
 * Width and height parameters for tiled surfaces.
 */
#define NVRM_SURFACE_TILE_WIDTH_LOG2     6UL
#define NVRM_SURFACE_TILE_WIDTH          (1UL<<NVRM_SURFACE_TILE_WIDTH_LOG2)
#define NVRM_SURFACE_TILE_HEIGHT_LOG2    4UL
#define NVRM_SURFACE_TILE_HEIGHT         (1UL<<NVRM_SURFACE_TILE_HEIGHT_LOG2)

#define NVRM_SURFACE_SUB_TILE_WIDTH_LOG2    4UL
#define NVRM_SURFACE_SUB_TILE_WIDTH         (1UL<<NVRM_SURFACE_SUB_TILE_WIDTH_LOG2)
#define NVRM_SURFACE_SUB_TILE_HEIGHT_LOG2   4UL
#define NVRM_SURFACE_SUB_TILE_HEIGHT        (1UL<<NVRM_SURFACE_SUB_TILE_HEIGHT_LOG2)

/**
 * Maximum number of NvRmSurfaces that GLES textureinfo/NvWsiPixmap contain
 */
#define NVRM_MAX_SURFACES 3

/**
 * The size of the MD5 string buffer used by NvRmSurfaceComputeMD5.
 */
#define NVRM_SURFACE_MD5_BUFFER_SIZE (2*16+1)

/**
 * The possible layouts that a surface can presently have. More layouts can be
 * added when invented by HW.
 *
 * Swizzled surfaces are not allowed here; the 3D driver will take care of
 * swizzling on its own, without RM's help.
 */
typedef enum
{
    /* Pitch: Address formula is:
     * <pre>
     *     y * Pitch + x * BytesPerPixel
     * </pre>
     * Also referred to as 'Pitchlinear'. */
    NvRmSurfaceLayout_Pitch = 1,

    /* DEPRECATED - Tiled format is only supported on pre-Kepler chips.
     *
     * Tiled: Surface is stored in 1024-byte blocks that are 64 bytes wide and
     * 16 rows tall, further divided into 256 B blocks that are 16 B x16. */
    NvRmSurfaceLayout_Tiled,

    /* BlockLinear: Surface is stored in a hardware defined, opaque
     * "blocklinear" organization. The exact byte layout of blocklinear format
     * is determined by surface \c Kind and \c BlockHeightLog2. */
    NvRmSurfaceLayout_Blocklinear,

    NvRmSurfaceLayout_Num,
    NvRmSurfaceLayout_Force32 = 0x7FFFFFFF
} NvRmSurfaceLayout;

#ifndef NV_ALIGN_POINTER
/* XXX - temporary hack for GPU sources which have a different version
 * of nvcommon.h
 */
#define NV_ALIGN_POINTER_NAME(f) f##Align
#define NV_ALIGN_POINTER(t, f)                  \
    union {                                     \
    t f;                                        \
    NvU64 NV_ALIGN_POINTER_NAME(f);             \
}
#endif

/**
 * An exposed (not opaque) structure representing a surface. Clients should
 * always initialize NvRmSurfaces using either NvRmSurfaceSetup or
 * NvRmMultiplanarSurfaceSetup.
 *
 * This struct must be tightly packed and the pointer 64-bit aligned
 * for sharing across 32-bit/64-bit process boundaries, even though
 * the pointer is never actually shared.  This is enforced by
 * compile-time asserts.
 */
typedef struct NvRmSurfaceRec
{
    /* The dimensions of the surface. */
    NvU32 Width;
    NvU32 Height;

    /* The color format of the surface, e.g., ::NvColorFormat_A8R8G8B8. */
    NvColorFormat ColorFormat;

    /* The layout of the surface, usually, ::NvRmSurfaceLayout_Blocklinear. */
    NvRmSurfaceLayout Layout;

    /* The pitch of the surface in bytes. The exact meaning of this value
     * depends on \c Layout. */
    NvU32 Pitch;

    /* If the surface is backed by an RM memory alloc, the handle of that
     * memory alloc and the offset where the surface starts within that alloc.
     * If the surface is not backed by an RM alloc, \c hMem should be NULL and
     * offset is a don't care. Offset must be aligned to no less than the word
     * size of \c ColorFormat. (For example, it must be 16-bit aligned for
     * R5G6B5 data.) */
    NvRmMemHandle hMem;
    NvU32 Offset;

    /* The "kind" of the surface -- used only if the \c Layout is blocklinear. */
    NvRmMemKind Kind;

    /* The block height of the surface -- used only if the \c Layout is
     * blocklinear. */
    NvU32 BlockHeightLog2;

    /* The display scan format of the surface - progressive/interlaced */
    NvDisplayScanFormat DisplayScanFormat;

    /* For interlaced pictures the Second field offset will represent offset
     * of the second field relative to the hMem. This variable must be aligned
     * to the layout specifications. */
    NvU32 SecondFieldOffset;

    NvU32 Reserved;

    /* If the surface is not backed by an RM alloc, the pointer to the storage.
     * This pointer must be aligned to no less than the word size of
     * \c ColorFormat. If the surface is backed by an RM alloc, \c pBase should
     * be NULL. (That is, exactly one of \c hMem and \c pBase should be
     * non-NULL.) */
    NV_ALIGN_POINTER(void*, pBase);

} NvRmSurface;

/**
 * Defines the different color formats in the YUV color space. This
 * enumeration describes the chroma sub-sampling ratios.
 */
typedef enum
{
    NvYuvColorFormat_Unspecified = 0,

    NvYuvColorFormat_YUV420,  // Horizontal and vertical sub-sampling
    NvYuvColorFormat_YUV422,  // Horizontal sub-sampling
    NvYuvColorFormat_YUV422R, // Vertical sub-sampling
    NvYuvColorFormat_YUV444,  // No sub-sampling

    NvYuvColorFormat_Force32 = 0x7FFFFFFF
} NvYuvColorFormat;

typedef enum
{
    NvRmSurfaceUnitSupport_3D          = 1 << 0,
    NvRmSurfaceUnitSupport_2D          = 1 << 1,
    NvRmSurfaceUnitSupport_Display     = 1 << 2,
    NvRmSurfaceUnitSupport_All         = 0xffffffff,
} NvRmSurfaceUnitSupport;

typedef enum
{
    /* Must terminate the attribute list */
    NvRmSurfaceAttribute_None = 0,

    /* Type: NvRmSurfaceLayout
     * Default: NvRmSurfaceGetDefaultLayout() */
    NvRmSurfaceAttribute_Layout,

    /* A bitmask of HW units that will access the surface.
     * Type: NvRmSurfaceUnitSupport,
     * Default: NvRmSurfaceUnitSupport_All */
    NvRmSurfaceAttribute_UnitSupport,

    /* Type: NvDisplayScanFormat
     * Default: NvDisplayScanFormat_Progressive */
    NvRmSurfaceAttribute_DisplayScanFormat,

    /* If this is NV_TRUE, the layout of the surface is optimized for
     * consumption by the display. This might mean choosing a non-standard
     * block height if the layout is blocklinear, for example.
     * Type: NvBool
     * Default: NV_FALSE */
    NvRmSurfaceAttribute_OptimizeForDisplay,

    /* If this is NV_TRUE, a compressible kind is selected for the surface.
     * This flag is silently ignored if the surface does not support a
     * compressible kind (e.g. layout not blocklinear, unsupported BPP).
     * Type: NvBool
     * Default: NV_FALSE */
    NvRmSurfaceAttribute_Compression,

} NvRmSurfaceAttribute;

/**
 * Sets up an NvRmSurface. All fields are initialized to sensible defaults.
 * hMem, Offset and and pBase are set to zero. Attributes array can be used for
 * communicating extra requirements.
 *
 * @param Surface The surface to set up.
 * @param Width Width of the surface in pixels.
 * @param Height Height of the surface in pixels.
 * @param ColorFormat Color format of the surface.
 * @param Attributes A NvRmSurfaceAttribute_None terminated list of attribute
 *        value pairs that override defaults. Pass NULL to use defaults for
 *        all attributes.
 */
void NvRmSurfaceSetup(
    NvRmSurface *Surface,
    NvU32 Width,
    NvU32 Height,
    NvColorFormat ColorFormat,
    const NvU32 *Attributes);

/**
 * Sets up a set of NvRmSurfaces describing a multiplanar surface.
 * All fields are initialized to sensible defaults.
 * hMem, Offset and and pBase are set to zero. Attributes array can be used for
 * communicating extra requirements.
 * This function should always be used instead of NvRmSurfaceSetup for
 * multiplanar surfaces, to ensure that parameters of the different planes are
 * compatible with each other.
 *
 * @param Surfaces The array of surfaces to set up.
 * @param NumSurfaces How many surfaces are in the array.
 * @param Width Width of the luma plane in pixels.
 * @param Height Height of the luma plane in pixels.
 * @param YuvFormat Format of the YUV surface, used to compute sub-sampling
 *        for the chroma planes. Use NvYuvColorFormat_Unspecified when the
 *        surface to set up only contains a single plane.
 * @param ColorFormats Array containing one color format for each surface.
 * @param Attributes A NvRmSurfaceAttribute_None terminated list of attribute
 *        value pairs that override defaults. Pass NULL to use defaults for
 *        all attributes.
 */
void NvRmMultiplanarSurfaceSetup(
    NvRmSurface *Surfaces,
    NvU32 NumSurfaces,
    NvU32 Width,
    NvU32 Height,
    NvYuvColorFormat YuvFormat,
    NvColorFormat *ColorFormats,
    const NvU32 *Attributes);

/**
 * Returns the system-wide default surface layout on the current platform.
 *
 * The default layout can be overridden by setting the default_layout config.
 * The allowed values are currently:
 *      pitch
 *      tiled
 *
 * On Android, for example, the default layout can be forced to pitchlinear
 * by issuing the following command:
 *
 *   adb shell setprop persist.tegra.default_layout pitch
 *
 * @returns Default surface layout on the current platform.
 */
NvRmSurfaceLayout NvRmSurfaceGetDefaultLayout(void);

/**
 * DEPRECATED - Use NvRmSurfaceSetup instead.
 *
 * Helper function to initialize an ::NvRmSurface to point to a preallocated
 * pitch layout chunk of non-RM memory. This function first memsets the
 * surface struct to zero, so it is not necessary to do the memset if you use
 * this function.
 *
 * @param pSurf A pointer to structure to fill in with description of surface.
 * @param Width Width of surface in pixels.
 * @param Height Height of surface in pixels.
 * @param ColorFormat Color format of surface.
 * @param Pitch Pitch of surface in bytes.
 * @param pBase A pointer to the first pixel of the surface.
 *     Must be aligned to no less than the word size of \a ColorFormat.
 */
void NvRmSurfaceInitNonRmPitch(
    NvRmSurface *pSurf,
    NvU32 Width,
    NvU32 Height,
    NvColorFormat ColorFormat,
    NvU32 Pitch,
    void *pBase);

/**
 * DEPRECATED - Use NvRmSurfaceSetup instead.
 *
 * Helper function to initialize an ::NvRmSurface to point to a preallocated
 * pitch layout chunk of RM memory. This function first memsets the surface
 * struct to zero, so it is not necessary to do the memset if you use this
 * function.
 *
 * @param pSurf A pointer to structure to fill in with description of surface.
 * @param Width Width of surface in pixels.
 * @param Height Height of surface in pixels.
 * @param ColorFormat Color format of surface.
 * @param Pitch Pitch of surface in bytes.
 * @param hMem A memory allocation handle returned from \c NvRmMemHandleAlloc.
 * @param Offset Offset within \a hMem to the first pixel of the surface.
 *     Must be aligned to no less than the word size of \c ColorFormat.
 */
void NvRmSurfaceInitRmPitch(
    NvRmSurface *pSurf,
    NvU32 Width,
    NvU32 Height,
    NvColorFormat ColorFormat,
    NvU32 Pitch,
    NvRmMemHandle hMem,
    NvU32 Offset);

/**
 * Helper function to free the storage associated with a surface. Calls
 * either ::NvRmMemHandleFree or ::NvOsFree to free the storage, then clears the
 * relevant handle/pointer to NULL.
 *
 * @param pSurf A pointer to structure describing the surface.
 */
void NvRmSurfaceFree(NvRmSurface *pSurf);

/** \def NV_RM_SURFACE_TILE_PITCH_NON_RESTRICTIVE
 *
 * The various flag bits that can be passed into NvRmSurfaceComputePitch().
 *
 * \c NV_RM_SURFACE_TILE_PITCH_NON_RESTRICTIVE: When computing the pitch for
 *     a tiled surface, by default the pitch is *not* simply rounded up to
 *     the next tile boundary. The reason for this is that not all units
 *     in AP15 support XY addressing (2D and VI). In order to support
 *     tiled surfaces that can still be addressed linearly, a further
 *     restriction is placed upon the pitch.  To handle linear addressing
 *     of a tiled surface, the pitch must match a value returned by the
 *     following equation:
 *     <pre>
 *         k * 2^(6+n); k = { 1, 3, 5, 7, 9, 11, 13, 15}
 *                      n = { 0, 1, 2, 3, 4, 5, 6 }
 *     </pre>
 *     Notice that there are only 56 possible restricted pitch values (8
 *     values for k, 7 for n). This has the potential to increase the
 *     amount of wasted memory. Check your use cases to see how much this
 *     affects you. Care has been taken to cover small-to-medium pitches
 *     well, but once you start going over pitches of 2048, the amount of
 *     memory wasted becomes more significant (but it never gets above 15%).
 *
 *     The \c NV_RM_SURFACE_TILE_PITCH_NON_RESTRICTIVE flag bit removes the
 *     extra restriction, and will calculate a pitch that is simply
 *     rounded up to the next tile boundary. On AP15 this will produce
 *     surfaces that cannot be referenced by 2D. This flag is provided
 *     for units that may require "tight" pitch padding (like 3D texturing).
 *
 *     @note In future chips, it is likely that all units will support
 *     XY addressing and this distinction will not be necessary. On such
 *     chips, passing in this flag bit will be ignored.
 */
#define NV_RM_SURFACE_TILE_PITCH_NON_RESTRICTIVE (1UL << 0)

/**
 * DEPRECATED - Use NvRmSurfaceSetup instead.
 *
 * Fills in an appropriate pitch for a surface. If\a  hDevice is NULL, we assume
 * that the surface will live in non-RM-allocated memory. If \a hDevice is
 * non-NULL, we align the surface appropriately for that device.
 *
 * @param hDevice Handle to device the surface is for, or NULL if none.
 * @param flags
 * @param pSurf A pointer to structure describing the surface.
 */
void NvRmSurfaceComputePitch(
    NvRmDeviceHandle hDevice,
    NvU32 flags,
    NvRmSurface *pSurf);

/**
 * Computes the number of bytes of storage that will be required for all the
 * pixels in a surface. This function assumes that the \c Width, \c Height,
 * \c ColorFormat, \c Layout, \c Pitch, \c SecondFieldOffset, and \c BlockHeight
 * (if \c Layout is BlockLinear) fields in the surface have already been
 * filled in.
 *
 * @param pSurf A pointer to structure describing the surface.
 *
 * @returns Size of surface in bytes.
 */
NvU32 NvRmSurfaceComputeSize(
    NvRmSurface *pSurf);

/**
 * Computes the alignment in bytes that the surface's memory will need to be
 * allocated with. This function assumes that the \c Width, \c Height,
 * \c  ColorFormat, \c Layout, and \c Pitch fields in the surface have already
 * been filled in.
 *
 * Alignment refers to minimum physical address alignment when using carveout,
 * and minimum IOVA (SMMU) address alignment when SMMU translation is enabled.
 *
 * @param hDevice Handle to device the surface is for, or NULL if none.
 * @param pSurf A pointer to structure describing the surface.
 *
 * @returns Required alignment for surface in bytes.
 */
NvU32 NvRmSurfaceComputeAlignment(
    NvRmDeviceHandle hDevice,
    NvRmSurface *pSurf);

/**
 * DEPRECATED - Use NvRmSurfaceSetup instead.
 *
 * Computes the second field offset for the second field to support interlaced
 * format within RM surface. This function assumes that the
 * \c Height, \c DisplayScanFormat, \c Layout, \c Offset, \c BlockHeightLog2 and
 * \c Pitch fields in the surface have already been filled in.
 * The offset is relative to the base of the \a hMem of the surface.
 *
 * @param hDevice Handle to device the surface is for, or NULL if none.
 * @param pSurf A pointer to structure describing the surface.
 *
 */
void NvRmSurfaceComputeSecondFieldOffset(
    NvRmDeviceHandle hDevice,
    NvRmSurface *pSurf);

/**
 * Computes the offset of a pixel (x,y) within a surface. This offset is
 * relative to the base of the surface, not to the base of the \a hMem of the
 * surface. That is, you still have to add in \c pSurf->Offset (if necessary) to
 * the value returned by this function.
 *
 * For any surface with less than 8 bpp, x*bpp must be a multiple of 8.
 *
 * @param pSurf A pointer to structure describing the surface.
 * @param x X coordinate of pixel.
 * @param y Y coordinate of pixel.
 *
 * @returns Offset of pixel within the surface.
 */
NvU32 NvRmSurfaceComputeOffset(
    NvRmSurface *pSurf,
    NvU32 x,
    NvU32 y);

/**
 * Writes a subregion of a surface. Not intended to be the fastest API (e.g.,
 * it will not make use of 2D acceleration), but should always work and should
 * at least be faster than calling ::NvRmSurfaceComputeOffset once per pixel.
 * Does not perform any color space conversion. Assumes that the surface is
 * not compressed.
 *
 * For any surface with less than 8 bpp, x*bpp and w*bpp must both be multiples
 * of 8. Further, the given region to write must be within the bounds of the
 * surface -- clipping is not suppported.
 *
 * @param pSurf A pointer to structure describing the surface.
 * @param x X coordinate of top-left pixel of rectangle to write.
 * @param y Y coordinate of top-left pixel of rectangle to write.
 * @param Width Width of rectangle to write.
 * @param Height Height of rectangle to write.
 * @param pSrcPixels A pointer to an array of Width*Height pixels to write. The
 *     pixels are stored in pitch format with a pitch of Width*BytesPerPixel.
 *    Must be aligned to no less than the word size of \c ColorFormat.
 */
void NvRmSurfaceWrite(
    NvRmSurface *pSurf,
    NvU32 x,
    NvU32 y,
    NvU32 Width,
    NvU32 Height,
    const void *pSrcPixels);

/**
 * Reads a subregion of a surface. Not intended to be the fastest API (e.g.,
 * it will not make use of 2D acceleration), but should always work and should
 * at least be faster than calling ::NvRmSurfaceComputeOffset once per pixel.
 * Does not perform any color space conversion or decompression.
 *
 * For any surface with less than 8 bpp, x*bpp and w*bpp must both be multiples
 * of 8. Further, the given region to read must be within the bounds of the
 * surface -- clipping is not suppported.
 *
 * @param pSurf A pointer to structure describing the surface.
 * @param x X coordinate of top-left pixel of rectangle to read.
 * @param y Y coordinate of top-left pixel of rectangle to read.
 * @param Width Width of rectangle to read.
 * @param Height Height of rectangle to read.
 * @param pDstPixels A pointer to an array of Width*Height pixels to fill in.
 *     The pixels are stored in pitch format with a pitch of
 *     Width*BytesPerPixel. Must be aligned to no less than the word size of
 *     \c ColorFormat.
 */
void NvRmSurfaceRead(
    NvRmSurface *pSurf,
    NvU32 x,
    NvU32 y,
    NvU32 Width,
    NvU32 Height,
    void *pDstPixels);

/**
 * Returns the YUV color format from an array of the \em N surfaces used to
 * represent the YUV planes.
 */
NvYuvColorFormat NvRmSurfaceGetYuvColorFormat(
    NvRmSurface **pSurf,
    NvU32 numSurfaces);

/**
 * Stereo info related defines
 */
#define NV_FIELD_MASK_INPLACE(F) (NV_FIELD_MASK(F) << NV_FIELD_SHIFT(F))
#define NV_FIELD_GET(F, V)       (V & NV_FIELD_MASK_INPLACE(F))
#define NV_FIELD_VAL(F, V)       ((V >> NV_FIELD_SHIFT(F)) & NV_FIELD_MASK(F))
#define NV_FIELD_SET(F, V)       ((V & NV_FIELD_MASK(F)) << NV_FIELD_SHIFT(F))
#define NV_FIELD_ADD(F, A, V)    ((A & (~NV_FIELD_MASK_INPLACE(F))) |  \
                                  NV_FIELD_SET(F,V))

// Stereo Layout details
// FPA- Frame packing arrangement (left-right, top-bottom etc)
// CI- Content interpretation. (left first, right first etc.)
#define NV_STEREO_ENABLE_BIT                             15
#define NV_STEREO_FPA_FIELD                           18:16
#define NV_STEREO_CI_FIELD                            20:19

#define NV_STEREO_ENABLE_MASK (1 << NV_STEREO_ENABLE_BIT)
#define NV_STEREO_GET_FPA(x)  NV_FIELD_VAL(NV_STEREO_FPA_FIELD, x)
#define NV_STEREO_GET_CI(x)   NV_FIELD_VAL(NV_STEREO_CI_FIELD, x)

#define NV_STEREO_MASK (NV_FIELD_MASK_INPLACE(NV_STEREO_FPA_FIELD) | \
                        NV_FIELD_MASK_INPLACE(NV_STEREO_CI_FIELD)  | \
                        NV_STEREO_ENABLE_MASK)

// Stereo property values based on frame packing arrangement SEI
// message in ITU-T H.264 (03/2009) spec.
// We plan to support only the following 3 modes:
// LEFTRIGHT: Two half width images kept side by side.
// TOPBOTTOM: Two half height images kept top and bottom.
// SEPARATE: Separate surface for left/right images.
// UNSPECIFIED: Separate surface stereo but the actual size
//              is determined by the HAL. (could be half-width)
#define NV_STEREO_FPA_CHECKERBOARD 0
#define NV_STEREO_FPA_COL_INTERLV  1
#define NV_STEREO_FPA_ROW_INTERLV  2
#define NV_STEREO_FPA_LEFTRIGHT    3
#define NV_STEREO_FPA_TOPBOTTOM    4
#define NV_STEREO_FPA_SEPARATE     5
#define NV_STEREO_FPA_UNSPECIFIED  6

// Position of left/right images while going left->right or
// top->bottom in one of the above frame packing arrangements
#define NV_STEREO_CI_LEFTFIRST    1
#define NV_STEREO_CI_RIGHTFIRST   2

#define NV_STEREO_ENABLE NV_STEREO_ENABLE_MASK
#define NV_STEREO_SBS  NV_FIELD_SET(NV_STEREO_FPA_FIELD,   \
                                    NV_STEREO_FPA_LEFTRIGHT)
#define NV_STEREO_TB NV_FIELD_SET(NV_STEREO_FPA_FIELD,   \
                                  NV_STEREO_FPA_TOPBOTTOM)
#define NV_STEREO_SF NV_FIELD_SET(NV_STEREO_FPA_FIELD,   \
                                  NV_STEREO_FPA_SEPARATE)
#define NV_STEREO_LEFT NV_FIELD_SET(NV_STEREO_CI_FIELD,   \
                                    NV_STEREO_CI_LEFTFIRST)
#define NV_STEREO_RIGHT NV_FIELD_SET(NV_STEREO_CI_FIELD,    \
                                     NV_STEREO_CI_RIGHTFIRST)
#define NV_STEREO_US NV_FIELD_SET(NV_STEREO_FPA_FIELD,     \
                                  NV_STEREO_FPA_UNSPECIFIED)

// Supported Stereo combinations.
typedef enum
{
    NV_STEREO_NONE       = 0,
    NV_STEREO_LEFTRIGHT  = NV_STEREO_SBS | NV_STEREO_LEFT  | NV_STEREO_ENABLE,
    NV_STEREO_RIGHTLEFT  = NV_STEREO_SBS | NV_STEREO_RIGHT | NV_STEREO_ENABLE,
    NV_STEREO_TOPBOTTOM  = NV_STEREO_TB  | NV_STEREO_LEFT  | NV_STEREO_ENABLE,
    NV_STEREO_BOTTOMTOP  = NV_STEREO_TB  | NV_STEREO_RIGHT | NV_STEREO_ENABLE,
    NV_STEREO_SEPARATELR = NV_STEREO_SF  | NV_STEREO_LEFT  | NV_STEREO_ENABLE,
    NV_STEREO_SEPARATERL = NV_STEREO_SF  | NV_STEREO_RIGHT | NV_STEREO_ENABLE,
    NV_STEREO_UNSPECIFIED = NV_STEREO_US | NV_STEREO_LEFT  | NV_STEREO_ENABLE,
} NvStereoType;

#define NV_STEREO_IS_SUPPORTED_MODE(FPA, CI)           \
                  ((FPA >= NV_STEREO_FPA_LEFTRIGHT) && \
                   (FPA <= NV_STEREO_FPA_UNSPECIFIED)) && \
                  ((CI == NV_STEREO_CI_LEFTFIRST) ||   \
                   (CI == NV_STEREO_CI_RIGHTFIRST))

#define NV_STEREO_GET_FPA_CI_FIELD(V)                       \
                   (NV_FIELD_GET(NV_STEREO_FPA_FIELD, V) |  \
                    NV_FIELD_GET(NV_STEREO_CI_FIELD, V))

/**
 * Computes a string representation of a list of surfaces, which is stored in
 * the specified buffer.
 *
 * @param buffer Buffer to store string represention
 * @param bufferSize Size of string buffer
 * @param surfaces Array of surfaces
 * @param numSurfaces Number of surfaces in array
 *
 * @returns The number of characters written to the string buffer (not including
 *          the nul terminator '\0'). The buffer was printed to successfully if
 *          the returned value is greater than -1 and less than \a bufferSize.
 */
NvS32 NvRmSurfaceComputeName(
    char *buffer,
    size_t bufferSize,
    const NvRmSurface *surfaces,
    NvU32 numSurfaces);

/**
 * Dumps the pixel data from a list of surfaces, writing it out in sequence to
 * the specified filename. The pixel data is written as laid out in memory,
 * except that extra padding between width and stride is discarded. The surfaces
 * are written out in sequence with no gap between their pixel data.
 *
 * This caller is responsible for ensuring surfaces are correctly synchronised.
 *
 * @param surfaces Array of surfaces
 * @param numSurfaces Number of surfaces in array
 * @param filename Filename to write pixel data

 * @retval NvSuccess Indicates the operation was successful
 */
NvError NvRmSurfaceDump(
    NvRmSurface *surfaces,
    NvU32 numSurfaces,
    const char *filename);

/**
 * Computes a MD5 hash from the pixel data from a list of surfaces and converts
 * the hash bytes into a printable hex string.
 *
 * This caller is responsible for ensuring surfaces are correctly synchronised.
 *
 * @param surfaces Array of surfaces
 * @param numSurfaces Number of surfaces in array
 * @param md5 A string buffer where the MD5 hash string should be written
 * @param size The size of \a md5, must equal NVRM_SURFACE_MD5_BUFFER_SIZE

 * @retval NvSuccess Indicates the operation was successful
 */
NvError NvRmSurfaceComputeMD5(
    NvRmSurface *surfaces,
    NvU32 numSurfaces,
    char *md5,
    NvU32 size);

#if defined(__cplusplus)
}
#endif

/** @} */
#endif
