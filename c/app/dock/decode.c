/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : decode.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月15日
  最近修改   :
  功能描述   : 解码处理，H264格式解码成YUV420格式
  函数列表   :
              create_decode_thread
              decode1_handle_thd
  修改历史   :
  1.日    期   : 2016年4月15日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "decode.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static pthread_mutex_t m_decode_count_mutex;
static pthread_t m_decode_thread[DECODE_THREAD_NUM];   /* 四路解码线程的ID */
static u8 m_decoded_count = 0;    /* 四路输入数据中已解码路数计数 */

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

 /* 一路摄像头输入数据的最大长度 */
#define MAX_CAMERA_INPUT_DATA_LEN    ((1280 * 1080 * 3) / 2)



/*****************************************************************************
 函 数 名  : iDecodeFrame
 功能描述  : 解码接口
 说    明  : 由于解码模块的源代码多线程并行操作有问题，暂时解决方案:每个解码线程单独调用一份独立的解码API
*****************************************************************************/
static bool iDecodeFrame(stCodec_param stCodec, const char *pInbuff, const int InBuffLen, char *pOutbuf, int *pOutBuffLen)
{
	bool bRet = false;

	if (m_decode_thread[0] == pthread_self())
	{
		bRet = EvDecodeFrame(stCodec, pInbuff, InBuffLen, pOutbuf, pOutBuffLen);
	}
	else if (m_decode_thread[1] == pthread_self())
	{
		bRet = EvDecodeFrame_1(stCodec, pInbuff, InBuffLen, pOutbuf, pOutBuffLen);
	}
	else if (m_decode_thread[2] == pthread_self())
	{
		bRet = EvDecodeFrame_2(stCodec, pInbuff, InBuffLen, pOutbuf, pOutBuffLen);
	}
	else if (m_decode_thread[3] == pthread_self())
	{
		bRet = EvDecodeFrame_3(stCodec, pInbuff, InBuffLen, pOutbuf, pOutBuffLen);
	}
	else
	{
		msg("(%s, %d) param error !", __FUNCTION__, __LINE__);
	}

	return bRet;
}


