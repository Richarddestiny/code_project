/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : stitch.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��18��
  ����޸�   :
  ��������   : ƴ�Ӵ�������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��18��
    ��    ��   : jiangxiaohui
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "include/stitch.h"
#include "../stitch/stitching.h"


/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : stitch_handle_thd
 ��������  : ƴ�Ӵ����߳�, ��Ԥ�������߳� preview_opera_thd����
 �������  :
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��16��
    ��    ��   : ��С��
    �޸�����   : �����ɺ���

*****************************************************************************/
void *stitch_handle_thd(void *arg)
{
	int i = 0;
	stDock *pstDock = (stDock *)arg;
	u32 FinalImgLen = 0; /* ƴ�Ӳ������������ݴ�С */
	char *pStitchOutBuf = NULL; /* ƴ�Ӻ���������ݴ洢buffer */
	int stitchFlag = STITCH_COARSE;
	ImageParam IP; /*ͼ���������������ͼ�����Ŀ����ȡ��߶ȡ��ߴ��Լ����ͼ��Ŀ�ȡ��߶ȡ��ߴ硣*/

	if (NULL == pstDock)
	{
		msg("(%s, %d)Param error !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	if ((pstDock->HDMI_preview_flag) || (pstDock->app_preview_flag))
	{
		stitchFlag = STITCH_COARSE;  /*��ƴ��*/
	}
	else if (pstDock->video_stitch_flag)
	{
		stitchFlag = STITCH_REFINED; /*ϸƴ��*/
	}

	/*ƴ�ӳ�ʼ��*/
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
	    /* �ȴ� decode_handle_thd �߳��������� */
		sem_wait(&pstDock->sem_stitch);

		//ƴ�Ӳ���
		StitchProcess(pstDock->pDecodeToStitchBuffer[0],
					  pstDock->pDecodeToStitchBuffer[1],
		              pstDock->pDecodeToStitchBuffer[2],
		              pstDock->pDecodeToStitchBuffer[3],
		              pStitchOutBuf);

        /* ����д�HDMIԤ����־ */
		if (pstDock->HDMI_preview_flag)
		{
			/* ��ƴ�Ӻ�����ݵ�ַ������HDMI����� */
			memcpy(pstDock->pHdmiPreviewBuffer, pStitchOutBuf, FinalImgLen);

			/* ����HDMIԤ���߳� hdmi_preview_thd */
			sem_post(&pstDock->sem_HDMI_preview);
		}

        /* ����д�AppԤ����־��¼��ƴ�ӱ�־ */
		if ((pstDock->app_preview_flag) || (pstDock->video_stitch_flag))
		{
			if (pstDock->encode_done_flag)
			{
				//��ƴ�Ӻ�����ݴ���encodeģ��ȥ
				memcpy(pstDock->pStitchToEncodeBuffer, pStitchOutBuf, FinalImgLen);

				//��һ֡������ɣ����ѱ��봦���߳� encode_handle_thd
				sem_post(&pstDock->sem_encode);
			}
			/* �������ƴ�ӵ�֡���� */
		}

        //����ƴ����ɱ�־
		pstDock->stitch_done_flag = true;

        /* ֹͣ��������¼��ƴ�Ӷ��ر�ʱ�˳�ѭ�����ر��߳� */
		if (!pstDock->stop_data_flag && !pstDock->video_stitch_flag)
    		break;
	}

	free(pStitchOutBuf);
	free(pstDock->pHdmiPreviewBuffer);
	free(pstDock->pStitchToEncodeBuffer);

	StitchUninit();

	pthread_exit("thread stitch_handle_thd done!");
}


