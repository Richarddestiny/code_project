/*
 * Copyright (c) 2007-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "nvmm_buffertype.h"
#include "nvddk_2d_v2.h"

#include "nv_buffer.h"
//#include "nv_buffer_pool.h"

typedef struct nv_buffer_impl {
    pthread_mutex_t mtx;
    pthread_mutexattr_t mtx_attr;
    unsigned ref_cnt;
    unsigned mapped;
    NvMMBuffer * nvmm;
    NvRmDeviceHandle rm;
} nv_buffer_impl_t;

static nv_buffer_t * nvmm_buffer_new(nv_buffer_param_t * param);
static int nvmm_buffer_del(nv_buffer_t ** pbuff);

static nv_buffer_param_t * nv_buffer_param_new(nv_buffer_param_t * param);
static void nv_buffer_param_del(nv_buffer_param_t ** pparam);

static nv_buffer_impl_t * nvmm_buffer_impl_new(nv_buffer_param_t * param);
static int nvmm_buffer_impl_del(nv_buffer_impl_t ** pimpl);

static nv_buffer_data_t * nvmm_buffer_data_new(nv_buffer_impl_t * impl);
static int nvmm_buffer_data_del(nv_buffer_data_t ** pdata, nv_buffer_impl_t * impl);

nv_buffer_t * nv_buffer_new(nv_buffer_param_t * param)
{
    if (param->type == NV_BUFFER_TYPE_NVMM) {
        return nvmm_buffer_new(param);
    }
    else {
        printf("%s %d: buffer type not supported\n", __func__, __LINE__);
        return NULL;
    }
}

int nv_buffer_del(nv_buffer_t ** pbuff)
{
    nv_buffer_t * buff;

    if (pbuff) {
        buff = *pbuff;
        if (buff) {
            if (buff->param) {
                if (buff->param->type == NV_BUFFER_TYPE_NVMM) {
                    nvmm_buffer_del(pbuff);
                }
                else {
                    printf("%s %d: buffer type not supported\n", __func__, __LINE__);
                }
            }
        }
        *pbuff = NULL;
    }

    return 1;
}

static nv_buffer_t * nvmm_buffer_new(nv_buffer_param_t * param)
{
    nv_buffer_t * buff = (nv_buffer_t *)malloc(sizeof(nv_buffer_t));

    buff->param = nv_buffer_param_new(param);
    if (buff->param == NULL)
        goto FAIL;

    buff->impl = nvmm_buffer_impl_new(buff->param);
    if (buff->impl == NULL)
        goto FAIL;

    buff->data = nvmm_buffer_data_new(buff->impl);
    if (buff->data == NULL)
        goto FAIL;

    return buff;
FAIL:
    nvmm_buffer_del(&buff);
    return NULL;
}

static int nvmm_buffer_del(nv_buffer_t ** pbuff)
{
    if (pbuff) {
        nv_buffer_t * buff = *pbuff;
        if (buff) {
            nvmm_buffer_data_del(&(buff->data), buff->impl);

            nvmm_buffer_impl_del(&(buff->impl));

            nv_buffer_param_del(&(buff->param));

            free(buff);
        }

        *pbuff = NULL;
    }
    return 1;
}

static nv_buffer_param_t * nv_buffer_param_new(nv_buffer_param_t * param)
{
    nv_buffer_param_t * _param = NULL;

    if (param) {
        _param = (nv_buffer_param_t *)malloc(sizeof(nv_buffer_param_t));
        *_param = *param;
    }

    return _param;
}

static void nv_buffer_param_del(nv_buffer_param_t ** pparam)
{
    nv_buffer_param_t * param;

    if (pparam) {
        param = *pparam;
        free(param);
        *pparam = NULL;
    }
}

static nv_buffer_impl_t * nvmm_buffer_impl_new(nv_buffer_param_t * param)
{
    NvError err;
    unsigned width;
    unsigned height;
    unsigned size;
    unsigned total_size;
    unsigned align;
    int i;
    unsigned attrs[] = { NvRmSurfaceAttribute_Layout, NvRmSurfaceLayout_Pitch,
                      NvRmSurfaceAttribute_DisplayScanFormat, NvDisplayScanFormat_Progressive,
                      NvRmSurfaceAttribute_None };
    NvColorFormat plane_format[3];
    NvYuvColorFormat color_format = NvYuvColorFormat_YUV420;
    NvRmSurface * surfs = NULL;
    nv_buffer_impl_t * impl = NULL;
    NvMMSurfaceDescriptor * desc;

    if (param) {
        width = param->req_width;
        height = param->req_height;

        impl = (nv_buffer_impl_t *)malloc(sizeof(nv_buffer_impl_t));
        memset(impl, 0, sizeof(nv_buffer_impl_t));

        err = NvRmOpenNew(&(impl->rm));
        if (err != NvSuccess) {
            impl->rm = NULL;
            goto FAIL;
        }
        impl->nvmm = (NvMMBuffer *)malloc(sizeof(NvMMBuffer));
        memset(impl->nvmm, 0, sizeof(NvMMBuffer));

        impl->nvmm->StructSize = sizeof(NvMMBuffer);
        impl->nvmm->PayloadType = NvMMPayloadType_SurfaceArray;
        //impl->nvmm->PayloadInfo.TimeStamp = 0;

        if (param->layout == NV_BUFFER_LAYOUT_BLOCK_LINEAR) {
            attrs[1] = NvRmSurfaceLayout_Blocklinear;
        }

        desc = &(impl->nvmm->Payload.Surfaces);

        if (param->format == NV_BUFFER_FORMAT_NV12) {
            plane_format[0] = NvColorFormat_Y8;
            plane_format[1] = NvColorFormat_U8_V8;
            desc->SurfaceCount = 2;
        }
        else if (param->format == NV_BUFFER_FORMAT_I420) {
            plane_format[0] = NvColorFormat_Y8;
            plane_format[1] = NvColorFormat_U8;
            plane_format[2] = NvColorFormat_V8;
            desc->SurfaceCount = 3;
        }
        else if (param->format == NV_BUFFER_FORMAT_UYVY){
            plane_format[0] = NvColorFormat_UYVY;
            desc->SurfaceCount = 1;
        }
        else {
            printf("%s %d -- unsupported format %d\n", __func__, __LINE__, param->format);
            goto FAIL;
        }
        surfs = desc->Surfaces;
        width = (width + 1) & ~1;
        height = (height + 1) & ~1;

        if (width < 32)
            width = 32;

        if(desc->SurfaceCount == 1){
            attrs[1] = NvRmSurfaceLayout_Pitch;
            NvRmSurfaceSetup(surfs, width, height, NvColorFormat_UYVY, attrs);
            size  = NvRmSurfaceComputeSize(&surfs[0]);
            align = NvRmSurfaceComputeAlignment(impl->rm, surfs);

            if(NvSuccess != NvRmMemHandleCreate(impl->rm, &surfs[0].hMem, size)){
                printf("NvRmMemHandleCreate error\n");
                goto FAIL;
            }
            if(NvSuccess != NvRmMemAlloc(surfs->hMem, NULL, 0, align, NvOsMemAttribute_Uncached))
            {
                printf("%s %d Error in nvmm_buffer_impl_new\n", __func__, __LINE__);
                goto FAIL;
            }

            // Else, success. Clear frame data
			//void* p = NULL;  //dummy here to cheat compiler
            //err = NvRmMemMap( surfs->hMem, surfs->Offset, size, NVOS_MEM_READ_WRITE, &p);
            //if (err == NvSuccess)
            //{
            //    NvRmMemUnmap( surfs[0].hMem, NULL, size);
            //}
        }
        else{
            total_size = 0;
            NvRmMultiplanarSurfaceSetup(surfs, desc->SurfaceCount,
                width, height, color_format, plane_format, attrs);
            for (i = 0; i < desc->SurfaceCount; i++)
            {
                if (param->align_type != NV_BUFFER_ALIGN_DEFAULT && surfs[i].Layout == NvRmSurfaceLayout_Pitch) {
                    surfs[i].Pitch = surfs[i].Width * NV_COLOR_GET_BPP(surfs[i].ColorFormat);
                    surfs[i].Pitch = (surfs[i].Pitch + 7) >> 3;
                    switch(param->align_type) {
                    case NV_BUFFER_ALIGN_BYTE:
                        printf("byte alignment\n");
                    default:
                        align = 1;
                    }
                    surfs[i].Pitch = ((surfs[i].Pitch + align - 1) / align) * align;
                }
                else {
                    align = NvRmSurfaceComputeAlignment(impl->rm, &surfs[i]);
                }

                size = NvRmSurfaceComputeSize(&surfs[i]);

                NvRmMemHandleCreate(impl->rm, &(surfs[i].hMem), size);

                if (err != NvSuccess)
                {
                    printf("%s %d Error in create mem handle\n", __func__, __LINE__);
                    goto FAIL;
                }

                NvRmMemAllocBlocklinear(surfs[i].hMem, NULL, 0,
                    align, NvOsMemAttribute_WriteBack, surfs[i].Kind,
                    NvRmMemCompressionTags_None);

                if (err != NvSuccess)
                {
                    printf("%s %d Error in memory allocation\n", __func__, __LINE__);
                    goto FAIL;
                }
                printf("---- component size %d alignment %d %dx%d pitch %d\n", size,
                    align, surfs[i].Width,
                    surfs[i].Height, surfs[i].Pitch);

                total_size += size;
            }
        }
    }
    return impl;

FAIL:
    nvmm_buffer_impl_del(&impl);
    return impl;
}

static int nvmm_buffer_impl_del(nv_buffer_impl_t ** pimpl)
{
    int i;
    NvRmSurface * surfs;
    nv_buffer_impl_t * impl;

    if (pimpl) {
        impl = *pimpl;
        if (impl) {
            if (impl->nvmm) {
                surfs = impl->nvmm->Payload.Surfaces.Surfaces;
                for (i = 0; i < impl->nvmm->Payload.Surfaces.SurfaceCount; ++i) {
                    if (surfs[i].hMem) {
                        NvRmMemHandleFree(surfs[i].hMem);
                        surfs[i].hMem = NULL;
                    }
                }
                free(impl->nvmm);
                impl->nvmm = NULL;
            }
            if (impl->rm) {
                NvRmClose(impl->rm);
                //free(impl->rm);
                impl->rm = NULL;
            }
            free(impl);
            *pimpl = NULL;
        }
    }

    return 1;
}

static nv_buffer_data_t * nvmm_buffer_data_new(nv_buffer_impl_t * impl)
{
    unsigned size;
    int i;
    nv_buffer_data_t * data = NULL;
    NvMMSurfaceDescriptor * desc;
    NvRmSurface * surfs;

    if (impl) {
        desc = &(impl->nvmm->Payload.Surfaces);
        surfs = desc->Surfaces;
        
        data = (nv_buffer_data_t *)malloc(sizeof(nv_buffer_data_t));
        memset(data, 0, sizeof(nv_buffer_data_t));

        for (i = 0; i < desc->SurfaceCount; i++)
        {
            size = NvRmSurfaceComputeSize(&surfs[i]);

            // map buffer
            data->size[i] = size;
            data->width[i] = surfs[i].Width;
            data->height[i] = surfs[i].Height;
            data->pitch[i] = surfs[i].Pitch;

            NvError err = NvRmMemMap(surfs[i].hMem,
                                     surfs[i].Offset,
                                     size,
                                     NVOS_MEM_READ_WRITE | NVOS_MEM_MAP_WRITEBACK,
                                     &(data->mem[i]));
            if (err == NvSuccess)
            {
                NvRmMemCacheSyncForCpu(surfs[i].hMem, data->mem[i], size);
            }
            else
            {
                data->mem[i] = NULL;
                goto FAIL;
            }

            switch (surfs[i].ColorFormat) {
            case NvColorFormat_Y8:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_Y8;
                break;
            case NvColorFormat_U8_V8:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_U8V8;
                break;
            case NvColorFormat_U8:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_U8;
                break;
            case NvColorFormat_V8:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_V8;
                break;
            case NvColorFormat_UYVY:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_UYVY;
                break;
            default:
                data->color_format[i] = NV_BUFFER_COLOR_FORMAT_NONE;
                break;
            }
        }

        data->num_surf = desc->SurfaceCount;
    }

    return data;

FAIL:
    nvmm_buffer_data_del(&data, impl);
    return data;
}

static int nvmm_buffer_data_del(nv_buffer_data_t ** pdata, nv_buffer_impl_t * impl)
{
    unsigned i;
    nv_buffer_data_t * data;
    NvMMSurfaceDescriptor * desc;
    NvRmSurface * surfs;

    if (pdata) {
        if (*pdata) {
            data = *pdata;
            if (data) {
                if (impl && impl->rm && impl->nvmm) {
                    desc = &(impl->nvmm->Payload.Surfaces);
                    surfs = desc->Surfaces;
                    for (i = 0; i < data->num_surf; ++i) {
                        if (surfs[i].hMem && data->mem[i]) {
                            NvRmMemUnmap(surfs[i].hMem, data->mem[i], data->size[i]);
                        }
                        else {
                            printf("alloc hmem %p mapped %p\n", surfs[i].hMem, data->mem[i]);
                        }
                    }
                }
                else {
                    printf("%s %d: wiered to be here!\n", __func__, __LINE__);
                }
                free(data);
                *pdata = NULL;
            }
        }
    }

    return 1;
}

struct NvRmSurfaceRec * nv_buffer_get_surfaces(nv_buffer_t * buff)
{
    if (buff->impl && buff->impl->nvmm) {
        return &(buff->impl->nvmm->Payload.Surfaces.Surfaces[0]);
    }

    return NULL;
}

struct NvMMBufferRec * nv_buffer_get_nvmm(nv_buffer_t * buff)
{
    if (buff->impl && buff->impl->nvmm) {
        return buff->impl->nvmm;
    }

    return NULL;
}

int nv_buffer_convert(nv_buffer_t * my, nv_buffer_t * other)
{
    int result = 1;
    NvError err = NvSuccess;
    NvDdk2dHandle ddk2d;
    NvRmDeviceHandle rm = my->impl->rm;
    err = NvDdk2dOpen(rm, NULL, &(ddk2d));
    if (NvSuccess != err) {
        printf("failed to open 2D to do buffer convert\n");
    }

    NvRect dst_rect;
    NvDdk2dFixedRect src_rect;
    NvDdk2dSurface * ddk2d_src = NULL;
    NvDdk2dSurface * ddk2d_dst = NULL;
    NvDdk2dSurfaceType Type1;
    NvDdk2dSurfaceType Type2;

    dst_rect.left = 0;
    dst_rect.right = my->param->req_width;
    dst_rect.top = 0;
    dst_rect.bottom = my->param->req_height;
    src_rect.left = NV_SFX_ZERO;
    src_rect.right = NV_SFX_ADD (src_rect.left,
        NV_SFX_WHOLE_TO_FX (other->param->req_width));
    src_rect.top = NV_SFX_ZERO;
    src_rect.bottom = NV_SFX_ADD (src_rect.top,
        NV_SFX_WHOLE_TO_FX (other->param->req_height));

    //NvRmSurface * src_surfs = nv_buffer_get_surfaces(other);
    NvMMSurfaceDescriptor * p_src_desc = &(other->impl->nvmm->Payload.Surfaces);
    //NvRmSurface * dst_surfs = nv_buffer_get_surfaces(my);
    NvMMSurfaceDescriptor * p_dst_desc = &(my->impl->nvmm->Payload.Surfaces);
    if ((p_src_desc->Surfaces[1].ColorFormat == NvColorFormat_U8_V8)
         || (p_src_desc->Surfaces[1].ColorFormat == NvColorFormat_V8_U8))
        Type1 = NvDdk2dSurfaceType_Y_UV;
    else
        Type1 = NvDdk2dSurfaceType_Y_U_V;

    if ((p_dst_desc->Surfaces[1].ColorFormat == NvColorFormat_U8_V8)
         || (p_dst_desc->Surfaces[1].ColorFormat == NvColorFormat_V8_U8))
        Type2 = NvDdk2dSurfaceType_Y_UV;
    else
        Type2 = NvDdk2dSurfaceType_Y_U_V;

    //pthread_mutex_lock(&(hw->mtx));
    err = NvDdk2dSurfaceCreate(ddk2d, Type1,
                               &(p_src_desc->Surfaces[0]), &ddk2d_src);
    if (NvSuccess != err)
    {
        result = 0;
        goto FAIL;
    }
    err = NvDdk2dSurfaceCreate(ddk2d, Type2,
                               &(p_dst_desc->Surfaces[0]), &ddk2d_dst);
    if (NvSuccess != err)
    {
        result = 0;
        goto FAIL;
    }

    err = NvDdk2dBlit(ddk2d, ddk2d_dst, NULL,
                      ddk2d_src, NULL, NULL);
    if (NvSuccess != err)
    {
        result = 0;
        goto FAIL;
    }
    // Lock unlock to make sure 2D is done
    NvDdk2dSurfaceLock(ddk2d_dst,
                       NvDdk2dSurfaceAccessMode_ReadWrite,
                       NULL,
                       NULL,
                       NULL);
    NvDdk2dSurfaceUnlock(ddk2d_dst, NULL, 0);

    //pthread_mutex_unlock(&(hw->mtx));

FAIL:
    if (ddk2d_src)
        NvDdk2dSurfaceDestroy(ddk2d_src);
    if (ddk2d_dst)
        NvDdk2dSurfaceDestroy(ddk2d_dst);

    //pthread_mutex_unlock(&(hw->mtx));

    NvDdk2dClose(ddk2d);
    return result;
}

int nv_buffer_data_fill(nv_buffer_t * buff, nv_buffer_data_t * data)
{
    int rslt = 0;
    unsigned width;
    unsigned height;
    NvMMSurfaceDescriptor * desc = &(buff->impl->nvmm->Payload.Surfaces);
    NvRmSurface * surfs = desc->Surfaces;

    if (buff->param->type == NV_BUFFER_TYPE_NVMM) {
        if (buff->param->format == NV_BUFFER_FORMAT_NV12) {
            switch (data->num_surf) {
            case 2:
                if (data->color_format[0] != NV_BUFFER_COLOR_FORMAT_Y8 || data->color_format[1] != NV_BUFFER_COLOR_FORMAT_U8V8) {
                    printf("unsupport input data color format\n");
                    goto FAIL;
                }

                width = data->width[0] > surfs[0].Width ? surfs[0].Width : data->width[0];
                height = data->height[0] > surfs[0].Height ? surfs[0].Height : data->height[0];
                printf("surface 0: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[0]), 0, 0, width, height, data->mem[0]);
                width = data->width[1] > surfs[1].Width ? surfs[1].Width : data->width[1];
                height = data->height[1] > surfs[1].Height ? surfs[1].Height : data->height[1];
                printf("surface 1: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[1]), 0, 0, width, height, data->mem[1]);
                break;
            default:
                printf("number of input data plane not support\n");
                goto FAIL;
            }
        }
        else if (buff->param->format == NV_BUFFER_FORMAT_I420) {
            switch (data->num_surf) {
            case 3:
                if (data->color_format[0] != NV_BUFFER_COLOR_FORMAT_Y8 ||
                    data->color_format[1] != NV_BUFFER_COLOR_FORMAT_U8 ||
                    data->color_format[2] != NV_BUFFER_COLOR_FORMAT_V8) {
                    printf("unsupport input data color format\n");
                    goto FAIL;
                }

                width = data->width[0] > surfs[0].Width ? surfs[0].Width : data->width[0];
                height = data->height[0] > surfs[0].Height ? surfs[0].Height : data->height[0];
                printf("surface 0: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[0]), 0, 0, width, height, data->mem[0]);
                width = data->width[1] > surfs[1].Width ? surfs[1].Width : data->width[1];
                height = data->height[1] > surfs[1].Height ? surfs[1].Height : data->height[1];
                printf("surface 1: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[1]), 0, 0, width, height, data->mem[1]);
                width = data->width[2] > surfs[2].Width ? surfs[2].Width : data->width[2];
                height = data->height[2] > surfs[2].Height ? surfs[2].Height : data->height[2];
                printf("surface 2: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[2]), 0, 0, width, height, data->mem[2]);
                break;
            default:
                printf("number of input data plane not support\n");
                goto FAIL;
            }
        }
        else if( buff->param->format == NV_BUFFER_FORMAT_UYVY ){
            switch (data->num_surf) {
            case 1:
                if (data->color_format[0] != NV_BUFFER_COLOR_FORMAT_UYVY ) {
                    printf("unsupport input data color format\n");
                    goto FAIL;
                }
                width = data->width[0] > surfs[0].Width ? surfs[0].Width : data->width[0];
                height = data->height[0] > surfs[0].Height ? surfs[0].Height : data->height[0];
                printf("surface 0: %dx%d\n", width, height);
                NvRmSurfaceWrite(&(surfs[0]), 0, 0, width, height, data->mem[0]);
                break;
            default:
                printf("number of input data plane not support\n");
                goto FAIL;
            }
        }
        else {
            printf("unsupported buffer format %d\n", buff->param->format);
        }
    }
    else {
        printf("unsupported buffer type %d\n", buff->param->type);
    }

    return rslt;
FAIL:
    rslt = 0;
    return rslt;
}

#if 0
int nv_buffer_set_with_nvmm(nv_buffer_t * buff, struct NvMMBufferRec * nvmm)
{
    if (nvmm->PayloadType != NvMMPayloadType_SurfaceArray)
        return 0;

    if (buff->rm)
        return 0;

    buff->rm = NULL;
    buff->type = NV_BUFFER_TYPE_NVMM;
    if (nvmm->Payload.Surfaces.Surfaces[1].ColorFormat == NvColorFormat_U8_V8) {
        buff->format = NV_BUFFER_FORMAT_NV12;
        buff->num_surf = 2;
        buff->color_format[0] = NV_BUFFER_COLOR_FORMAT_Y8;
        buff->color_format[1] = NV_BUFFER_COLOR_FORMAT_U8V8;
    }
    else if (nvmm->Payload.Surfaces.Surfaces[1].ColorFormat == NvColorFormat_U8) {
        buff->format = NV_BUFFER_FORMAT_I420;
        buff->num_surf = 3;
        buff->color_format[0] = NV_BUFFER_COLOR_FORMAT_Y8;
        buff->color_format[1] = NV_BUFFER_COLOR_FORMAT_U8;
        buff->color_format[2] = NV_BUFFER_COLOR_FORMAT_V8;
    }
    else {
        printf("%s %d: unknown format\n", __func__, __LINE__);
        return 0;
    }

    if (nvmm->Payload.Surfaces.Surfaces[0].Layout == NvRmSurfaceLayout_Blocklinear)
    {
        buff->layout = NV_BUFFER_LAYOUT_BLOCK_LINEAR;
    }
    else if (nvmm->Payload.Surfaces.Surfaces[0].Layout == NvRmSurfaceLayout_Pitch)
    {
        buff->layout = NV_BUFFER_LAYOUT_PITCH_LINEAR;
    }
    else {
        printf("%s %d: unknown layout\n", __func__, __LINE__);
        return 0;
    }
    buff->req_width = nvmm->Payload.Surfaces.Surfaces[0].Width;
    buff->req_height = nvmm->Payload.Surfaces.Surfaces[0].Height;

    buff->nvmm = (NvMMBuffer *) malloc(sizeof(NvMMBuffer));
    memcpy(buff->nvmm, nvmm, sizeof(NvMMBuffer));
    nv_buffer_map(buff);
}
#endif

