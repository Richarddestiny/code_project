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
#include <stdbool.h>
#include "nvos.h"

#include <dlfcn.h>
#include "nv_buffer.h"


#include "nvmm_buffertype.h"
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

#include "util.h"
#include "../include/decode.h"

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

	/* add by jiangxiaohui  */
	unsigned char *pInput;
	unsigned char *pOutput;
	unsigned nInputSize;
	unsigned *pOututSize;


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
    int have_header_info;
    int use_nvmm;
    int disabledpb;
    NvU32 start;
    NvU32 end;
    codec_type codec;
    struct NvMMBuffer** input_nvmm_buf;
} nv_omx_codec_t;

static long get_time_ms(void);


static void * read_thread(void *self);

static void * write_thread(void * self);


void alloc_nvmm_buf(nv_omx_codec_t * nv_codec);

void nv_omx_codec_init(nv_omx_codec_t * nv_codec);

void nv_omx_codec_deinit(nv_omx_codec_t * nv_codec);

void set_state(nv_omx_codec_t * nv_codec, OMX_HANDLETYPE handle, OMX_STATETYPE state, OMX_BOOL wait);

// Event Callback
static OMX_ERRORTYPE
EventHandler(
    OMX_IN OMX_HANDLETYPE hComponent,
    OMX_IN OMX_PTR pAppData,
    OMX_IN OMX_EVENTTYPE eEvent,
    OMX_IN OMX_U32 nData1,
    OMX_IN OMX_U32 nData2,
    OMX_IN OMX_PTR pEventData);

// Empty Buffer Callback
static OMX_ERRORTYPE EmptyBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);

// Fill Buffer call
static OMX_ERRORTYPE FillBufferDone(
        OMX_OUT OMX_HANDLETYPE hComponent,
        OMX_OUT OMX_PTR pAppData,
        OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer);



//#include "memorymgr.h"

OMX_VERSIONTYPE vOMX;
//NvBool bDisableOuputPort = NV_FALSE;

static void port_settings_changed(nv_omx_codec_t * test, OMX_U32 nPortIndex);
static int bfirst = 1;
static long get_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

// Event Callback
static OMX_ERRORTYPE
EventHandler(
		OMX_IN OMX_HANDLETYPE hComponent,
		OMX_IN OMX_PTR pAppData,
		OMX_IN OMX_EVENTTYPE eEvent,
		OMX_IN OMX_U32 nData1,
		OMX_IN OMX_U32 nData2,
		OMX_IN OMX_PTR pEventData)
{
	nv_omx_codec_t * nv_codec = (nv_omx_codec_t *)pAppData;
	OMX_U32 nPortIndex;
	OMX_ERRORTYPE result;
	int i =0;
	PRINT("%s ++\n", __func__);

	switch (eEvent)
	{
		case OMX_EventBufferFlag:
			nv_codec->is_eos = 1;
			nv_codec->end = NvOsGetTimeMS();
			PRINT("encoder, at%u takes about:%u\n", nv_codec->end, nv_codec->end - nv_codec->start);

			sem_post(&(nv_codec->sem_fill_buffer_done));
			sem_post(&(nv_codec->sem_eos));

			break;

		case OMX_EventPortSettingsChanged:
			PRINT("Got OMX_EventPortSettingsChanged event ++ data1:%lu,\n", nData1);
			{
				nPortIndex = nData1;
				PRINT("Got OMX_EventPortSettingsChanged event %d %d\n",
						(int)nPortIndex,
						(int)nData2);

				OMX_PARAM_PORTDEFINITIONTYPE oPortDef;
				INIT_PARAM(oPortDef);
				oPortDef.nPortIndex = nPortIndex;
				OMX_GetParameter(
						nv_codec->component,
						OMX_IndexParamPortDefinition,
						&oPortDef);

				nv_codec->c_width = oPortDef.format.video.nFrameWidth;
				nv_codec->c_height = oPortDef.format.video.nFrameHeight;

				PRINT("w= %d,h= %d\n",
						(int)oPortDef.format.video.nFrameWidth,
						(int)oPortDef.format.video.nFrameHeight);

			}
			PRINT("Got OMX_EventPortSettingsChanged event --\n");

			break;

		case OMX_EventCmdComplete:
			nPortIndex = nData2;
			if ((nData1 == OMX_CommandStateSet)||(nData1 == OMX_CommandFlush))
			{
				sem_post(&(nv_codec->sem_state_change));
			}
			break;

		default:
			PRINT("Got %d event\n", eEvent);
	}

	PRINT("%s --\n", __func__);

	return OMX_ErrorNone;
}

