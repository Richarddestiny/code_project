/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : stitch.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月18日
  最近修改   :
  功能描述   : 拼接处理流程
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月18日
    作    者   : jiangxiaohui
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "include/stitch.h"
#include "../stitch/stitching.h"


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

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : stitch_handle_thd
 功能描述  : 拼接处理线程, 由预览操作线程 preview_opera_thd创建
 输入参数  :
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年4月16日
    作    者   : 蒋小辉
    修改内容   : 新生成函数

*****************************************************************************/
void *stitch_handle_thd(void *arg)
{
	int i = 0;
	stDock *pstDock = (stDock *)arg;
	u32 FinalImgLen = 0; /* 拼接操作后的输出数据大小 */
	char *pStitchOutBuf = NULL; /* 拼接后输出的数据存储buffer */
	int stitchFlag = STITCH_COARSE;
	ImageParam IP; /*图像参数，包括输入图像的数目、宽度、高度、尺寸以及输出图像的宽度、高度、尺寸。*/

	if (NULL == pstDock)
	{
		msg("(%s, %d)Param error !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if ((pstDock->HDMI_preview_flag) || (pstDock->app_preview_flag))
	{
		stitchFlag = STITCH_COARSE;  /*粗拼接*/
	}
	else if (pstDock->video_stitch_flag)
	{
		stitchFlag = STITCH_REFINED; /*细拼接*/
	}

	/*拼接初始化*/
	memset(&IP, 0, sizeof(ImageParam));
	if (!StitchInit("/usr/data/param_medium.evo", &IP, 5, 5, 0, 0, stitchFlag))
	{
		msg("error!!\n");
		return NULL;
	}

	msg("Stitch version = %s\n", GetStitchingVer());
	msg("nFinalWidth: %d, nFinalHeight: %d\n", IP.nFinalWidth, IP.nFinalHeight);

	FinalImgLen = (IP.nFinalWidth * IP.nFinalHeight * 3) / 2;
	pStitchOutBuf = (char *)malloc(FinalImgLen);
	if (!pStitchOutBuf)
	{
		msg("(%s, %d)malloc pStitchOut fail !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	pstDock->pHdmiPreviewBuffer = (char *)malloc(FinalImgLen);
	if (!pstDock->pHdmiPreviewBuffer)
	{
		msg("(%s, %d)malloc pHdmiPreviewBuffer fail !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	pstDock->pStitchToEncodeBuffer = (char *)malloc(FinalImgLen);
	if (!pstDock->pStitchToEncodeBuffer)
	{
		msg("(%s, %d)malloc pStitchToEncodeBuffer fail !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	while(1)
	{
	    /* 等待 decode_handle_thd 线程来唤醒它 */
		sem_wait(&pstDock->sem_stitch);

		//拼接操作
		StitchProcess(pstDock->pDecodeToStitchBuffer[0],
					  pstDock->pDecodeToStitchBuffer[1],
		              pstDock->pDecodeToStitchBuffer[2],
		              pstDock->pDecodeToStitchBuffer[3],
		              pStitchOutBuf);

        /* 如果有打开HDMI预览标志 */
		if (pstDock->HDMI_preview_flag)
		{
			/* 将拼接后的数据地址拷贝到HDMI输出端 */
			memcpy(pstDock->pHdmiPreviewBuffer, pStitchOutBuf, FinalImgLen);

			/* 唤醒HDMI预览线程 hdmi_preview_thd */
			sem_post(&pstDock->sem_HDMI_preview);
		}

        /* 如果有打开App预览标志和录像拼接标志 */
		if ((pstDock->app_preview_flag) || (pstDock->video_stitch_flag))
		{
			if (pstDock->encode_done_flag)
			{
				//将拼接后的数据传到encode模块去
				memcpy(pstDock->pStitchToEncodeBuffer, pStitchOutBuf, FinalImgLen);

				//上一帧编码完成，唤醒编码处理线程 encode_handle_thd
				sem_post(&pstDock->sem_encode);
			}
			/* 丢弃完成拼接的帧数据 */
		}

        //设置拼接完成标志
		pstDock->stitch_done_flag = true;

        /* 停止数据流和录像拼接都关闭时退出循环，关闭线程 */
		if (!pstDock->stop_data_flag && !pstDock->video_stitch_flag)
    		break;
	}

	free(pStitchOutBuf);
	free(pstDock->pHdmiPreviewBuffer);
	free(pstDock->pStitchToEncodeBuffer);

	StitchUninit();

	pthread_exit("thread stitch_handle_thd done!");
}


