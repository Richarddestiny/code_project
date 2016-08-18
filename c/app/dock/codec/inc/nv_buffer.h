/*
 * Copyright (c) 2007-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef _NV_BUFFER_H_
#define _NV_BUFFER_H_

typedef enum {
    NV_BUFFER_TYPE_MEM = 0,
    NV_BUFFER_TYPE_NVMM,
    NV_BUFFER_TYPE_EGL,
    NV_BUFFER_TYPE_NONE
} NVBufferType;

typedef enum {
    NV_BUFFER_LAYOUT_PITCH_LINEAR = 0,
    NV_BUFFER_LAYOUT_BLOCK_LINEAR,
    NV_BUFFER_LAYOUT_NONE
} NVBufferLayout;

typedef enum {
    NV_BUFFER_FORMAT_NV12 = 0,
    NV_BUFFER_FORMAT_I420,
    NV_BUFFER_FORMAT_UYVY,
    NV_BUFFER_FORMAT_NONE
} NVBufferFormat;

typedef enum {
    NV_BUFFER_COLOR_FORMAT_Y8 = 0,
    NV_BUFFER_COLOR_FORMAT_U8V8,
    NV_BUFFER_COLOR_FORMAT_U8,
    NV_BUFFER_COLOR_FORMAT_V8,
    NV_BUFFER_COLOR_FORMAT_UYVY,
    NV_BUFFER_COLOR_FORMAT_NONE
} NVBufferColorFormat;

typedef enum {
    NV_BUFFER_ALIGN_DEFAULT = 0,
    NV_BUFFER_ALIGN_BYTE,
    NV_BUFFER_ALIGN_NONE
} NVBufferAlign;

#define MAX_SURFACES (3)

typedef struct nv_buffer_param {
    NVBufferType type;
    NVBufferFormat format;
    NVBufferLayout layout;
    NVBufferAlign align_type;
    unsigned req_width;
    unsigned req_height;
} nv_buffer_param_t;

typedef struct nv_buffer_data {
    unsigned num_surf;
    void * mem[MAX_SURFACES];
    unsigned width[MAX_SURFACES];
    unsigned height[MAX_SURFACES];
    unsigned pitch[MAX_SURFACES];
    unsigned size[MAX_SURFACES];
    NVBufferColorFormat color_format[MAX_SURFACES];
} nv_buffer_data_t;

struct nv_buffer_impl;

typedef struct nv_buffer{
    unsigned id;
    nv_buffer_param_t * param;
    nv_buffer_data_t * data;
    struct nv_buffer_impl * impl;
} nv_buffer_t;

typedef nv_buffer_t * nv_buffer_p;

nv_buffer_t * nv_buffer_new(nv_buffer_param_t * param);

int nv_buffer_del(nv_buffer_t ** pbuff);

int nv_buffer_data_fill(nv_buffer_t * buff, nv_buffer_data_t * data);

#if 0
int nv_buffer_set_with_nvmm(nv_buffer_t * buff, struct NvMMBufferRec * nvmm);
#endif

struct NvRmSurfaceRec * nv_buffer_get_surfaces(nv_buffer_t * buff);
struct NvMMBufferRec * nv_buffer_get_nvmm(nv_buffer_t * buff);
int nv_buffer_convert(nv_buffer_t * my, nv_buffer_t * other);

#endif//_NV_BUFFER_H_

