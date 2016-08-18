
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

#include "nvddk_2d_v2.h"

#include "nv_buffer.h"
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

	NvRmDeviceHandle rm;

	NvMMBuffer *yuv420buffer;
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
    int use_nvmm;  // 1:用上它表示用的是nv private buffer。0:不用它的话，用的是malloc的buffer
    int disabledpb;
    NvU32 start;
    NvU32 end;
    codec_type codec;
    struct nv_buffer ** input_nvmm_buf;
} nv_omx_codec_t;

static long get_time_ms(void);


static void * read_thread(void *self);

static void * write_thread(void * self);



static void nv_omx_codec_init(nv_omx_codec_t * nv_codec);

static void nv_omx_codec_deinit(nv_omx_codec_t * nv_codec);

static void set_state(nv_omx_codec_t * nv_codec, OMX_HANDLETYPE handle, OMX_STATETYPE state, OMX_BOOL wait);

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
static NvBool bDisableOuputPort = NV_FALSE;

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
			//MemMgr* mgr = MemMgrnew();
			nv_codec->is_eos = 1;
			PRINT("EOS\n");
			sem_post(&(nv_codec->sem_fill_buffer_done));
			/*sem_post(&(mgr->sem_vic_done));
			  if(nv_codec->is_dec)
			  {
			  pthread_mutex_lock(&mgr->mutex_dec_exit);
			  pthread_cond_wait(&mgr->cond_dec_exit, &mgr->mutex_dec_exit);
			  pthread_mutex_unlock(&mgr->mutex_dec_exit);
			  }*/
			sem_post(&(nv_codec->sem_eos));

			break;

		case OMX_EventPortSettingsChanged:
			PRINT("Got OMX_EventPortSettingsChanged event ++ data1:%lu, \n", nData1 );

			if (OMX_IndexParamPortDefinition == (OMX_INDEXTYPE)nData2 &&
					(nData1 == 1) )
			{
				//our openmax has a bug, it always report a DRC(dynamic resolution change) event when decoder begin,  so first time don't handle it
				if( bfirst == 1 ){
					bfirst = 0;
					break;
				}
				nPortIndex = nData1;
				PRINT("Got OMX_EventPortSettingsChanged event:%d \n",(int)nPortIndex);
				bDisableOuputPort = NV_TRUE;
				//MemMgrnew()->WaitBuffertoBeConsumed(); //before sending disable command, we need make sure, all buffer has been drained
				result = OMX_SendCommand(
						nv_codec->component,
						OMX_CommandPortDisable,
						nPortIndex, 0 );
				if (result != OMX_ErrorNone) {
					PRINT("OMX_SendCommand failed 0x%x \n",result);
				}
				else{
					PRINT("command OMX_CommandPortDisable successfully!\n");
				}
			}
			else
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
			PRINT("Got OMX_EventCmdComplete event, dec\n");
			nPortIndex = nData2;
			if( nData1 == OMX_CommandPortDisable ) {
				bDisableOuputPort = NV_FALSE;
				port_settings_changed(nv_codec, nPortIndex);
			}
			else if (nData1 == OMX_CommandPortEnable)
			{
				// start filling buffers
				PRINT("decoder, OMX_CommandPortEnable");
				for(i = 0; i < nv_codec->num_output_buffers; i++)
				{
					nv_codec->outputBuffer[i]->nFilledLen = 0;
					OMX_FillThisBuffer(nv_codec->component, nv_codec->outputBuffer[i]);
				}
			}
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
	if (bDisableOuputPort == NV_TRUE &&  pBuffer->nFilledLen == 0) {
		PRINT("DRC scenario, FREE BUFFER\n");
		//sleep(1);
		OMX_FreeBuffer(nv_codec->component,
				nv_codec->outputPortDef.nPortIndex,
				pBuffer);
		return OMX_ErrorNone;
	}
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


static void release_nvmm_buf(nv_omx_codec_t * nv_codec)
{
	unsigned i;

	if (!nv_codec->input_nvmm_buf) return;

	for (i = 0; i < nv_codec->num_input_buffers; ++i) {
		nv_buffer_del(&(nv_codec->input_nvmm_buf[i]));
	}
	free(nv_codec->input_nvmm_buf);
}

	static unsigned