// Empty Buffer Callback
static OMX_ERRORTYPE EmptyBufferDone(
		OMX_OUT OMX_HANDLETYPE hComponent,
		OMX_OUT OMX_PTR pAppData,
		OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
	nv_omx_codec_t *omx_video = (nv_omx_codec_t *)pAppData;

	PRINT("%s:	HDR: 0x%x pBuffer:0x%x \n", __func__,
			(unsigned int)pBuffer,
			(unsigned int)pBuffer->pBuffer);
		sem_post(&(omx_video->sem_empty_buffer_done));

	return OMX_ErrorNone;
}

// Fill Buffer call
static OMX_ERRORTYPE FillBufferDone(
		OMX_OUT OMX_HANDLETYPE hComponent,
		OMX_OUT OMX_PTR pAppData,
		OMX_OUT OMX_BUFFERHEADERTYPE* pBuffer)
{
	nv_omx_codec_t * nv_codec = (nv_omx_codec_t *)pAppData;
	PRINT("%s: HDR: 0x%x pBuffer:0x%x  len: %ld\n", __func__,
			(unsigned int)pBuffer,
			(unsigned int)pBuffer->pBuffer,
			pBuffer->nFilledLen);

	if (!pBuffer->nFilledLen)
		return OMX_ErrorNone;

	sem_post(&(nv_codec->sem_fill_buffer_done));

	return OMX_ErrorNone;
}

	void
set_state(
		nv_omx_codec_t * nv_codec,
		OMX_HANDLETYPE handle,
		OMX_STATETYPE state,
		OMX_BOOL wait)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	PRINT("%s: ++\n", __func__);

	eError = OMX_SendCommand(handle, OMX_CommandStateSet, state, 0 );

	if (eError != OMX_ErrorNone)
	{
		PRINT("SendCommand with state=%d failed\n", state);
	}

	if (wait)
		sem_wait(&nv_codec->sem_state_change);

	PRINT("%s: --\n", __func__);
}

	static NvMMBuffer* createbuffer(  int m_width, int m_height )
	{
		NvRmDeviceHandle hRm = NULL;



		NvError e;
		int size = 0;
		int numPlanes = 3;

		NvError err;
		unsigned total_size;
		unsigned align;
		int i;

		NvRmSurface * surf = NULL;
		NvMMSurfaceDescriptor * desc;

		NvMMBuffer *nvmm = (NvMMBuffer *)malloc(sizeof(NvMMBuffer));
		memset(nvmm, 0, sizeof(NvMMBuffer));

		nvmm->StructSize = sizeof(NvMMBuffer);
		nvmm->PayloadType = NvMMPayloadType_SurfaceArray;


		desc = &(nvmm->Payload.Surfaces);
		surf = &desc->Surfaces[0];
		desc->SurfaceCount = 3;

		NvColorFormat	nvColorFormat[] =
		{
			NvColorFormat_Y8,
			NvColorFormat_U8,
			NvColorFormat_V8
		};
		NvU32 surfAttrs[] = {
			NvRmSurfaceAttribute_Layout, NvRmSurfaceLayout_Pitch,
			NvRmSurfaceAttribute_None
		};


		NvRmHeap Heaps[] =
		{
			NvRmHeap_ExternalCarveOut,
			NvRmHeap_External
		};
		e = NvRmOpen(&hRm, 0);
		if (e != NvSuccess) {
			return -1;
		}


		for (i = 0; i < numPlanes; i++) {
			int width = (i == 0) ? m_width : m_width / 2;
			int height = (i == 0) ? m_height : m_height / 2;


			NvRmSurfaceSetup(&surf[i], width, height, nvColorFormat[i], surfAttrs);
			NvRmSurfaceComputePitch(NULL,0,&surf[i]);
			int planeAlign = NvRmSurfaceComputeAlignment(hRm, &surf[i]);

			size = NvRmSurfaceComputeSize(&surf[i]);
			NV_CHECK_ERROR_CLEANUP(
				NvRmMemHandleAlloc(hRm, Heaps,
				NV_ARRAY_SIZE(Heaps), planeAlign , NvOsMemAttribute_WriteCombined,
				size, 0, 0, &surf[i].hMem));
		}
		NvRmClose(hRm);
		return nvmm;

	fail:
		NvRmClose(hRm);
		return -1;

	}

	static int destroyBuffer( NvMMBuffer *nvmm )
	{
		NvRmSurface * surf = &(nvmm->Payload.Surfaces.Surfaces[0]);
		int numPlanes = 3;
		int i;
		for( i = 0 ; i < numPlanes; i ++ )
		{
			NvRmMemHandleFree(surf[i].hMem);
		}
		return 0;
	}


