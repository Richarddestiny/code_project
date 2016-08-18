/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技

 ******************************************************************************
  文 件 名   : encode.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月18日
  最近修改   :
  功能描述   : 解码处理流程
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月18日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "encode.h"
#include "include/dbg.h"

/*****************************************************************************
 函 数 名  : encode_handle_thd
 功能描述  : 编码处理线程, 由app命令处理线程 app_cmd_handle_thd 创建
 输入参数  :
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : jiangxiaohui

    修改内容   : 新生成函数

*****************************************************************************/
void* encode_handle_thd(void *arg)
{
	bool bRet = false;
	stDock *pstDock = (stDock *)arg;
	char *pEncodeOutBuf = NULL;         /* 编码后输出的数据存储buffer */
	u32 nLen = 0;
	stCodec_param stCodec;

	if (NULL == pstDock)
	{
		msg("(%s, %d)Param error !\n", __FUNCTION__, __LINE__);
		return;
	}

	/* 申请编码后输出的数据存储缓冲区，按照最大分辨率申请 */
	pEncodeOutBuf = (char*)malloc(sizeof(char) * STITCH_VIDEO_FREAM_LEN);
	if(NULL == pEncodeOutBuf)
	{
		msg("(%s, %d)malloc buff fail !\n", __FUNCTION__, __LINE__);
		return false;
	}

	/* 申请输出给App的预览数据存储缓冲区 */
	pstDock->pAppPreviewBuffer = (char*)malloc(sizeof(char) * STITCH_PREVIEW_FREAM_LEN);
	if(NULL == pstDock->pAppPreviewBuffer)
	{
		msg("(%s, %d)malloc buff fail !\n", __FUNCTION__, __LINE__);
		return false;
	}

    /* 当检查到App预览标志为false时，退出循环，关闭线程 */
	while(true)
	{
	    /* 等待 stitch_handle_thd 线程来唤醒它 */
		sem_wait(&pstDock->sem_encode);

		if (pstDock->app_preview_flag)
		{
			/*设置编码参数 -- 分辨率: 1920*1080*/
			memset(&stCodec, 0, sizeof(stCodec_param));
			stCodec.width    = 1920;
			stCodec.height   = 1080;
			stCodec.fps      = 25;
			stCodec.use_nvmm = 1;
		}
		else if (pstDock->video_stitch_flag)
		{
			/*设置编码参数 -- 分辨率: 2048*1024 */
			memset(&stCodec, 0, sizeof(stCodec_param));
			stCodec.width    = 2048;
			stCodec.height   = 1024;
			stCodec.fps      = 25;
			stCodec.use_nvmm = 1;
		}

		//读取拼接后的帧数据，调用硬件编码接口
		bRet = EvEncodeFrame(stCodec, pstDock->pStitchToEncodeBuffer, STITCH_PREVIEW_FREAM_LEN, pEncodeOutBuf, &nLen);
		if (!bRet)
		{
			msg("(%s, %d)EvEncodeFrame error !!!\n", __FUNCTION__, __LINE__);
			continue;
		}

	    /* 如果有打开App预览标志 */
	    if (pstDock->app_preview_flag)
	    {
            /* 检查App发送完成标志 */
			if (pstDock->app_send_done_flag)
			{
				//将编码后的数据地址拷贝到app
				memcpy(pstDock->pAppPreviewBuffer, pEncodeOutBuf, nLen);
				pstDock->stAppFrameHead.uiLen = nLen;
				/* [add_code]要填写帧头 */

				/* 唤醒app预览线程 app_preview_thd */
				sem_post(&pstDock->sem_app_preview);
			}
			/* 丢弃完成编码的帧数据 */
		}

	    /* 如果有打开录像拼接标志 */
        if (pstDock->video_stitch_flag)
        {
    		//FFmpeg转换处理，生成录像文件(拼接后的H264 -> Mp4文件)
    		char *path = "../file/media/media.mp4"; //式例

    		FFmpegH264ToMP4(pEncodeOutBuf, nLen, path);
        }

		pstDock->encode_done_flag = true; /*设置编码完成标志*/

        /* App预览和录像拼接都关闭时退出循环，关闭线程 */
		if (!pstDock->app_preview_flag && !pstDock->video_stitch_flag)
    		break;
	}

	free(pEncodeOutBuf);
	free(pstDock->pAppPreviewBuffer);
	pthread_exit("thread encode_handle_thd done!");
}

