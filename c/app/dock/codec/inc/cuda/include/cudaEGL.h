/*
 * Copyright 2014 NVIDIA Corporation.  All rights reserved.
 *
 * NOTICE TO LICENSEE:
 *
 * This source code and/or documentation ("Licensed Deliverables") are
 * subject to NVIDIA intellectual property rights under U.S. and
 * international Copyright laws.
 *
 * These Licensed Deliverables contained herein is PROPRIETARY and
 * CONFIDENTIAL to NVIDIA and is being provided under the terms and
 * conditions of a form of NVIDIA software license agreement by and
 * between NVIDIA and Licensee ("License Agreement") or electronically
 * accepted by Licensee.  Notwithstanding any terms or conditions to
 * the contrary in the License Agreement, reproduction or disclosure
 * of the Licensed Deliverables to any third party without the express
 * written consent of NVIDIA is prohibited.
 *
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, NVIDIA MAKES NO REPRESENTATION ABOUT THE
 * SUITABILITY OF THESE LICENSED DELIVERABLES FOR ANY PURPOSE.  IT IS
 * PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY OF ANY KIND.
 * NVIDIA DISCLAIMS ALL WARRANTIES WITH REGARD TO THESE LICENSED
 * DELIVERABLES, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY,
 * NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE.
 * NOTWITHSTANDING ANY TERMS OR CONDITIONS TO THE CONTRARY IN THE
 * LICENSE AGREEMENT, IN NO EVENT SHALL NVIDIA BE LIABLE FOR ANY
 * SPECIAL, INDIRECT, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THESE LICENSED DELIVERABLES.
 *
 * U.S. Government End Users.  These Licensed Deliverables are a
 * "commercial item" as that term is defined at 48 C.F.R. 2.101 (OCT
 * 1995), consisting of "commercial computer software" and "commercial
 * computer software documentation" as such terms are used in 48
 * C.F.R. 12.212 (SEPT 1995) and is provided to the U.S. Government
 * only as a commercial end item.  Consistent with 48 C.F.R.12.212 and
 * 48 C.F.R. 227.7202-1 through 227.7202-4 (JUNE 1995), all
 * U.S. Government End Users acquire the Licensed Deliverables with
 * only those rights set forth herein.
 *
 * Any use of the Licensed Deliverables in individual and commercial
 * software must include, in the user documentation and internal
 * comments to the code, the above Disclaimer and U.S. Government End
 * Users Notice.
 */

#ifndef CUDAEGL_H
#define CUDAEGL_H

#include "cuda.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

/** 
  CUDA API versioning support
 */
#if defined(CUDA_FORCE_API_VERSION)
    #if (CUDA_FORCE_API_VERSION == 3010)
        #define __CUDA_API_VERSION 3010
    #else
        #error "Unsupported value of CUDA_FORCE_API_VERSION"
    #endif
#else
    #define __CUDA_API_VERSION 6050