void alloc_nvmm_buf(nv_omx_codec_t * nv_codec)
{
	unsigned i;
	nv_buffer_param_t param;
	PRINT("%s: ++\n", __func__);

	if (nv_codec->use_nvmm && !nv_codec->input_nvmm_buf)
	{
		param.type = NV_BUFFER_TYPE_NVMM;
		param.format = NV_BUFFER_FORMAT_NV12;
		param.layout = NV_BUFFER_LAYOUT_PITCH_LINEAR;
		param.align_type = NV_BUFFER_ALIGN_DEFAULT;
		param.req_width = nv_codec->width;
		param.req_height = nv_codec->height;

		nv_codec->input_nvmm_buf = (NvMMBuffer **)malloc(sizeof(NvMMBuffer *) * nv_codec->num_input_buffers);
		for (i = 0; i < nv_codec->num_input_buffers; ++i) {
			nv_codec->input_nvmm_buf[i] = createbuffer( nv_codec->width,nv_codec->height );
			PRINT("---- allocate %d %p\n", i, nv_codec->input_nvmm_buf[i]);
		}
	}
	PRINT("%s: --\n", __func__);
}

static void release_nvmm_buf(nv_omx_codec_t * nv_codec)
{
	unsigned i;

	if (!nv_codec->input_nvmm_buf) return;

	for (i = 0; i < nv_codec->num_input_buffers; ++i) {
		destroyBuffer(&(nv_codec->input_nvmm_buf[i]));
	}
	free(nv_codec->input_nvmm_buf);
}

	static unsigned
read_nvmm_from_yuv(
		nv_omx_codec_t * nv_codec)
{
	unsigned read_size = 0;
	unsigned luma_size = nv_codec->height * nv_codec->width;
	unsigned chroma_size = luma_size / 4;
	unsigned total_read = 0;

	unsigned char * pbuff = (unsigned char *)malloc(sizeof(unsigned char) * luma_size *3/2);
	NvMMBuffer* pNvMM = nv_codec->input_nvmm_buf[nv_codec->input_index];
	//PRINT("%p %u %p\n", nv_codec->input_nvmm_buf, nv_codec->InputIndex, pnvbuff);


	PRINT("%s: read nvmm[%u] from NV12: + size %d\n", __func__, nv_codec->input_index, luma_size + chroma_size);

	//read_size = fread(pbuff, 1, luma_size*3/2, nv_codec->f_input);

	//add by jiangxiaohui
	read_size = nv_codec->nInputSize;
	memcpy(pbuff, nv_codec->pInput, read_size);

	if (read_size == luma_size*3/2) {
		//memset(pbuff, 128, read_size);
		NvRmSurfaceWrite(&pNvMM->Payload.Surfaces.Surfaces[0], 0, 0, nv_codec->width, nv_codec->height, pbuff);
		NvRmSurfaceWrite(&pNvMM->Payload.Surfaces.Surfaces[1], 0, 0, nv_codec->width/2, nv_codec->height/2, pbuff + luma_size);
		NvRmSurfaceWrite(&pNvMM->Payload.Surfaces.Surfaces[2], 0, 0, nv_codec->width/2, nv_codec->height/2, pbuff + luma_size +chroma_size );
	}
	else {
		PRINT("read nvmm from yuv: - error read %d|%d\n", read_size, luma_size);
		free(pbuff);
		return total_read;
	}


	total_read = read_size;

	PRINT("%s: -- :%u\n", __func__, read_size);

	free(pbuff);
	return total_read;
}

	static unsigned
read_yuv(
		nv_omx_codec_t * nv_codec,
		unsigned char * p_buffer,
		unsigned * p_size)
{
	unsigned read_size = 0;
	unsigned luma_size = nv_codec->height * nv_codec->width;
	unsigned  frame_size = luma_size + luma_size/2;

	PRINT("%s: ++\n", __func__);

	//read_size = fread(p_buffer, 1, frame_size, nv_codec->f_input);
	//add by jiangxiaohui
	read_size = luma_size*3/2;
	memcpy(p_buffer, nv_codec->pInput, read_size);


	if (read_size == 0)
	{
		PRINT("%s: - return 0\n", __func__);
		return 0;
	}
	*p_size = read_size;

	PRINT("%s: - :%d \n", __func__, read_size);

	return read_size;
}

