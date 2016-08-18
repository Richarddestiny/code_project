/*
 * Copyright (c) 2011-2012 NVIDIA Corporation.  All rights reserved.
 *
 * NVIDIA Corporation and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA Corporation is strictly prohibited.
 */
#ifndef _NV_OMX_VIDEO_H_
#define _NV_OMX_VIDEO_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>
#include "nvos.h"
#include <OMX_Index.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OMX_Video.h>
#include <OMX_Audio.h>
#include <OMX_Other.h>
#include <OMX_Image.h>
#include <NVOMX_IndexExtensions.h>


#define MAX_INPUT_BUFFERS 100
#define MAX_OUTPUT_BUFFERS 100
#define READ_INPUT_BUFFER_SIZE 6*1024
#define MAX_FILE_NAME_LEN 512
#define MAX_BITSTREAM_SIZE 600*1024

#define NOTSET_U8 ((OMX_U8)0xDE)
#define NOTSET_U16 ((OMX_U16)0xDEDE)
#define NOTSET_U32 ((OMX_U32)0xDEDEDEDE)
#define INIT_PARAM(_X_)  (memset(&(_X_), NOTSET_U8, sizeof(_X_)), ((_X_).nSize = sizeof (_X_)), (_X_).nVersion = vOMX)

#define MAX_PLAYWINDOW_WIDTH 1280
#define MAX_PLAYWINDOW_HEIGHT 720


#include "util.h"


extern OMX_VERSIONTYPE vOMX;

enum{
    NAL_UNIT_START_CODE = 0x00000001,
    GOV_START_CODE = 0x000001B3,
    VOP_START_CODE = 0x000001B6,
    SVH_START_CODE = 0x20,
    MPEG2_SEQUENCE_HEADER_CODE = 0x000001B3,
    MPEG2_PICTURE_START_CODE = 0x00000100,
    NV_VC1_START_CODE_PREFIX = 0x000001,
    NV_VC1_SLICE_START_CODE  = 0x0000010B,
    NV_VC1_ENTRYPOINT_START_CODE = 0x0000010E,
    NV_VC1_USER_LEVEL_START_CODE_START = 0x0000011B,
    NV_VC1_USER_LEVEL_START_CODE_END   = 0x0000011F,
    NVMM_VC1_SIMPLE_MAIN_PROF_SEQ_LAYER_NUM_BYTES = 32
};

typedef enum codec_type{
    INVALID = -1,
    H264
} codec_type;

typedef struct codecs
{
    codec_type type;
    const char * name;
} codecs;

typedef struct nv_omx_video
{
    sem_t sem_state_change;
    sem_t sem_empty_buffer_done;
    sem_t sem_fill_buffer_done;
    sem_t sem_eos;
    thread_info read_thread;
    thread_info write_thread;

    FILE * f_input;
    FILE * f_output;
    char name_input[MAX_FILE_NAME_LEN];
    char name_output[MAX_FILE_NAME_LEN];

    OMX_U32 nPortIndex;
    OMX_CALLBACKTYPE oCallbacks;
    OMX_HANDLETYPE component;
    OMX_PARAM_PORTDEFINITIONTYPE inputPortDef;
    OMX_PARAM_PORTDEFINITIONTYPE outputPortDef;
    OMX_BUFFERHEADERTYPE    *inputBuffer[MAX_INPUT_BUFFERS];
    OMX_BUFFERHEADERTYPE    *outputBuffer[MAX_OUTPUT_BUFFERS];
    unsigned max_bitstream_size;
    unsigned num_input_buffers;
    unsigned num_output_buffers;
    unsigned input_index;
    unsigned output_index;
    unsigned width;
    unsigned height;
    unsigned c_width;
    unsigned c_height;
    unsigned fps;
    unsigned bitrate;
    unsigned cal_fps;
    unsigned golden_fps;
    unsigned output_size;
    unsigned time_to_decode;
    int is_eos;
    int is_enc;
    int is_dec;
    int have_header_info;
    int use_nvmm;
    int disabledpb;
    NvU32 start;
    NvU32 end;
    codec_type codec;
    struct nv_buffer ** input_nvmm_buf;
} nv_omx_codec_t;

#if 0
long get_time_ms(void);


void * read_thread(void *self);

void * write_thread(void * self);


void alloc_nvmm_buf(nv_omx_codec_t * nv_codec);

void nv_omx_codec_init(nv_omx_codec_t * nv_codec);

void nv_omx_codec_deinit(nv_omx_codec_t * nv_codec);

void set_state(nv_omx_codec_t * nv_codec, OMX_HANDLETYPE handle, OMX_STATETYPE state, OMX_BOOL wait);

// Event Callback
OMX_ERRORTYPE
EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData);

// Empty Buffer Callback
OMX_ERRORTYPE EmptyBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

// Fill Buffer call
OMX_ERRORTYPE FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

#endif

#endif  // _NV_OMX_VIDEO_H_