#endif /* CUDA_FORCE_API_VERSION */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup CUDA_EGL EGLIMAGE Interoperability
 * \ingroup CUDA_DRIVER
 *
 * ___MANBRIEF___ EGL interoperability functions of the low-level CUDA driver
 * API (___CURRENT_FILE___) ___ENDMANBRIEF___
 *
 * This section describes the EGL interoperability functions of the
 * low-level CUDA driver application programming interface.
 *
 * @{
 */

#if __CUDA_API_VERSION >= 6050

#define MAX_PLANES 3

typedef enum CUeglFrameType_enum {
    CU_EGL_FRAME_TYPE_ARRAY = 0,
    CU_EGL_FRAME_TYPE_PITCH = 1,
} CUeglFrameType;

/*
 * YUV Formats Explanation:
  1. CU_FORMAT_YUV420_PLANAR : Y, U, V in three surface, each in a separate surface,
         U/V width = 1/2 Y width, U/V height = 1/2 Y height
  2. CU_FORMAT_YUV420_SEMIPLANAR: Y, UV in two surfaces (UV as one surface),
         width, height ratio same as case 1
  3. CU_FORMAT_YUV422_PLANAR: Y, U, V  each in a separate  surface,
         U/V width = 1/2 Y width, U/V height = Y height
  4. CU_FORMAT_YUV422_SEMIPLANAR: Y, UV in two surfaces,
         width, height ratio same as case 3
*/
typedef enum CUeglColorFormat_enum {
    CU_EGL_COLOR_FORMAT_YUV420_PLANAR       = 0x00,
    CU_EGL_COLOR_FORMAT_YUV420_SEMIPLANAR   = 0x01,
    CU_EGL_COLOR_FORMAT_YUV422_PLANAR       = 0x02,
    CU_EGL_COLOR_FORMAT_YUV422_SEMIPLANAR   = 0x03,
    CU_EGL_COLOR_FORMAT_RGB                 = 0x04,
    CU_EGL_COLOR_FORMAT_BGR                 = 0x05,
    CU_EGL_COLOR_FORMAT_ARGB                = 0x06,
    CU_EGL_COLOR_FORMAT_RGBA                = 0x07
} CUeglColorFormat;

/**
 * EGLFrame structure
 */
typedef struct CUeglFrame_st {
    union {
        CUarray pArray[MAX_PLANES];
        void*   pPitch[MAX_PLANES];
    } frame;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
    unsigned int pitch;
    unsigned int planeCount;                    // Number of planes
    unsigned int numChannels;                   // Number of channels
    CUeglFrameType frameType;                   // Array or Pitch
    CUeglColorFormat eglColorFormat;
    CUarray_format cuFormat;
} CUeglFrame;

/**
 * \brief Registers an EGL image
 *
 * Registers the EGLImageKHR specified by \p image for access by
 * CUDA. A handle to the registered object is returned as \p pCudaResource.
 * The surface's intended usage is specified using \p flags, as follows:
 *
 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE: Specifies no hints about how this
 *   resource will be used. It is therefore assumed that this resource will be
 *   read from and written to by CUDA. This is the default value.
 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_READ_ONLY: Specifies that CUDA
 *   will not write to this resource.
 * - ::CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITE_DISCARD: Specifies that
 *   CUDA will not read from this resource and will write over the
 *   entire contents of the resource, so none of the data previously
 *   stored in the resource will be preserved.
 *
 * The EGLImageKHR is an object which can be used to create EGLImage target resource. It is defined as a void pointer.
 * typedef void* EGLImageKHR
 *
 * \param pCudaResource   - Pointer to the returned object handle
 * \param image           - An EGLImageKHR image which can be used to create target resource.
 * \param flags           - Map flags
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_ALREADY_MAPPED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * \notefnerr
 *
 * \sa ::cuGraphicsEGLRegisterImage, ::cuGraphicsUnregisterResource,
 * ::cuGraphicsResourceSetMapFlags, ::cuGraphicsMapResources,
 * ::cuGraphicsUnmapResources
 */
CUresult CUDAAPI cuGraphicsEGLRegisterImage(CUgraphicsResource *pCudaResource, EGLImageKHR image, unsigned int flags);

/**
 * \brief Get an eglFrame through which to access a mapped graphics resource.
 *
 * Returns in \p *eglFrame an eglFrame pointer through which the mapped graphics resource
 * \p resource may be accessed.
 *
 * If \p resource is not mapped then ::CUDA_ERROR_NOT_MAPPED is returned.
 * *
 * \param eglFrame   - Returned eglFrame.
 * \param resource   - Mapped resource to access.
 * \param index      - Index for cubemap surfaces.
 * \param mipLevel   - Mipmap level for the subresource to access.
 *
 * \return
 * ::CUDA_SUCCESS,
 * ::CUDA_ERROR_DEINITIALIZED,
 * ::CUDA_ERROR_NOT_INITIALIZED,
 * ::CUDA_ERROR_INVALID_CONTEXT,
 * ::CUDA_ERROR_INVALID_VALUE,
 * ::CUDA_ERROR_INVALID_HANDLE,
 * ::CUDA_ERROR_NOT_MAPPED
 * \notefnerr
 *
 * \sa
 * ::cuGraphicsMapResources,
 * ::cuGraphicsSubResourceGetMappedArray,
 * ::cuGraphicsResourceGetMappedPointer
 */
CUresult CUDAAPI cuGraphicsResourceGetMappedEglFrame(CUeglFrame* eglFrame, CUgraphicsResource resource, unsigned int index, unsigned int mipLevel);

#endif

/** @} */ /* END CUDA_EGL */

#ifdef __cplusplus
};
#endif

#undef __CUDA_API_VERSION

#endif