static void read_loop(nv_omx_codec_t * nv_codec)
{
	static unsigned frame_decoded = 0;
	unsigned bytes_read = 0;
	unsigned size;
	int filelen;
	clock_t start = 0,end = 0;
	nv_codec->input_index = 0;

	fseek(nv_codec->f_input, 0, SEEK_END);
	filelen = ftell(nv_codec->f_input);
	fseek(nv_codec->f_input, 0, SEEK_SET);
	unsigned char * readBuffer = (unsigned char *)malloc(sizeof(unsigned char) * nv_codec->max_bitstream_size);
	OMX_BOOL bEOSSent = OMX_FALSE;


	while(nv_codec->read_thread.is_running)
	{
		// Any empty input buffers
		sem_wait(&nv_codec->sem_empty_buffer_done);

		if (bEOSSent == OMX_TRUE)
			break;

		size = 0;
		{
			if (nv_codec->use_nvmm) {
				bytes_read = read_nvmm_from_yuv(nv_codec);
				size = sizeof(NvMMBuffer);
			}
			else {
				bytes_read = read_yuv(nv_codec, readBuffer, &size);
			}
		}

		nv_codec->inputBuffer[nv_codec->input_index]->nFlags = 0;

		if (nv_codec->have_header_info)
		{
			nv_codec->inputBuffer[nv_codec->input_index]->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;
		}

		//		  if( filelen == ftell( hInputFile ) )
		//		  {
		//			  if (bVerbose)
		//				  PRINT("Reader Sending EOS\n");
		//			  inputBuffer[InputIndex]->nFlags |= OMX_BUFFERFLAG_EOS;
		//			  bEOSSent = OMX_TRUE;
		//		  }

		/* 发送完成结束线程 */
		//if (bytes_read == 0)
		{
			PRINT("Read loop, Reader Sending EOS\n");
			nv_codec->inputBuffer[nv_codec->input_index]->nFlags |= OMX_BUFFERFLAG_EOS;
			bEOSSent = OMX_TRUE;
		}

		if (nv_codec->use_nvmm) {
			PRINT("copy %u size %u to input\n", nv_codec->input_index, size);
			memcpy(
					(OMX_U8 *)nv_codec->inputBuffer[nv_codec->input_index]->pBuffer,
					(nv_codec->input_nvmm_buf[nv_codec->input_index]),
					size);
		}
		else {
			memcpy(
					(OMX_U8 *)nv_codec->inputBuffer[nv_codec->input_index]->pBuffer,
					&readBuffer[0],
					size);
		}

		// Set length of valid data
		nv_codec->inputBuffer[nv_codec->input_index]->nFilledLen = size;
		nv_codec->inputBuffer[nv_codec->input_index]->nOffset = 0;
		// Send filled input buffer to component
		PRINT("%s: Read From File HDR 0x%x Buffer:0x%x Size:%u\n", __func__,
				(unsigned int)nv_codec->inputBuffer[nv_codec->input_index],
				(unsigned int)nv_codec->inputBuffer[nv_codec->input_index]->pBuffer,
				size);
		OMX_EmptyThisBuffer(nv_codec->component, nv_codec->inputBuffer[nv_codec->input_index]);

		if (nv_codec->golden_fps != 0)
		{
			if (frame_decoded == 0)
			{
				start = clock();
			}
			if (nv_codec->inputBuffer[nv_codec->input_index]->nFilledLen == 0)
			{
				end = clock();
			}
			frame_decoded++;
		}

		nv_codec->input_index++;
		if (nv_codec->input_index >= nv_codec->num_input_buffers)
		{
			nv_codec->input_index = 0;
		}
	}

	if (nv_codec->golden_fps != 0)
	{
		nv_codec->time_to_decode = (end-start)/CLOCKS_PER_SEC;
		nv_codec->cal_fps = frame_decoded/nv_codec->time_to_decode;
		PRINT("\nnv_omx_video_t :: Calculated Fps:%d \tGolden Fps:%d \n", (int)nv_codec->cal_fps, (int)nv_codec->golden_fps);
		if (nv_codec->cal_fps > (nv_codec->golden_fps+10))
			PRINT("\n -------------- nv_omx_codec_t FAILED (Abnormally high fps) ---------------------\n");
		else if (nv_codec->cal_fps >= (nv_codec->golden_fps-5))
			PRINT("\n -------------- nv_omx_codec_t PASSED ---------------------\n");
		else
			PRINT("\n -------------- nv_omx_codec_t FAILED (Fps lower than golden value) ---------------------\n");
	}

	free(readBuffer);
}

void * read_thread(void *self)
{
	read_loop((nv_omx_codec_t *)self);
	return NULL;
}

	static unsigned
