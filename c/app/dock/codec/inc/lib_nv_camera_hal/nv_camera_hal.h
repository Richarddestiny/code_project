/*
 *  Copyright (c) 2009-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software and related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */

#ifndef INCLUDED_CAMERA_CFVC_H
#define INCLUDED_CAMERA_CFVC_H

#include "nvmm.h"
#include "nvcam_properties_public.h"

#if defined(__cplusplus)
extern "C"
{
#endif

/* Parameters for cfvc client */
struct CfvcParam
{
    NvOsSemaphoreHandle hReceivedFrameSema;

    // Output buffer requirements.
    NvU32  SurfaceLayout;
    NvSize Resolution;
    NvU32  SensorId;

    NvU32 BufferPoolSize;

    NvU32 ErrorCount;

    void *pPriv;
};

struct CfvcFrame {
    NvMMBuffer *pSurf;
    NvU32 FrameNo;
    struct CfvcParam *pParams;
};

/** CfvcHandle is an opaque handle to the Cfvc interface  */
typedef struct CfvcRec *CfvcHandle;

NvError OpenCamera(CfvcHandle *phCfvc, struct CfvcParam *pParam);
void CloseCamera(CfvcHandle hCfvc);

NvError StartCamera(CfvcHandle hCfvc);
void StopCamera(CfvcHandle hCfvc);
NvError PeekFrame(CfvcHandle hCfvc, struct CfvcFrame *pFrame);
NvError GetFrame(CfvcHandle hCfvc, struct CfvcFrame *pFrame);
void ReleaseFrame(CfvcHandle hCfvc, struct CfvcFrame*);
NvError CfvcDoTNR(CfvcHandle hCfvc, struct CfvcFrame*);
void SetAWB(CfvcHandle hCfvc, NvCamAwbMode);
void SetAeMode(CfvcHandle hCfvc, NvCamAeMode);
void SetExposureTime(CfvcHandle hCfvc, NvF32);
void SetISO(CfvcHandle hCfvc, NvU32);

#if defined(__cplusplus)
}
#endif

#endif //INCLUDED_CAMERA_CFVC_H