/*****************************************************************************
 函 数 名  : decode_handle_thd
 功能描述  : 解码线程
 输入参数  :
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月16日
    作    者   : jiangxiaohui

    修改内容   : 新生成函数

*****************************************************************************/
static void *decode_handle_thd(void *arg)
{
	int i = 0;
	bool bRet = false;
	stDock *pstDock = (stDock *)arg;
	char *pDecodeOutBuf = NULL; /* 一路摄像头数据解码后输出的数据存储buffer */
	u32 nLen = 0;
	stCodec_param stCodec;

	if(NULL == pstDock)
	{
		msg("(%s, %d)param error !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	/* 申请解码输出buff空间 */
	pDecodeOutBuf = (char*)malloc(sizeof(char) * MAX_CAMERA_INPUT_DATA_LEN);
	if (NULL == pDecodeOutBuf)
	{
		msg("(%s, %d)malloc fail !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	/* 确定当前线程 */
	for (i = 0; i < DECODE_THREAD_NUM; i++)
	{
		if (m_decode_thread[i] == pthread_self())
		{
		    break;
		}
	}

	if (DECODE_THREAD_NUM <= i)
	{
		msg("can't find my thread !\n");
		return NULL;
	}
	
	while(1)
	{
		memset(pDecodeOutBuf, 0, MAX_CAMERA_INPUT_DATA_LEN);

		/*设置解码参数*/
		memset(&stCodec, 0, sizeof(stCodec_param));
		stCodec.width    = 1280;
		stCodec.height   = 1080;
		stCodec.fps      = 25;
		stCodec.use_nvmm = 1;

        /* 等待球机数据线程 ball_data_thd 来唤醒 */
		sem_wait(&pstDock->sem_decode[i]);

        /* 读取一路摄像头数据, 调用硬件解码操作 */
		bRet = iDecodeFrame(stCodec, gFrameBuffer.pszFrameBuffer[i], gFrameBuffer.nLen[i], pDecodeOutBuf, &nLen);
		if (!bRet)
		{
			msg("(%s, %d)EvDecodeFrame error !\n", __FUNCTION__, __LINE__);
			continue;
		}


        if (CAMERA_DATA_INPUT_NUM != m_decoded_count)
		{
			pthread_mutex_lock(&m_decode_count_mutex);
			m_decoded_count++;
			pthread_mutex_unlock(&m_decode_count_mutex);
		}
		else
		{
	        /* m_decoded_count 被增加到四时，表示解码完成一帧完整的数据 */
			pthread_mutex_lock(&m_decode_count_mutex);
			m_decoded_count = 0;
			pthread_mutex_unlock(&m_decode_count_mutex);

            /* 如果上一帧拼接已经完成 */
			if (pstDock->stitch_done_flag)
			{
				//将解码后的数据拷贝到用于拼接处理的数据buff,准备去做拼接处理
				//memcpy(g_pDecodebuff[i].pszFrameBuffer, pDecodeOutBuf, nLen);
				pstDock->pDecodeToStitchBuffer[i] = pDecodeOutBuf;  //将拼接输入buf指向 pDecodeOutBuf.

				//唤醒拼接处理线程 stitch_handle_thd
				sem_post(&pstDock->sem_stitch);

	            /* 设置解码完成标志 */
				pstDock->decode_done_flag = true;
			}
			else
			{
				//丢弃解码的数据
				memset(pDecodeOutBuf, 0, MAX_CAMERA_INPUT_DATA_LEN);
			}
		}

		if (pstDock->stop_data_flag)
		{
			break;
		}
	}

	free(pDecodeOutBuf);

	//释放用于解码操作输入buffer分配的空间
	free(pstDock->pDecodeToStitchBuffer[i]);

	pthread_exit("thread done");
}



/*****************************************************************************
 函 数 名  : create_decode_thread
 功能描述  : 创建解码线程 (4个解码线程，每个线程解码一路数据)， 由预览前段线程 preview_front_thd 创建
 输入参数  : void
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 蒋小辉
    修改内容   : 新生成函数

*****************************************************************************/
bool create_decode_thread(stDock *pstDock)
{
	bool bRet = true;
    int res = 0;
	int i = 0;

	/*解码模块初始化,并行的解码之前只需要初始化一次即可*/
	OMX_Init();

	//为解码操作的输入buffer分配空间,并初始化
	for(i = 0; i < CAMERA_DATA_INPUT_NUM; i++)
	{
		pstDock->pDecodeToStitchBuffer[i] = (char*)malloc(sizeof(char) * MAX_CAMERA_INPUT_DATA_LEN);
		if(NULL == pstDock->pDecodeToStitchBuffer[i])
		{
			msg("(%s, %d)malloc index %d buff fail !\n", __FUNCTION__, __LINE__, i);
			return false;
		}

		memset(pstDock->pDecodeToStitchBuffer[i], 0, MAX_CAMERA_INPUT_DATA_LEN);
	}

	/* 创建4个解码线程 */
	for (i = 0 ; i < DECODE_THREAD_NUM; i++)
	{
	    res = pthread_create(&m_decode_thread[i], NULL, (void *)decode_handle_thd, (stDock *)pstDock);
	    if (res != 0)
		{
	    	msg("pthread_create thread%d fail !\n", i);
			bRet = false;
			break;
	    }
	}

	/*结束解码模块, 与 OMX_Init() 对应*/
	//OMX_Deinit();

	for (i = 0; i < DECODE_THREAD_NUM; i++)
	{
	    /* 设置子线程为脱离状态 */
	    pthread_detach(pstDock->decodeThread[i]);
	}

    return bRet;
}



/*****************************************************************************
 函 数 名  : decode_module_Init
 功能描述  : 解码模块初始化
 输入参数  : void
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月16日
    作    者   : 蒋小辉

    修改内容   : 新生成函数

*****************************************************************************/
bool decode_module_Init(void)
{
	if (0 != pthread_mutex_init(&m_decode_count_mutex, NULL))
	{
		printf("(%s, %d)pthread_mutex_init fail\n", __FUNCTION__, __LINE__);
		return false;
	}

	return true;
}

/*****************************************************************************
 函 数 名  : decode_module_uninit
 功能描述  : 解码模块结束
 输入参数  : void
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 蒋小辉

    修改内容   : 新生成函数

*****************************************************************************/
bool decode_module_uninit(void)
{
	if (0 != pthread_mutex_destroy(&m_decode_count_mutex))
	{
		msg("(%s, %d)pthread_mutex_destroy fail\n", __FUNCTION__, __LINE__);
		return false;
	}

	return true;
}