write_yuv(
		FILE * h_file,
		unsigned char * p_buffer,
		unsigned size)
{
	PRINT("%s: ++\n", __func__);

	unsigned write_size = fwrite(p_buffer, 1, size, h_file);

	PRINT("%s: -- :%u \n", __func__, write_size);

	return write_size;
}

/*for H264 & h265, use same function*/
	static unsigned
write_nal(
		unsigned char *p_H264buf,
		unsigned char * p_buffer,
		unsigned size)
{
	PRINT("%s: ++\n", __func__);

	//unsigned write_size = fwrite(p_buffer, 1, size, h_file);
	memcpy(p_H264buf, p_buffer, size);

	static int frame = 0;
	PRINT("%s: cnt :%d -- time :%ld:%u\n", __func__,frame++, get_time_ms(), size);

	return size;
}

static void write_loop(nv_omx_codec_t * nv_codec)
{
	nv_codec->output_index = 0;

	while(nv_codec->read_thread.is_running)
	{
		sem_wait(&nv_codec->sem_fill_buffer_done);

		if (nv_codec->is_eos)
			break;

		PRINT("%s: HDR: 0x%x pBuffer:0x%x  len: %ld\n", __func__,
				(unsigned int)nv_codec->outputBuffer[nv_codec->output_index],
				(unsigned int)nv_codec->outputBuffer[nv_codec->output_index]->pBuffer,
				nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen);

		if(nv_codec->outputBuffer[nv_codec->output_index]->nFlags & OMX_BUFFERFLAG_EOS )
		{
			PRINT("Got OMX_EventBufferFlag event\n");
			nv_codec->is_eos = 1;
			sem_post(&(nv_codec->sem_fill_buffer_done));
			sem_post(&(nv_codec->sem_eos));
		}

			write_nal(nv_codec->pOutput,
					nv_codec->outputBuffer[nv_codec->output_index]->pBuffer,
					nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen);
			OMX_FillThisBuffer(nv_codec->component, nv_codec->outputBuffer[nv_codec->output_index]);
			nv_codec->output_index = nv_codec->output_index + 1;
			if (nv_codec->output_index >= nv_codec->num_output_buffers)
			{
				nv_codec->output_index = 0;
			}

		/*OMX_FillThisBuffer(nv_codec->component, nv_codec->outputBuffer[nv_codec->output_index]);
		  nv_codec->output_index = nv_codec->output_index + 1;
		  if (nv_codec->output_index >= nv_codec->num_output_buffers)
		  {
		  nv_codec->output_index = 0;
		  }*/
	}
}

void * write_thread(void *self)
{
	write_loop((nv_omx_codec_t *)self);
	return NULL;
}


void port_settings_changed(nv_omx_codec_t * nv_codec, OMX_U32 nPortIndex)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32  i;

	PRINT("%s ++, after disable port\n", __func__);

	OMX_PARAM_PORTDEFINITIONTYPE oPortDef;
	INIT_PARAM(oPortDef);
	oPortDef.nPortIndex = nPortIndex;
	eError = OMX_GetParameter(
			nv_codec->component,
			OMX_IndexParamPortDefinition,
			&oPortDef);

	PRINT("%s, video width:%lu, height:%lu, size:%lu\n", __func__, oPortDef.format.video.nFrameWidth, oPortDef.format.video.nFrameHeight, oPortDef.nBufferSize);

	// allocate new output buffer
	for (i = 0; i < nv_codec->outputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_AllocateBuffer(
				nv_codec->component,
				&nv_codec->outputBuffer[i],
				oPortDef.nPortIndex,
				nv_codec,
				oPortDef.nBufferSize);

		if (eError != OMX_ErrorNone)
		{
			PRINT("port_settings_changed::AllocateBuffer Error: %x\n", eError);
		}
		nv_codec->outputBuffer[i]->nFlags = 0;
		PRINT("port_settings_changed::AllocateBuffer size: %ld HDR: 0x%x buffer: 0x%x\n",
				nv_codec->outputPortDef.nBufferSize,
				(unsigned int)nv_codec->outputBuffer[i],
				(unsigned int)nv_codec->outputBuffer[i]->pBuffer);
	}

	OMX_SendCommand(
			nv_codec->component,
			OMX_CommandPortEnable,
			nPortIndex,
			0 );

	PRINT("%s ++, after enable port\n", __func__);
	nv_codec->num_output_buffers = nv_codec->outputPortDef.nBufferCountActual;

	PRINT("%s: --\n", __func__);
}