read_nvmm_from_yuv(
		nv_omx_codec_t * nv_codec)
{
	unsigned read_size = 0;
	unsigned luma_size = nv_codec->height * nv_codec->width;
	unsigned chroma_size = luma_size / 2;
	unsigned total_read = 0;

	unsigned char * pbuff = (unsigned char *)malloc(sizeof(unsigned char) * luma_size);
	nv_buffer_t * pnvbuff = nv_codec->input_nvmm_buf[nv_codec->input_index];
	//PRINT("%p %u %p\n", nv_codec->input_nvmm_buf, nv_codec->InputIndex, pnvbuff);
	NvMMBuffer * pNvMM = nv_buffer_get_nvmm(pnvbuff);

	PRINT("%s: read nvmm[%u] from NV12: + size %d\n", __func__, nv_codec->input_index, luma_size + chroma_size);

	read_size = fread(pbuff, 1, luma_size, nv_codec->f_input);

	if (read_size == luma_size) {
		//memset(pbuff, 128, read_size);
		NvRmSurfaceWrite(&pNvMM->Payload.Surfaces.Surfaces[0], 0, 0, nv_codec->width, nv_codec->height, pbuff);
	}
	else {
		PRINT("read nvmm from yuv: - error read %d|%d\n", read_size, luma_size);
		free(pbuff);
		return total_read;
	}
	total_read = read_size;

	read_size = fread(pbuff, 1, chroma_size, nv_codec->f_input);

	if (read_size == chroma_size) {
		NvRmSurfaceWrite(&pNvMM->Payload.Surfaces.Surfaces[1], 0, 0, nv_codec->width/2, nv_codec->height/2, pbuff);
	}
	else {
		PRINT("read nvmm from yuv: - error read %d|%d\n", read_size, chroma_size);
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

	read_size = fread(p_buffer, 1, frame_size, nv_codec->f_input);


	if (read_size == 0)
	{
		PRINT("%s: - return 0\n", __func__);
		return 0;
	}
	*p_size = read_size;

	PRINT("%s: - :%d \n", __func__, read_size);

	return read_size;
}

/*H264 reader*/
	static unsigned
read_nal(
		nv_omx_codec_t * nv_codec,
		unsigned char * p_buffer,
		unsigned * p_size)
{
	static unsigned s_total = 0;
	unsigned count = 0, vop = 0, val = 0, buf_size = 0;
	unsigned char * temp = p_buffer;

	PRINT("%s: ++\n", __func__);

	buf_size = fread(p_buffer, 1, nv_codec->max_bitstream_size - 1024, nv_codec->f_input);
	if (buf_size == 0)
	{
		return 0;
	}
	count = 0;
	while (count < buf_size)
	{
		val = (temp[0] << 24);
		val = val | (temp[1] << 16);
		val = val | (temp[2] << 8);
		val = val | temp[3];

		if (val == NAL_UNIT_START_CODE)
		{
			if (vop == 1)
			{
				vop = 0;
				break;
			}
			vop = 1;
		}
		count ++;
		temp++;
	}
	s_total += count;
	*p_size = count;
	fseek(nv_codec->f_input, s_total, SEEK_SET);

	PRINT("%s: - :%u \n", __func__, count);
	return count;
}

static void read_loop(nv_omx_codec_t * nv_codec)
{
	static unsigned frame_decoded = 0;
	unsigned bytes_read = 0;
	unsigned size;
	int filelen;
	clock_t start = 0,end = 0;
	nv_codec->input_index = 0;

	/*
	//以下三条语句，获取输入文件的长度
	fseek(nv_codec->f_input, 0, SEEK_END);
	filelen = ftell(nv_codec->f_input);
	fseek(nv_codec->f_input, 0, SEEK_SET);
	*/
	unsigned char * readBuffer = (unsigned char *)malloc(sizeof(unsigned char) * nv_codec->max_bitstream_size);
	OMX_BOOL bEOSSent = OMX_FALSE;

	while(nv_codec->read_thread.is_running)
	{
		// Any empty input buffers
		sem_wait(&nv_codec->sem_empty_buffer_done);

		if (bEOSSent == OMX_TRUE)
			break;

		//bytes_read = read_nal(nv_codec, readBuffer, &size);  //从输入文件中读取， size应该与bytes_read相同
		size = nv_codec->nInputSize;  //输入数据的长度
		memcpy(readBuffer, nv_codec->pInput, size);

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


			memcpy(
					(OMX_U8 *)nv_codec->inputBuffer[nv_codec->input_index]->pBuffer,
					&readBuffer[0],
					size);

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

static void * read_thread(void *self)
{
	read_loop((nv_omx_codec_t *)self);
	return NULL;
}

static unsigned
write_yuv(
		nv_omx_codec_t * nv_codec,
		unsigned char * p_buffer,
		unsigned size)
{
	PRINT("%s: ++\n", __func__);

	//unsigned write_size = fwrite(p_buffer, 1, size, h_file);
	memcpy(nv_codec->pOutput, p_buffer, size);  //add by jiangxiaohui 20160418
	*(nv_codec->pOututSize) = size; //输出的buff大小

	PRINT("%s: -- :%u \n", __func__, size);

	return size;
}

/*for H264 & h265, use same function*/
	static unsigned
write_nal(
		FILE * h_file,
		unsigned char * p_buffer,
		unsigned size)
{
	PRINT("%s: ++\n", __func__);

	unsigned write_size = fwrite(p_buffer, 1, size, h_file);

	static int frame = 0;
	PRINT("%s: cnt :%d -- time :%ld:%u\n", __func__,frame++, get_time_ms(), write_size);

	return write_size;
}

static void convertnvmmnv12toyuv420( NvRmDeviceHandle rm, NvMMBuffer* from, NvMMBuffer* to )
{
	int width = from->Payload.Surfaces.Surfaces[0].Width;
	int height = from->Payload.Surfaces.Surfaces[0].Height;
	 int result = 1;
	 NvError err = NvSuccess;
	 NvDdk2dHandle ddk2d;
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
	 dst_rect.right = width;
	 dst_rect.top = 0;
	 dst_rect.bottom = height;
	 src_rect.left = NV_SFX_ZERO;
	 src_rect.right = NV_SFX_ADD (src_rect.left,
		 NV_SFX_WHOLE_TO_FX (width));
	 src_rect.top = NV_SFX_ZERO;
	 src_rect.bottom = NV_SFX_ADD (src_rect.top,
		 NV_SFX_WHOLE_TO_FX (height));

	 //NvRmSurface * src_surfs = nv_buffer_get_surfaces(other);
	 NvMMSurfaceDescriptor * p_src_desc = &(from->Payload.Surfaces);
	 //NvRmSurface * dst_surfs = nv_buffer_get_surfaces(my);
	 NvMMSurfaceDescriptor * p_dst_desc = &(to->Payload.Surfaces);
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

static NvMMBuffer* createtestbuffer(  int m_width, int m_height )
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

    NvColorFormat   nvColorFormat[] =
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


static NvMMBuffer* allocnvmmbufferyuv420(  NvRmDeviceHandle rm, int width ,int height )
{

	NvError err;
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
	NvMMSurfaceDescriptor * desc;

	NvMMBuffer *nvmm = (NvMMBuffer *)malloc(sizeof(NvMMBuffer));
	memset(nvmm, 0, sizeof(NvMMBuffer));

	nvmm->StructSize = sizeof(NvMMBuffer);
	nvmm->PayloadType = NvMMPayloadType_SurfaceArray;
	//impl->nvmm->PayloadInfo.TimeStamp = 0;
	attrs[1] = NvRmSurfaceLayout_Blocklinear;


	desc = &(nvmm->Payload.Surfaces);

	plane_format[0] = NvColorFormat_Y8;
	plane_format[1] = NvColorFormat_U8;
	plane_format[2] = NvColorFormat_V8;
	desc->SurfaceCount = 3;

	surfs = desc->Surfaces;
	width = (width + 1) & ~1;
	height = (height + 1) & ~1;

	if (width < 32)
		width = 32;


	total_size = 0;
	NvRmMultiplanarSurfaceSetup(surfs, desc->SurfaceCount,
		width, height, color_format, plane_format, attrs);
	for (i = 0; i < desc->SurfaceCount; i++)
	{
		{
			align = NvRmSurfaceComputeAlignment(rm, &surfs[i]);
		}

		size = NvRmSurfaceComputeSize(&surfs[i]);

		NvRmMemHandleCreate(rm, &(surfs[i].hMem), size);

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
	return nvmm;
FAIL:
	free(nvmm);
	return NULL;
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
			//MemMgr* mgr = MemMgrnew();
			//sem_post(&(mgr->sem_vic_done));
			sem_post(&(nv_codec->sem_eos));
		}


			//unsigned char * pTempBuffer = nv_codec->outputBuffer[nv_codec->output_index]->pBuffer;
			//unsigned tSize = 0;
			PRINT("------ get buffer %u size %lu\n", nv_codec->output_index, nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen);
			/*MemMgr* mgr = MemMgrnew();
			  if(nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen != 0){
			  sem_post(&(mgr->sem_vic_done));
			  }*/

			if (nv_codec->use_nvmm) {
				unsigned l_size, tSize;
				unsigned char * buff ;
				NvMMBuffer * pNvMM;
				convertnvmmnv12toyuv420( nv_codec->rm, nv_codec->outputBuffer[nv_codec->output_index]->pBuffer,nv_codec->yuv420buffer );
				pNvMM = nv_codec->yuv420buffer;
				//PRINT("---- count %d\n", pNvMM->Payload.Surfaces.SurfaceCount);
				l_size = pNvMM->Payload.Surfaces.Surfaces[0].Width * pNvMM->Payload.Surfaces.Surfaces[0].Height;
				buff = (OMX_U8 *) malloc(sizeof(OMX_U8) * l_size * 3 / 2);
				if (pNvMM->Payload.Surfaces.SurfaceCount == 3) {
				//PRINT("---- write NVMM NV12\n");
				NvRmSurfaceRead(&(pNvMM->Payload.Surfaces.Surfaces[0]), 0, 0, pNvMM->Payload.Surfaces.Surfaces[0].Width, pNvMM->Payload.Surfaces.Surfaces[0].Height, buff);
				NvRmSurfaceRead(&(pNvMM->Payload.Surfaces.Surfaces[1]), 0, 0, pNvMM->Payload.Surfaces.Surfaces[1].Width, pNvMM->Payload.Surfaces.Surfaces[1].Height, buff + l_size);
				NvRmSurfaceRead(&(pNvMM->Payload.Surfaces.Surfaces[2]), 0, 0, pNvMM->Payload.Surfaces.Surfaces[2].Width, pNvMM->Payload.Surfaces.Surfaces[2].Height, buff + l_size + l_size /4 );
				}
				tSize = l_size * 3 / 2;
				write_yuv(nv_codec, buff, tSize);
				free(buff);
			}
			else if ((nv_codec->c_width != 0) || (nv_codec->c_height != 0))
			{
				/*  // Write Y //
				    tSize = nv_codec->width * nv_codec->height;
				    write_yuv(nv_codec->f_output, pTempBuffer, tSize);

				// Write U //
				pTempBuffer += (nv_codec->c_width * nv_codec->c_height);
				tSize = (nv_codec->width / 2) * (nv_codec->height / 2);
				write_yuv(nv_codec->f_output, pTempBuffer, tSize);

				// Write V //
				pTempBuffer += ((nv_codec->c_width / 2 ) * (nv_codec->c_height/ 2));
				tSize = ((nv_codec->width/2) * (nv_codec->height/2));
				write_yuv(nv_codec->f_output, pTempBuffer, tSize);*/
			}
			else
			{
				write_yuv(nv_codec,
						nv_codec->outputBuffer[nv_codec->output_index]->pBuffer,
						nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen);
			}
			nv_codec->outputBuffer[nv_codec->output_index]->nFilledLen = 0;


		OMX_FillThisBuffer(nv_codec->component, nv_codec->outputBuffer[nv_codec->output_index]);
		  nv_codec->output_index = nv_codec->output_index + 1;
		  if (nv_codec->output_index >= nv_codec->num_output_buffers)
		  {
		  nv_codec->output_index = 0;
		  }
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
	nv_codec->f_input = NULL;
	nv_codec->f_output = NULL;

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

static codecs decoders[] = {
    {H264, "OMX.Nvidia.h264.decode"},
};

static codecs encoders[] = {
    {H264, "OMX.Nvidia.h264.encoder"},
};


/*****************************************************************************
 函 数 名  : EvDecodeFrame
 功能描述  : 硬件解码接口
 输入参数  : const char *pInbuff
             const int InBuffLen
             char *pOutbuf
             int *pOutBuffLen
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : jiangxiaohui
    修改内容   : 新生成函数

*****************************************************************************/
bool EvDecodeFrame_1(stCodec_param stCodec, unsigned char *pInbuff, int InBuffLen, unsigned char *pOutbuf, int *pOutBuffLen)
{
	OMX_ERRORTYPE eError = OMX_ErrorNone;
	OMX_U32 i;
	NvError err;
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
		PRINT("malloc fail !\n");
		return false;
	}

	PRINT("\n ++++++++++++++ nv_omx_codec_t START +++++++++++++++++++++ time :%ld \n ", get_time_ms());
	//OMX_Init();

	nv_codec->golden_fps = 0;

	nv_codec->width  = stCodec.width;
	nv_codec->height = stCodec.height;
	nv_codec->fps    = stCodec.fps;
	nv_codec->use_nvmm = stCodec.fps;

	nv_codec->pInput = pInbuff;
	nv_codec->pOutput = pOutbuf;
	nv_codec->nInputSize = InBuffLen;
	nv_codec->pOututSize = pOutBuffLen;


	//PRINT("---- set use NVMM2\n");
	//nv_codec->use_nvmm = OMX_TRUE;

	err = NvRmOpen(&(nv_codec->rm),0);
	if (err != NvSuccess) {
		nv_codec->rm = NULL;
		goto clean_up;
	}
	nv_codec->yuv420buffer = createtestbuffer(  nv_codec->width, nv_codec->height );
	nv_codec->codec = H264;


	/*
	if ((nv_codec->name_input == NULL) &&
			(nv_codec->name_output == NULL))
	{
		PRINT(" nv_omx_codec_t:: Enc/Dec INPUT Filename not specified \n");
		goto clean_up;
	}
	else if (nv_codec->name_output == NULL)
	{
		PRINT(" nv_omx_codec_t:: Enc/Dec OUTPUT Filename not specified \n");
		goto clean_up;
	}

		if ((nv_codec->width == 0)  ||
				(nv_codec->height == 0))
		{
			PRINT(" nv_omx_codec_t::Enc:Dec width/height/fps not specified\n");
			goto clean_up;
		}

		PRINT("\n\t\t H264 DECODER TEST SAMPLE\n");
		PRINT("Input File Name:          %s\n",nv_codec->name_input);
		PRINT("Output Filename :%s\n",nv_codec->name_output);
	*/

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
				(OMX_STRING)decoders[nv_codec->codec].name,
				nv_codec,
				&nv_codec->oCallbacks);

		// Use lowbuffers
		OMX_INDEXTYPE eIndexExt;
		eError = OMX_GetExtensionIndex(
				nv_codec->component,
				NVX_INDEX_PARAM_USELOWBUFFER,
				&eIndexExt);

		NVX_PARAM_USELOWBUFFER useLowBuffer;
		INIT_PARAM(useLowBuffer);
		useLowBuffer.bUseLowBuffer = OMX_TRUE;
		useLowBuffer.nPortIndex = 0;
		eError = OMX_SetParameter(
				nv_codec->component,
				eIndexExt,
				&useLowBuffer);
		if (eError != OMX_ErrorNone)
		{
			PRINT("nv_omx_codec_t:: lowbuffers OMX_Error: %x\n", eError);
			goto clean_up;
		}

		// disable DPB
		eError = OMX_GetExtensionIndex(
				nv_codec->component,
				NVX_INDEX_PARAM_H264_DISABLE_DPB,
				&eIndexExt);
		NVX_PARAM_H264DISABLE_DPB dpbmode;
		INIT_PARAM(dpbmode);
		dpbmode.bDisableDPB = OMX_TRUE;
		eError = OMX_SetParameter(
				nv_codec->component,
				eIndexExt,
				&dpbmode);
		if (eError != OMX_ErrorNone)
		{
			PRINT("nv_omx_codec_t:: Disable DPB OMX_Error: %x\n", eError);
			goto clean_up;
		}

		// set use NVMM
		if (nv_codec->use_nvmm) {
			PRINT("----- set use NVMM buffer3\n");
			OMX_INDEXTYPE eIndex;
			NVX_PARAM_USENVBUFFER useNvBuffParam;
			useNvBuffParam.nSize = sizeof(NVX_PARAM_USENVBUFFER);
			useNvBuffParam.nVersion = vOMX;
			useNvBuffParam.bUseNvBuffer = OMX_TRUE;
			useNvBuffParam.nPortIndex = 1; // output port

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

	nv_codec->inputPortDef.format.video.nFrameWidth = nv_codec->width;
	nv_codec->inputPortDef.format.video.nFrameHeight = nv_codec->height;



	// allocate input buffer

	for (i = 0; i < nv_codec->inputPortDef.nBufferCountActual; i++)
	{
		eError = OMX_AllocateBuffer(
				nv_codec->component,
				&nv_codec->inputBuffer[i],
				nv_codec->inputPortDef.nPortIndex,
				nv_codec,
				nv_codec->inputPortDef.nBufferSize);
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

	if (nv_codec->use_nvmm )
		nv_codec->output_size = sizeof(NvMMBuffer);
	else
		nv_codec->output_size = (nv_codec->width*nv_codec->height*3)/2;
	nv_codec->outputPortDef.nBufferCountActual
		= nv_codec->outputPortDef.nBufferCountMin;
	nv_codec->outputPortDef.format.video.nFrameWidth = nv_codec->width;
	nv_codec->outputPortDef.format.video.nFrameHeight = nv_codec->height;
	nv_codec->outputPortDef.nBufferSize = nv_codec->max_bitstream_size;
	if ( nv_codec->use_nvmm) {
		//omx_codec->outputPortDef.nBufferSize = sizeof(NvMMBuffer);
		PRINT("---- output 304\n");
	}

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

	nv_codec->num_input_buffers = nv_codec->inputPortDef.nBufferCountActual;
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


