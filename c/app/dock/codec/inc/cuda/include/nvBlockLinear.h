#ifndef __NVBLOCKLINEAR_H__
#define __NVBLOCKLINEAR_H__

#ifdef __cplusplus
extern "C" {
#endif

//
// nvBlockLinear.h
//
// Structure definitions and utility functions for handling block-linear
// surfaces.
//
// Block linear surfaces were added in NV50, and are similar in spirit to
// tiled surfaces
//
// First, the image is decomposed into gobs.  A gob on NV50 is a 256-byte 2D
// image block, consisting of 4 rows with 64 bytes of image data in each row.
// Within a gob, pixels are stored in normal raster-scan order (step first in
// X, then in Y).
//
// Then, the gobs are arranged into blocks.  The purpose of a block is to
// arrange a collection of gobs into a 2D/3D sub-surface whose sizes are
// roughly equal in each dimension.  This will tend to minimize the number of
// page transitions as we walk through a surface during rasterization or
// texturing.  The desired size of a block is implementation dependent (may
// want larger blocks for wide vidmem channels).  The class interface allows a
// programmable block size.
//
// For example, if you had a large 2D surface with 32-bit pixels, each gob
// would be 16x4 pixels in size.  If you want to arrange blocks inside a
// single 4KB page, you would stack 8 gobs vertically and 2 gobs horizontally.
// This would give you a 32x32 pixel block.  If you had a similar 3D surface,
// you might want to stack 1 gob horizontally, 2 gobs vertically, and 8 gobs
// in depth, which would give you a 16x8x8 pixel block.
//
// Within each block, individual gobs are stored in normal raster-scan order
// (step first in X, then in Y, then in Z).  Blocks themselves are also stored
// in normal raster-scan order.
//
// Starting with GM20X sparse texture support, blocks are conceptually 
// organized into tiles. A tile is a 64KB or 128KB (page size) set of blocks
// which are made resident in the page table as a group. The APIs require that 
// each tile in large allocations must be aligned on a page boundary, which 
// may require padding on the right edge of the texture.  For example, if tiles
// want to be 8 64B-wide gobs/blocks across but the rightmost tile only needs 5
// blocks, it would still need to be padded to 8 blocks.  A new control 
// <tileWidthInGobs> is provided to control that padding. The tile width 
// control is only implemented in the texture unit - other hardware units only 
// access a single facelod of a texture at a time, and the driver can program 
// them with the bloated size. The <tileWidthInGobs> parameter to these 
// functions is the requested tile width. Requesting a value of one will 
// produce the same (legacy) memory as on previous GPU architectures.
//
// If an image is smaller than a gob or a block, the size of the image is
// effectively padded out to the next boundary.  For example, 1D textures have
// only a single row, but there is no 1D-specific gob format.  Since each gob
// has four rows, 3/4 of each gob is wasted.  For small images, the driver
// should choose the gobs-per-block parameters to avoid storing completely
// unused gobs within in a block.  In the example above, if the 2D surface was
// only 16 pixels high, you would want to stack 4 gobs vertically and 4 gobs
// horizontally, which would yield a 64x16 block size.
//
// The gobs and blocks are arranged to represent a single 1D, 2D, or 3D
// surface.  Mipmapped and cubemap textures, as well as 2D texture image
// arrays (where the texture is an array of separate 2D images), consist of
// multiple images.
//
// If the texture is mipmapped, the LODs of each face are stored
// consecutively.  The class interface allows for only a single set of
// gobs-per-block parameters, which would correspond to those used for LOD
// zero.  For subsequent levels, the gobs-per-block parameters are computed
// as:
//
//   gobsPerBlockX(LOD n) = min(roundUpPow2(width(LOD n)/gobSizeX),
//                              gobsPerBlockX(LOD 0))
//   gobsPerBlockY(LOD n) = min(roundUpPow2(Height(LOD n)/gobSizeY),
//                              gobsPerBlockY(LOD 0))
//   gobsPerBlockZ(LOD n) = min(roundUpPow2(Depth(LOD n)),
//                              gobsPerBlockZ(LOD 0))
//
// Basically, the LOD zero gobs-per-block parameters are used, except if the
// LOD is smaller than a block in any dimension.  In that case, we instead use
// the number of gobs in that dimension, rounded up to the next power of two.
//
// After arranging all the mipmaps in the first image/face of the texture (if
// any), the subsequent image/face is stored immediately after the end of the
// current one.  The next cubeface or texture array element is then aligned to
// the shrunken block layout of mipmap level zero.  In this way, a cubeface's
// mipchain may be larger in size than a non-array texture mipchain, since a
// array texture must be padded to align the next texture in the array.

// Constants for fixed hardware gob sizes.
#define NV_TESLA_BLOCK_LINEAR_LOG_GOB_WIDTH       6    /*    64 bytes (2^6) */
#define NV_TESLA_BLOCK_LINEAR_LOG_GOB_HEIGHT      2    /* x   4 rows  (2^2) */
#define NV_TESLA_BLOCK_LINEAR_LOG_GOB_DEPTH       0    /* x   1 layer (2^0) */
#define NV_TESLA_BLOCK_LINEAR_GOB_SIZE            256  /* = 256 bytes (2^8) */

// Derived constants for fixed hardware gob sizes.
#define NV_TESLA_BLOCK_LINEAR_GOB_WIDTH               \
        (1 << NV_TESLA_BLOCK_LINEAR_LOG_GOB_WIDTH)
#define NV_TESLA_BLOCK_LINEAR_GOB_HEIGHT              \
        (1 << NV_TESLA_BLOCK_LINEAR_LOG_GOB_HEIGHT)
#define NV_TESLA_BLOCK_LINEAR_GOB_DEPTH               \
        (1 << NV_TESLA_BLOCK_LINEAR_LOG_GOB_DEPTH)

// Constants for fixed hardware gob sizes.
#define NV_FERMI_BLOCK_LINEAR_LOG_GOB_WIDTH       6    /*    64 bytes (2^6) */
#define NV_FERMI_BLOCK_LINEAR_LOG_GOB_HEIGHT      3    /* x   8 rows  (2^3) */
#define NV_FERMI_BLOCK_LINEAR_LOG_GOB_DEPTH       0    /* x   1 layer (2^0) */
#define NV_FERMI_BLOCK_LINEAR_GOB_SIZE            512  /* = 512 bytes (2^9) */

// Derived constants for fixed hardware gob sizes.
#define NV_FERMI_BLOCK_LINEAR_GOB_WIDTH               \
        (1 << NV_FERMI_BLOCK_LINEAR_LOG_GOB_WIDTH)
#define NV_FERMI_BLOCK_LINEAR_GOB_HEIGHT              \
        (1 << NV_FERMI_BLOCK_LINEAR_LOG_GOB_HEIGHT)
#define NV_FERMI_BLOCK_LINEAR_GOB_DEPTH               \
        (1 << NV_FERMI_BLOCK_LINEAR_LOG_GOB_DEPTH) 

// Defines for block linear parameters that are shared by all architectures.
#define NV_COMMON_BLOCK_LINEAR_LOG_GOB_WIDTH            NV_TESLA_BLOCK_LINEAR_LOG_GOB_WIDTH
#define NV_COMMON_BLOCK_LINEAR_LOG_GOB_HEIGHT(gob512B)  (NV_TESLA_BLOCK_LINEAR_LOG_GOB_HEIGHT + (gob512B))
#define NV_COMMON_BLOCK_LINEAR_LOG_GOB_DEPTH            NV_TESLA_BLOCK_LINEAR_LOG_GOB_DEPTH
#define NV_COMMON_BLOCK_LINEAR_GOB_SIZE(gob512B)        (NV_TESLA_BLOCK_LINEAR_GOB_SIZE * (1+(gob512B)))
#define NV_COMMON_BLOCK_LINEAR_GOB_WIDTH                NV_TESLA_BLOCK_LINEAR_GOB_WIDTH
#define NV_COMMON_BLOCK_LINEAR_GOB_HEIGHT(gob512B)      (NV_TESLA_BLOCK_LINEAR_GOB_HEIGHT * (1+(gob512B)))
#define NV_COMMON_BLOCK_LINEAR_GOB_DEPTH                NV_TESLA_BLOCK_LINEAR_GOB_DEPTH

// Temporary (?) defines so that the driver still compiles with the new
// TESLA/FERMI_BLOCK_LINEAR macro definitions
// XXX DO NOT use in OpenGL code.
#define NV_BLOCK_LINEAR_LOG_GOB_WIDTH       NV_TESLA_BLOCK_LINEAR_LOG_GOB_WIDTH
#define NV_BLOCK_LINEAR_LOG_GOB_HEIGHT      NV_TESLA_BLOCK_LINEAR_LOG_GOB_HEIGHT
#define NV_BLOCK_LINEAR_LOG_GOB_DEPTH       NV_TESLA_BLOCK_LINEAR_LOG_GOB_DEPTH
#define NV_BLOCK_LINEAR_GOB_SIZE            NV_TESLA_BLOCK_LINEAR_GOB_SIZE

// Derived constants for fixed hardware gob sizes.
#define NV_BLOCK_LINEAR_GOB_WIDTH           NV_TESLA_BLOCK_LINEAR_GOB_WIDTH
#define NV_BLOCK_LINEAR_GOB_HEIGHT          NV_TESLA_BLOCK_LINEAR_GOB_HEIGHT
#define NV_BLOCK_LINEAR_GOB_DEPTH           NV_TESLA_BLOCK_LINEAR_GOB_DEPTH

// NV_SIZE_IN_BLOCKS:  Macro to transform a size <baseSize> into a block
// count, where blocks of size 2^<logBlockSize>.  The base size is padded up
// to the next block boundary, if needed.
#define NV_SIZE_IN_BLOCKS(baseSize, logBlockSize)      \
    (((baseSize) + (1 << (logBlockSize)) - 1) >> (logBlockSize))

#if defined(NV_MACOSX_OPENGL)

// Mac GLRenderer platforms will supply their own surface view structure so
// it can maintain a structure that is compatible with all active shared 
// code branches
#include "nv_shared_types.h"

#else // defined(NV_MACOSX_OPENGL)

// NvBlockLinearLog2GobsPerBlock:  Holds the base2 logs of the size of a block
// (in gobs).
//
// IMPORTANT:  These parameters are necessary to interpret how block-linear
// memory is formatted.
typedef struct NvBlockLinearLog2GobsPerBlockRec {
    unsigned int        x, y, z;
} NvBlockLinearLog2GobsPerBlock;

// NvBlockLinearImageInfo:  Describes parameters for a given block linear
// image.  Includes both block linear formatting parameters and the overall
// size.
typedef struct NvBlockLinearImageInfoRec {

    // log2GobsPerBlock:  Holds the base2 logs of the number of gobs per block
    // in each dimension.
    NvBlockLinearLog2GobsPerBlock log2GobsPerBlock;

    // xBlocks, yBlocks, zBlocks:  Number of blocks in the image in the X, Y,
    // and Z dimensions.
    unsigned int    xBlocks, yBlocks, zBlocks;

    // offset:  Offset (in bytes) of this image from the surface base.  If the
    // surface is not mipmapped or an array, the offset is always zero.
    NvU64    offset;

    // size:  Size (in bytes) of this image.
    NvU64    size;

} NvBlockLinearImageInfo;

#endif // defined(NV_MACOSX_OPENGL)

// Enough data to determine the block size for a surface
// XXX most of these don't need to be 32 bits

/*  This function calculates block linear information about a texture
 *  mipmap level described by the parameters contained within *pTexParams. 
 *  lodBlockLinearInfo Information describing the block linear format of the
 *                     specified mipmap level is returned here.
 *  baseBlockLinearInfo Information describing the block linear format of the
 *                      base level of the image is supplied here.
 *  widthInPixels The width of the base mipmap level, in pixels.
 *  heightInPixels The height of the base mipmap level, in pixels.
 *  depthInPixels The depth of the base mipmap level, in pixels.
 *  dim The dimension of the texture, 1d, 2d, or 3d.
 *  elementSize The size of an "element", in bytes.
 *  compressionBlockWidthLog2 The base two logarithm of the width of a
 *                            compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of a
 *                             compression block.
 *  mipMapLevel The mipmap level of interest.
 *  borderSize The width of the texture's border, in pixels.
 *  tileWidthInGobs The width of a tile in the base mipmap level, in gobs (gm20x+)
 *  The NV_STDCALL is included so that source that is not compiled with
 *  __stdcall knows how to call this function.
 */
void NV_STDCALL nvBlockLinearGetImageLevelInfo(NvBlockLinearImageInfo *lodBlockLinearInfo,        // OUT
                                               const NvBlockLinearImageInfo *baseBlockLinearInfo, // IN
                                               const unsigned int widthInPixels,
                                               const unsigned int heightInPixels,
                                               const unsigned int depthInPixels,
                                               const unsigned int dim,
                                               const unsigned int elementSize,
                                               const unsigned int compressionBlockWidthLog2,
                                               const unsigned int compressionBlockHeightLog2,
                                               const unsigned int mipMapLevel,
                                               const unsigned int borderSize);

void NV_STDCALL nvBlockLinearGetImageLevelInfo_512BGOB(NvBlockLinearImageInfo *lodBlockLinearInfo,        // OUT
                                               const NvBlockLinearImageInfo *baseBlockLinearInfo, // IN
                                               const unsigned int widthInPixels,
                                               const unsigned int heightInPixels,
                                               const unsigned int depthInPixels,
                                               const unsigned int dim,
                                               const unsigned int elementSize,
                                               const unsigned int compressionBlockWidthLog2,
                                               const unsigned int compressionBlockHeightLog2,
                                               const unsigned int mipMapLevel,
                                               const unsigned int borderSize);

void NV_STDCALL nvBlockLinearGetImageLevelInfo_512BGOB_NP2Comp(NvBlockLinearImageInfo *lodBlockLinearInfo,        // OUT
                                               const NvBlockLinearImageInfo *baseBlockLinearInfo, // IN
                                               const unsigned int widthInPixels,
                                               const unsigned int heightInPixels,
                                               const unsigned int depthInPixels,
                                               const unsigned int dim,
                                               const unsigned int elementSize,
                                               const unsigned int compressionBlockWidth,
                                               const unsigned int compressionBlockHeight,
                                               const unsigned int mipMapLevel,
                                               const unsigned int borderSize);

void NV_STDCALL nvBlockLinearGetImageLevelInfo_512BGOB_NP2Comp_Tile(NvBlockLinearImageInfo *lodBlockLinearInfo,        // OUT
                                               const NvBlockLinearImageInfo *baseBlockLinearInfo, // IN
                                               const unsigned int widthInPixels,
                                               const unsigned int heightInPixels,
                                               const unsigned int depthInPixels,
                                               const unsigned int dim,
                                               const unsigned int elementSize,
                                               const unsigned int compressionBlockWidth,
                                               const unsigned int compressionBlockHeight,
                                               const unsigned int mipMapLevel,
                                               const unsigned int borderSize,
                                               const unsigned int tileWidthInGobs);

typedef void (NV_STDCALL *NvBlockLinearGetImageLevelFunc)
    (NvBlockLinearImageInfo *lodBlockLinearInfo,        // OUT
     const NvBlockLinearImageInfo *baseBlockLinearInfo, // IN
     const unsigned int widthInPixels,
     const unsigned int heightInPixels,
     const unsigned int depthInPixels,
     const unsigned int dim,
     const unsigned int elementSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int mipMapLevel,
     const unsigned int borderSize);


/*  This function calculates block linear information about a texture
 *  mipmap level described by the parameters contained within *pTexParams. 
 *  pBlockLinearInfo Information describing the block linear format of the
 *                   mipmap level described by *pTexParams is returned here.
 *  widthInPixels The width of the base mipmap level, in pixels.
 *  heightInPixels The height of the base mipmap level, in pixels.
 *  depthInPixels The depth of the base mipmap level, in pixels.
 *  dim The dimension of the texture, 1d, 2d, or 3d.
 *  elementSize The size of an "element", in bytes.
 *  compressionBlockWidthLog2 The base two logarithm of the width of a
 *                            compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of a
 *                             compression block.
 *  mipMapLevel The mipmap level of interest.
 *  borderSize The width of the texture's border, in pixels.
 *  superblockSize The size of a superblock, in bytes.  Also referred to
 *                 as DRAM page stride.
 *  The NV_STDCALL is included so that source that is not compiled with
 *  __stdcall knows how to call this function.
 */
void NV_STDCALL nvBlockLinearGetTexLevelInfo(NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidthLog2,
                                             const unsigned int compressionBlockHeightLog2,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const unsigned int superblockSize);

void NV_STDCALL nvBlockLinearGetTexLevelInfo_512BGOB(NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidthLog2,
                                             const unsigned int compressionBlockHeightLog2,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const unsigned int superblockSize);

void NV_STDCALL nvBlockLinearGetTexLevelInfo_512BGOB_NP2Comp(NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidth,
                                             const unsigned int compressionBlockHeight,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const unsigned int superblockSize);

typedef void (NV_STDCALL * NvBlockLineGetTexLevelInfoFunc)
    (NvBlockLinearImageInfo *pBlockLinearInfo,
     const unsigned int widthInPixels,
     const unsigned int heightInPixels,
     const unsigned int depthInPixels,
     const unsigned int dim,
     const unsigned int elementSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int mipMapLevel,
     const unsigned int borderSize,
     const unsigned int superblockSize);


/*  This function calculates block linear information about a texture
 *  mipmap level described by the parameters contained within *pTexParams. 
 *  This function is different from nvBlockLinearGetTexLevelInfo since it
 *  uses the specified gob per block layout instead of computing the
 *  optimal one
 *  pBlockLinearInfo Information describing the block linear format of the
 *                   mipmap level described by *pTexParams is returned here.
 *  widthInPixels The width of the base mipmap level, in pixels.
 *  heightInPixels The height of the base mipmap level, in pixels.
 *  depthInPixels The depth of the base mipmap level, in pixels.
 *  dim The dimension of the texture, 1d, 2d, or 3d.
 *  elementSize The size of an "element", in bytes.
 *  compressionBlockWidthLog2 The base two logarithm of the width of a
 *                            compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of a
 *                             compression block.
 *  mipMapLevel The mipmap level of interest.
 *  borderSize The width of the texture's border, in pixels.
 *  tileWidthInGobs The width of a tile in the base mipmap level, in gobs (gm20x+)
 *  pBlockLayout The gob per block layout that should be used for this surface
 *               (user specified - may not be the same as the optimal, must != NULL)
 *  The NV_STDCALL is included so that source that is not compiled with
 *  __stdcall knows how to call this function.
 */
void NV_STDCALL nvBlockLinearGetTexLevelInfoGivenBlockLayout
                                            (NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidthLog2,
                                             const unsigned int compressionBlockHeightLog2,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const NvBlockLinearLog2GobsPerBlock *pBlockLayout);

void NV_STDCALL nvBlockLinearGetTexLevelInfoGivenBlockLayout_512BGOB
                                            (NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidthLog2,
                                             const unsigned int compressionBlockHeightLog2,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const NvBlockLinearLog2GobsPerBlock *pBlockLayout);

void NV_STDCALL nvBlockLinearGetTexLevelInfoGivenBlockLayout_512BGOB_NP2Comp
                                            (NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidth,
                                             const unsigned int compressionBlockHeight,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const NvBlockLinearLog2GobsPerBlock *pBlockLayout);

void NV_STDCALL nvBlockLinearGetTexLevelInfoGivenBlockLayout_512BGOB_NP2Comp_Tile
                                            (NvBlockLinearImageInfo *pBlockLinearInfo,
                                             const unsigned int widthInPixels,
                                             const unsigned int heightInPixels,
                                             const unsigned int depthInPixels,
                                             const unsigned int dim,
                                             const unsigned int elementSize,
                                             const unsigned int compressionBlockWidth,
                                             const unsigned int compressionBlockHeight,
                                             const unsigned int mipMapLevel,
                                             const unsigned int borderSize,
                                             const unsigned int tileWidthInGobs,
                                             const NvBlockLinearLog2GobsPerBlock *pBlockLayout);

typedef void (NV_STDCALL * NvBlockLinearGetTexLevelInfoGivenBlockLayoutFunc)
    (NvBlockLinearImageInfo *pBlockLinearInfo,
     const unsigned int widthInPixels,
     const unsigned int heightInPixels,
     const unsigned int depthInPixels,
     const unsigned int dim,
     const unsigned int elementSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int mipMapLevel,
     const unsigned int borderSize,
     const NvBlockLinearLog2GobsPerBlock *pBlockLayout);


/*  This function calculates the number of gobs per block of a surface
 *  of interest given the size of a superblock.
 *  blockInfo The number of gobs per block in the x, y, and z dimensions
 *            of the surface described by *surfInfo are returned here.
 *  elementSize The size of an image element, in bytes.
 *  compressionBlockWidthLog2 The base two logarithm of the width of a compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of a compression block.
 *  superblockSize The size of a superblock, in bytes.  Also referred to
 *                 as DRAM page stride. 
 */
void NV_STDCALL nvBlockLinearGetOptimalBlockLayout(NvBlockLinearLog2GobsPerBlock *blockInfo,
                                        const unsigned int elementSize,
                                        const unsigned int compressionBlockWidthLog2,
                                        const unsigned int compressionBlockHeightLog2,
                                        const unsigned int superblockSize,
                                        const unsigned int dim,
                                        const unsigned int width,
                                        const unsigned int height,
                                        const unsigned int depth);

void NV_STDCALL nvBlockLinearGetOptimalBlockLayout_512BGOB(NvBlockLinearLog2GobsPerBlock *blockInfo,
                                        const unsigned int elementSize,
                                        const unsigned int compressionBlockWidthLog2,
                                        const unsigned int compressionBlockHeightLog2,
                                        const unsigned int superblockSize,
                                        const unsigned int dim,
                                        const unsigned int width,
                                        const unsigned int height,
                                        const unsigned int depth);

void NV_STDCALL nvBlockLinearGetOptimalBlockLayout_512BGOB_NP2Comp(NvBlockLinearLog2GobsPerBlock *blockInfo,
                                        const unsigned int elementSize,
                                        const unsigned int compressionBlockWidth,
                                        const unsigned int compressionBlockHeight,
                                        const unsigned int superblockSize,
                                        const unsigned int dim,
                                        const unsigned int width,
                                        const unsigned int height,
                                        const unsigned int depth);

typedef void (NV_STDCALL * NvBlockLinearGetOptimalBlockLayoutFunc)
    (NvBlockLinearLog2GobsPerBlock *blockInfo,
     const unsigned int elementSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int superblockSize,
     const unsigned int dim,
     const unsigned int width,
     const unsigned int height,
     const unsigned int depth);

void NV_STDCALL nvBlockLinearGetReducedFootprint2DBlockLayout_512BGOB(NvBlockLinearLog2GobsPerBlock *blockInfo,
                                                                      const unsigned int elementSize,
                                                                      const unsigned int compressionBlockWidthLog2,
                                                                      const unsigned int compressionBlockHeightLog2,
                                                                      const unsigned int superblockSize,
                                                                      const unsigned int dim,
                                                                      const unsigned int width,
                                                                      const unsigned int height,
                                                                      const unsigned int depth);

void NV_STDCALL nvBlockLinearGetReducedFootprint2DBlockLayout_512BGOB_NP2Comp(NvBlockLinearLog2GobsPerBlock *blockInfo,
                                                                              const unsigned int elementSize,
                                                                              const unsigned int compressionBlockWidth,
                                                                              const unsigned int compressionBlockHeight,
                                                                              const unsigned int superblockSize,
                                                                              const unsigned int dim,
                                                                              const unsigned int width,
                                                                              const unsigned int height,
                                                                              const unsigned int depth);

typedef void (NV_STDCALL * NvBlockLinearGetBlockLayoutFunc)
    (NvBlockLinearLog2GobsPerBlock *blockInfo,
     const unsigned int elementSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int superblockSize,
     const unsigned int dim,
     const unsigned int width,
     const unsigned int height,
     const unsigned int depth);

/*! This function shrinks block linear gob sizes to better fit the dimensions of an
 *  image, if the image is smaller than the gob.
 *  gobSize The gob sizes to shrink.
 *  width The width of the image, in elements.
 *  height The height of the image, in elements.
 *  depth The depth of the image, in elements.
 *  elementSize The number of bytes per element of the image in question.
 *
 *  Note: width, height, and depth are specified as "elements", not necessarily pixels.
 *        For example, if compression is being used, then these are the dimensions of the image
 *        in compression blocks.
 */
void NV_STDCALL nvBlockLinearShrinkBlock(NvBlockLinearLog2GobsPerBlock *gobSize,
                              const unsigned int width,
                              const unsigned int height,
                              const unsigned int depth,
                              const unsigned int elementSize);

void NV_STDCALL nvBlockLinearShrinkBlock_512BGOB(NvBlockLinearLog2GobsPerBlock *gobSize,
                              const unsigned int width,
                              const unsigned int height,
                              const unsigned int depth,
                              const unsigned int elementSize);

typedef void (NV_STDCALL * NvBlockLinearShrinkBlockFunc)
    (NvBlockLinearLog2GobsPerBlock *gobSize,
     const unsigned int width,
     const unsigned int height,
     const unsigned int depth,
     const unsigned int elementSize);


/*! This function calculates the size (in blocks) of a block linear image, given the image's dimensions,
 *  and the block layout to use.
 *  gobsPerBlock The log of the number of gobs to pack per block in the block linear image.
 *  width The width of the block linear image, in pixels.
 *  height The height of the block linear image, in pixels.
 *  depth The depth of the block linear image, in pixels.
 *  bpp The number of bytes per pixel in the image.
 *  tileWidthInGobs The width of a tile in the base mipmap level, in gobs (gm20x+)
 *  numberOfBlocksInX The number of blocks the image spans in the horizontal dimension
 *                    is returned here.
 *  numberOfBlocksInY The number of blocks the image spans in the vertical dimension
 *                    is returned here.
 *  numberOfBlocksInZ The number of blocks the image spans in the depth dimension
 *                    is returned here.
 *  Note that this function can only compute the size of a single image (or mipmap level) - not an entire mipchain.
 */
void NV_STDCALL nvBlockLinearImageCalculateSizeInBlocks(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                             const unsigned int width,
                                             const unsigned int height,
                                             const unsigned int depth,
                                             const unsigned int bpp,
                                             unsigned int *numberOfBlocksInX,
                                             unsigned int *numberOfBlocksInY,
                                             unsigned int *numberOfBlocksInZ);

void NV_STDCALL nvBlockLinearImageCalculateSizeInBlocks_512BGOB(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                             const unsigned int width,
                                             const unsigned int height,
                                             const unsigned int depth,
                                             const unsigned int bpp,
                                             unsigned int *numberOfBlocksInX,
                                             unsigned int *numberOfBlocksInY,
                                             unsigned int *numberOfBlocksInZ);

void NV_STDCALL nvBlockLinearImageCalculateSizeInBlocks_512BGOB_Tile(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                             const unsigned int width,
                                             const unsigned int height,
                                             const unsigned int depth,
                                             const unsigned int bpp,
                                             const unsigned int tileWidthInGobs,
                                             unsigned int *numberOfBlocksInX,
                                             unsigned int *numberOfBlocksInY,
                                             unsigned int *numberOfBlocksInZ);

typedef void (NV_STDCALL * NvBlockLinearImageCalculateSizeInBlocksFunc)
    (const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
     const unsigned int width,
     const unsigned int height,
     const unsigned int depth,
     const unsigned int bpp,
     unsigned int *numberOfBlocksInX,
     unsigned int *numberOfBlocksInY,
     unsigned int *numberOfBlocksInZ);


/*! This function calculates the size (in bytes) of a block linear image, given the image's block layout.
 *  gobsPerBlock The log of the number of gobs to pack per block in the block linear image.
 *  widthInBlocks The width of the block linear image, in blocks.
 *  heightInBlocks The height of the block linear image, in blocks.
 *  depthInBlocks The depth of the block linear image, in blocks.
 *  return The size of the described block linear image, in bytes.
 *  Note that this function can only compute the size of a single image (or mipmap level) not an entire mipchain.
 */
NvU64 NV_STDCALL nvBlockLinearImageCalculateSizeInBytes(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                    const unsigned int widthInBlocks,
                                                    const unsigned int heightInBlocks,
                                                    const unsigned int depthInBlocks);

NvU64 NV_STDCALL nvBlockLinearImageCalculateSizeInBytes_512BGOB(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                    const unsigned int widthInBlocks,
                                                    const unsigned int heightInBlocks,
                                                    const unsigned int depthInBlocks);


/*! This function calculates the size (in bytes) of a block linear mipchain, given the base level's dimensions
 *  and block layout.  This function compensates for the hardware's automatic block shrinking across mipmap levels.
 *  Use this function when you have a desired gobs per block layout and you are only interested in a mipmap's size.
 *
 *  gobsPerBlock The log of the number of gobs to pack per block in the base mipmap level. Note that the hardware
 *               may determine that this layout is "too large" and shrink it.  This function compensates for this case.
 *               If you don't know or care about the block layout, set this parameter to 0, and the computation will choose
 *               the optimal format for you.
 *  widthInPixels The width of the base mipmap level, in pixels.
 *  heightInPixels The height of the base mipmap level, in pixels.
 *  depthInPixels The depth of the base mipmap level, in pixels.
 *  dim The dimension of the texture, 1d, 2d, or 3d.
 *  elementSize The size of an element, in bytes.
 *  borderSizeInPixels The size of the mipmap's border, in pixels
 *  compressionBlockWidthLog2 The base two logarithm of width of the mipmap's compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of the mipmap's compression block.
 *  numberOfMipMapLevels The total number of mipmap levels of the chain.
 *  superblockSize The size of a superblock, in bytes.  Also referred to
 *                 as DRAM page stride. 
 *  tileWidthInGobs The width of a tile in the base mipmap level, in gobs (gm20x+)
 *  return The total size of the mipchain, in bytes.
 */
NvU64 NV_STDCALL nvBlockLinearMipMapCalculateSizeInBytes(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                const unsigned int widthInPixels,
                                                                const unsigned int heightInPixels,
                                                                const unsigned int depthInPixels,
                                                                const unsigned int dim,
                                                                const unsigned int elementSize,
                                                                const unsigned int borderSize,
                                                                const unsigned int compressionBlockWidthLog2,
                                                                const unsigned int compressionBlockHeightLog2,
                                                                const unsigned int numberOfMipMapLevels,
                                                                const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearMipMapCalculateSizeInBytes_512BGOB(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                const unsigned int widthInPixels,
                                                                const unsigned int heightInPixels,
                                                                const unsigned int depthInPixels,
                                                                const unsigned int dim,
                                                                const unsigned int elementSize,
                                                                const unsigned int borderSize,
                                                                const unsigned int compressionBlockWidthLog2,
                                                                const unsigned int compressionBlockHeightLog2,
                                                                const unsigned int numberOfMipMapLevels,
                                                                const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearMipMapCalculateSizeInBytes_512BGOB_NP2Comp(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                const unsigned int widthInPixels,
                                                                const unsigned int heightInPixels,
                                                                const unsigned int depthInPixels,
                                                                const unsigned int dim,
                                                                const unsigned int elementSize,
                                                                const unsigned int borderSize,
                                                                const unsigned int compressionBlockWidth,
                                                                const unsigned int compressionBlockHeight,
                                                                const unsigned int numberOfMipMapLevels,
                                                                const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearMipMapCalculateSizeInBytes_512BGOB_NP2Comp_Tile(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                const unsigned int widthInPixels,
                                                                const unsigned int heightInPixels,
                                                                const unsigned int depthInPixels,
                                                                const unsigned int dim,
                                                                const unsigned int elementSize,
                                                                const unsigned int borderSize,
                                                                const unsigned int compressionBlockWidth,
                                                                const unsigned int compressionBlockHeight,
                                                                const unsigned int numberOfMipMapLevels,
                                                                const unsigned int superblockSize,
                                                                const unsigned int tileWidthInGobs);


/*! This function calculates the size (in bytes) of a block linear mip array, given the dimensions of the surface's
 *  base miplevel.  This function compensates for the hardware's automatic block shrinking across mipmap levels.
 *  Use this function when you have a desired gobs per block layout and you are only interested in a single array's size.
 *
 *  gobsPerBlock The log of the number of gobs to pack per block in the base mipmap level. Note that the hardware
 *               may determine that this layout is "too large" and shrink it.  This function compensates for this case.
 *               If you don't know or care about the block layout, set this parameter to 0, and the computation will choose
 *               the optimal format for you.
 *  widthInPixels The width of the base mipmap level, in pixels.
 *  heightInPixels The height of the base mipmap level, in pixels.
 *  depthInPixels The depth of the base mipmap level, in pixels.
 *  dim The dimension of the texture, 1d, 2d, or 3d.
 *  elementSize The size of an element, in bytes.
 *  borderSizeInPixels The size of the mipmap's border, in pixels
 *  compressionBlockWidthLog2 The base two logarithm of width of the mipmap's compression block.
 *  compressionBlockHeightLog2 The base two logarithm of the height of the mipmap's compression block.
 *  numberOfMipMapLevels The total number of mipmap levels of the chain.
 *  superblockSize The size of a superblock, in bytes.  Also referred to
 *                 as DRAM page stride. 
 *  tileWidthInGobs The width of a tile in the base mipmap level, in gobs (gm20x+)
 *  return The total size of the mipchain, in bytes.
 *
 *  Note: The reason we include this call in addition to nvBlockLinearMipMapCalculateSizeInBytes is because each array mip
 *        chain must be aligned to base mipmap level block boundaries.  For this reason, a array's mipchain may be 
 *        slightly larger than an identical non-array mipchain.
 */
NvU64 NV_STDCALL nvBlockLinearArrayMipMapCalculateSizeInBytes(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                     const unsigned int widthInPixels,
                                                                     const unsigned int heightInPixels,
                                                                     const unsigned int depthInPixels,
                                                                     const unsigned int dim,
                                                                     const unsigned int elementSize,
                                                                     const unsigned int borderSize,
                                                                     const unsigned int compressionBlockWidthLog2,
                                                                     const unsigned int compressionBlockHeightLog2,
                                                                     const unsigned int numberOfMipMapLevels,
                                                                     const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearArrayMipMapCalculateSizeInBytes_512BGOB(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                              const unsigned int widthInPixels,
                                                                              const unsigned int heightInPixels,
                                                                              const unsigned int depthInPixels,
                                                                              const unsigned int dim,
                                                                              const unsigned int elementSize,
                                                                              const unsigned int borderSize,
                                                                              const unsigned int compressionBlockWidthLog2,
                                                                              const unsigned int compressionBlockHeightLog2,
                                                                              const unsigned int numberOfMipMapLevels,
                                                                              const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearArrayMipMapCalculateSizeInBytes_512BGOB_NP2Comp(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                              const unsigned int widthInPixels,
                                                                              const unsigned int heightInPixels,
                                                                              const unsigned int depthInPixels,
                                                                              const unsigned int dim,
                                                                              const unsigned int elementSize,
                                                                              const unsigned int borderSize,
                                                                              const unsigned int compressionBlockWidth,
                                                                              const unsigned int compressionBlockHeight,
                                                                              const unsigned int numberOfMipMapLevels,
                                                                              const unsigned int superblockSize);

NvU64 NV_STDCALL nvBlockLinearArrayMipMapCalculateSizeInBytes_512BGOB_NP2Comp_Tile(const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
                                                                              const unsigned int widthInPixels,
                                                                              const unsigned int heightInPixels,
                                                                              const unsigned int depthInPixels,
                                                                              const unsigned int dim,
                                                                              const unsigned int elementSize,
                                                                              const unsigned int borderSize,
                                                                              const unsigned int compressionBlockWidth,
                                                                              const unsigned int compressionBlockHeight,
                                                                              const unsigned int numberOfMipMapLevels,
                                                                              const unsigned int superblockSize,
                                                                              const unsigned int tileWidthInGobs);


/* This function pointer type can be used to call both the MipMap" and
 * "ArrayMipmap" functions above.
 */
typedef NvU64 (NV_STDCALL * NvBlockLinearCalcMipMapSizeFunc)
    (const NvBlockLinearLog2GobsPerBlock *gobsPerBlock,
     const unsigned int widthInPixels,
     const unsigned int heightInPixels,
     const unsigned int depthInPixels,
     const unsigned int dim,
     const unsigned int elementSize,
     const unsigned int borderSize,
     const unsigned int compressionBlockWidthLog2,
     const unsigned int compressionBlockHeightLog2,
     const unsigned int numberOfMipMapLevels,
     const unsigned int superblockSize);

/*! This function calculates a depth slice offset for block linear.
 */
    
NvU64 NV_STDCALL nvBlockLinearCalculateDepthSliceOffset(NvU32 z_offset,
                                                        NvBlockLinearImageInfo *bl_info);

    
NvU64 NV_STDCALL nvBlockLinearCalculateDepthSliceOffset_512BGOB(NvU32 z_offset,
                                                                NvBlockLinearImageInfo *bl_info);
    


// Compute the "shrunk" tile width for the given size of a mipmap level. This
// function will return <tileWidthInGobs> if the level is still so large that
// the hardware will use the tileWidth parameter, otherwise it will return 1.
unsigned int NV_STDCALL nvShrinkTileWidth(const NvBlockLinearLog2GobsPerBlock *blockLayout, 
                                          const unsigned int w, 
                                          const unsigned int h, 
                                          const unsigned int d, 
                                          const unsigned int elementSize, 
                                          const unsigned int tileWidthInGobs);

#ifdef __cplusplus
} // extern "C"
#endif


#endif // #ifndef __NVBLOCKLINEAR_H__