void nv_omx_codec_init(nv_omx_codec_t * nv_codec)
{
	PRINT("%s: ++\n", __func__);
	nv_codec->c_width = 0;
	nv_codec->c_height = 0;
	//nv_codec->use_nvmm = 0;
	nv_codec->input_nvmm_buf = NULL;
	nv_codec->input_index = 0;
	nv_codec->is_eos = 0;
	nv_codec->have_header_info = 0;
	nv_codec->f_input = fopen(nv_codec->name_input, "rb");
	if (!nv_codec->f_input)
	{
		PRINT("inputFile: Notfound \n");
	}

	nv_codec->f_output = fopen(nv_codec->name_output,"wb");
	if (!nv_codec->f_output)
	{
		PRINT("outputFilename : Not able to create \n");
	}
	sem_init(&nv_codec->sem_state_change, 0, 0);
	sem_init(&nv_codec->sem_empty_buffer_done, 0, 0);
	sem_init(&nv_codec->sem_fill_buffer_done, 0, 0);
	sem_init(&nv_codec->sem_eos, 0, 0);

	PRINT("%s: --\n", __func__);
}

void nv_omx_codec_deinit(nv_omx_codec_t * nv_codec)
{
	PRINT("%s: ++\n", __func__);
	release_nvmm_buf(nv_codec);
	destroy_thread(&nv_codec->read_thread);
	destroy_thread(&nv_codec->write_thread);
	if (nv_codec->f_input)
		fclose(nv_codec->f_input);
	if (nv_codec->f_output)
		fclose(nv_codec->f_output);

	sem_destroy(&nv_codec->sem_state_change);
	sem_destroy(&nv_codec->sem_empty_buffer_done);
	sem_destroy(&nv_codec->sem_fill_buffer_done);
	sem_destroy(&nv_codec->sem_eos);

	PRINT("%s: --\n", __func__);
}



#define ALIGNMENT 16

static codecs encoders[] = {
    {H264, "OMX.Nvidia.h264.encoder"},
};



/*****************************************************************************
 函 数 名  : EvEncodeFrame
 功能描述  : 硬件编码接口
 输入参数  : const char *pInbuff
             const int InBuffLen
             char *pOutbuf
             int *pOutBuffLen
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : jiangxiaohui
    修改内容   : 新生成函数

*****************************************************************************/
bool EvEncodeFrame(stCodec_param stCodec, unsigned char *pInbuff, int InBuffLen, unsigned char *pOutbuf, int *pOutBuffLen)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 i;
	nv_omx_codec_t * nv_codec = NULL;

	if ((NULL == pInbuff) || (NULL == pOutbuf) || (NULL == pOutBuffLen))
	{
		return false;
	}

	// Set OpenMAX version
	vOMX.s.nVersionMajor = 1;
	vOMX.s.nVersionMinor = 1;
	vOMX.s.nRevision = 0;
	vOMX.s.nStep = 0;

	nv_codec = (nv_omx_codec_t *)malloc(sizeof(nv_omx_codec_t));
	if (NULL == nv_codec)
	{
		return false;
	}

	nv_codec->codec = INVALID;
	nv_codec->golden_fps = 0;

	PRINT("\n ++++++++++++++ nv_omx_codec_t START +++++++++++++++++++++ time :%ld \n ", get_time_ms());
	//OMX_Init();

	nv_codec->width  = stCodec.width;
	nv_codec->height = stCodec.height;
	nv_codec->fps    = stCodec.fps;
	nv_codec->use_nvmm = stCodec.fps;
	nv_codec->pInput = pInbuff;
	nv_codec->pOutput = pOutbuf;
	nv_codec->nInputSize = InBuffLen;
	nv_codec->pOututSize = pOutBuffLen;

	nv_codec->codec = H264;


	nv_omx_codec_init(nv_codec);
	nv_codec->max_bitstream_size =
		( ( ( nv_codec->width + ALIGNMENT - 1 ) / ALIGNMENT ) * ALIGNMENT *
		  ( ( nv_codec->height + ALIGNMENT - 1 ) / ALIGNMENT ) * ALIGNMENT *  3 ) / 2;

	nv_codec->read_thread.start_routine = read_thread;
	nv_codec->read_thread.arg = nv_codec;
	nv_codec->read_thread.sema = nv_codec->sem_empty_buffer_done;
	create_thread(&nv_codec->read_thread);

	nv_codec->write_thread.start_routine = write_thread;
	nv_codec->write_thread.arg = nv_codec;
	nv_codec->write_thread.sema = nv_codec->sem_fill_buffer_done;
	create_thread(&nv_codec->write_thread);

	// Set callback functions
	nv_codec->oCallbacks.EventHandler    = EventHandler;
	nv_codec->oCallbacks.EmptyBufferDone = EmptyBufferDone;
	nv_codec->oCallbacks.FillBufferDone  = FillBufferDone;

	// get component handle
	//PRINT("---- set use NVMM2\n");
	//nv_codec->use_nvmm = OMX_TRUE;

		eError = OMX_GetHandle(
				&nv_codec->component,
				(OMX_STRING)encoders[nv_codec->codec].name,
				nv_codec,
				&nv_codec->oCallbacks);

		// set use NVMM
		if (nv_codec->use_nvmm) {
			PRINT("----- set use NVMM buffer3\n");
			OMX_INDEXTYPE eIndex;
			NVX_PARAM_USENVBUFFER useNvBuffParam;
			useNvBuffParam.nSize = sizeof(NVX_PARAM_USENVBUFFER);
			useNvBuffParam.nVersion = vOMX;
			useNvBuffParam.bUseNvBuffer = OMX_TRUE;
			useNvBuffParam.nPortIndex = 0; // input port

			eError = OMX_GetExtensionIndex (nv_codec->component,
					"OMX.Nvidia.index.config.usenvbuffer", &eIndex);
			if (eError == OMX_ErrorNone) {
				NvOsDebugPrintf("set use nv surface\n");
				OMX_SetParameter (nv_codec->component, eIndex, &useNvBuffParam);
				if (eError == OMX_ErrorNone) {
					PRINT("------- set use NVMM success\n");
					//omx_codec->nMaxBitStreamSize = sizeof(NvMMBuffer);
				}
			}
		}

	if (eError != OMX_ErrorNone)
	{
		PRINT("GetHandle %s OMX_Error: %x\n", (OMX_STRING)encoders[nv_codec->codec].name, eError);
		goto clean_up;
	}

	// get input port definition
	INIT_PARAM(nv_codec->inputPortDef);
	nv_codec->inputPortDef.nPortIndex = 0;
	eError = OMX_GetParameter(
			nv_codec->component,
			OMX_IndexParamPortDefinition,
			&nv_codec->inputPortDef);
	if (eError != OMX_ErrorNone)
	{
		PRINT("nv_omx_codec_t:: GetParameter OMX_Error: %x\n", eError);
		goto clean_up;
	}

	nv_codec->inputPortDef.nBufferCountActual
		= nv_codec->inputPortDef.nBufferCountMin;
	nv_codec->inputPortDef.nBufferSize = nv_codec->max_bitstream_size;
	if ( nv_codec->use_nvmm) {
		nv_codec->inputPortDef.nBufferSize = sizeof(NvMMBuffer);
		PRINT("---- input 304\n");
	}
	nv_codec->inputPortDef.format.video.nFrameWidth = nv_codec->width;
	nv_codec->inputPortDef.format.video.nFrameHeight = nv_codec->height;

	{
		eError = OMX_SetParameter(
				nv_codec->component,
				OMX_IndexParamPortDefinition,
				&nv_codec->inputPortDef);
	}

	// allocate input buffer

	nv_codec->num_input_buffers = nv_codec->inputPortDef.nBufferCountActual;
	alloc_nvmm_buf(nv_codec);

	for (i = 0; i < nv_codec->inputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_UseBuffer(
				nv_codec->component,
				&nv_codec->inputBuffer[i],
				nv_codec->inputPortDef.nPortIndex,
				nv_codec,
				sizeof(NvMMBuffer),
				nv_codec->input_nvmm_buf[i]
				);
		if (eError != OMX_ErrorNone)
		{
			PRINT("nv_omx_codec_t:: AllocateBuffer OMX_Error: %x\n", eError);
			goto clean_up;
		}

		nv_codec->inputBuffer[i]->nFlags = 0;
		PRINT("AllocateBuffer Input: size: %ld HDR: 0x%x buffer: 0x%x\n",
				nv_codec->inputPortDef.nBufferSize,
				(unsigned int)nv_codec->inputBuffer[i],
				(unsigned int)nv_codec->inputBuffer[i]->pBuffer);
	}

	// get output port definition

	INIT_PARAM(nv_codec->outputPortDef);
	nv_codec->outputPortDef.nPortIndex = 1;

	eError = OMX_GetParameter(
			nv_codec->component,
			OMX_IndexParamPortDefinition,
			&nv_codec->outputPortDef
			);
	if (eError != OMX_ErrorNone)
	{
		PRINT("nv_omx_codec_t:: GetParameter Output OMX_Error: %x\n", eError);
		goto clean_up;
	}

	nv_codec->output_size = (nv_codec->width*nv_codec->height*3)/2;
	nv_codec->outputPortDef.nBufferCountActual
		= nv_codec->outputPortDef.nBufferCountMin;
	nv_codec->outputPortDef.format.video.nFrameWidth = nv_codec->width;
	nv_codec->outputPortDef.format.video.nFrameHeight = nv_codec->height;
	nv_codec->outputPortDef.nBufferSize = nv_codec->max_bitstream_size;

	eError = OMX_SetParameter(
			nv_codec->component,
			OMX_IndexParamPortDefinition,
			&nv_codec->outputPortDef);
	if (eError != OMX_ErrorNone)
	{
		PRINT("nv_omx_codec_t:: SetParameter Output OMX_Error: %x\n", eError);
		goto clean_up;
	}

	// allocate output buffer
	for (i = 0; i < nv_codec->outputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_AllocateBuffer(
				nv_codec->component,
				&nv_codec->outputBuffer[i],
				nv_codec->outputPortDef.nPortIndex,
				nv_codec,
				nv_codec->outputPortDef.nBufferSize);
		if (eError != OMX_ErrorNone)
		{
			PRINT("nv_omx_codec_t::AllocateBuffer Error: %x\n", eError);
			goto clean_up;
		}
		nv_codec->outputBuffer[i]->nFlags = 0;
		PRINT("AllocateBuffer size: %ld HDR: 0x%x buffer: 0x%x\n",
				nv_codec->outputPortDef.nBufferSize,
				(unsigned int)nv_codec->outputBuffer[i],
				(unsigned int)nv_codec->outputBuffer[i]->pBuffer);
	}

	eError = OMX_GetParameter(
			nv_codec->component,
			OMX_IndexParamPortDefinition,
			&nv_codec->outputPortDef);
	if (eError != OMX_ErrorNone)
	{
		PRINT("nv_omx_codec_t:: SetParameter Output OMX_Error: %x\n", eError);
		goto clean_up;
	}

	nv_codec->num_output_buffers = nv_codec->outputPortDef.nBufferCountActual;

	PRINT("**** nv_omx_codec_t:: All preparation Done ***** \n");


	// set component state to idle
	set_state(nv_codec, nv_codec->component, OMX_StateIdle, OMX_TRUE);
	// set component state to execute
	set_state(nv_codec, nv_codec->component, OMX_StateExecuting, OMX_TRUE);
	PRINT("nv_omx_codec_t:: setState StateExecuting \n");

	// Send empty output buffers to component
	for(i = 0; i < nv_codec->num_output_buffers; i++)
	{
		nv_codec->outputBuffer[i]->nFilledLen = 0;
		PRINT("FillThisBuffer (1):: %p\n",
				nv_codec->outputBuffer[i]);
		OMX_FillThisBuffer(nv_codec->component,
				nv_codec->outputBuffer[i]);
	}
	// trigger read thread
	for( i = 0 ; i < nv_codec->num_input_buffers; i ++ )
		sem_post(&(nv_codec->sem_empty_buffer_done));

	PRINT("nv_omx_codec_t:: Wait till EOS \n");

	// wait till end
	sem_wait(&nv_codec->sem_eos);


	PRINT("EOS received ");

	// set state to idle
	set_state(nv_codec, nv_codec->component, OMX_StateIdle, OMX_TRUE);

	OMX_SendCommand(
			nv_codec->component,
			OMX_CommandFlush,
			nv_codec->inputPortDef.nPortIndex, 0 );
	sem_wait(&(nv_codec->sem_state_change));

	OMX_SendCommand(
			nv_codec->component,
			OMX_CommandFlush,
			nv_codec->outputPortDef.nPortIndex, 0 );
	sem_wait(&(nv_codec->sem_state_change));

	// release output buffers
	for (i = 0; i < nv_codec->outputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_FreeBuffer(
				nv_codec->component,
				nv_codec->outputPortDef.nPortIndex,
				nv_codec->outputBuffer[i]);
	}

	// release input buffers
	for (i = 0; i < nv_codec->inputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_FreeBuffer(
				nv_codec->component,
				nv_codec->inputPortDef.nPortIndex,
				nv_codec->inputBuffer[i]);
	}

clean_up:

	eError = OMX_FreeHandle(nv_codec->component);

	nv_omx_codec_deinit(nv_codec);
	//OMX_Deinit();

	free(nv_codec);
	PRINT("\n -------------- nv_omx_codec_t END ---------------------\n");
	return true;
}



