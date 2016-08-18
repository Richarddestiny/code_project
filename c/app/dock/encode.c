/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ�

 ******************************************************************************
  �� �� ��   : encode.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��18��
  ����޸�   :
  ��������   : ���봦������
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��18��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "encode.h"
#include "include/dbg.h"

/*****************************************************************************
 �� �� ��  : encode_handle_thd
 ��������  : ���봦���߳�, ��app������߳� app_cmd_handle_thd ����
 �������  :
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : jiangxiaohui

    �޸�����   : �����ɺ���

*****************************************************************************/
void* encode_handle_thd(void *arg)
{
	bool bRet = false;
	stDock *pstDock = (stDock *)arg;
	char *pEncodeOutBuf = NULL;         /* �������������ݴ洢buffer */
	u32 nLen = 0;
	stCodec_param stCodec;

	if (NULL == pstDock)
	{
		msg("(%s, %d)Param error !\n", __FUNCTION__, __LINE__);
		return;
	}

	/* ����������������ݴ洢���������������ֱ������� */
	pEncodeOutBuf = (char*)malloc(sizeof(char) * STITCH_VIDEO_FREAM_LEN);
	if(NULL == pEncodeOutBuf)
	{
		msg("(%s, %d)malloc buff fail !\n", __FUNCTION__, __LINE__);
		return false;
	}

	/* ���������App��Ԥ�����ݴ洢������ */
	pstDock->pAppPreviewBuffer = (char*)malloc(sizeof(char) * STITCH_PREVIEW_FREAM_LEN);
	if(NULL == pstDock->pAppPreviewBuffer)
	{
		msg("(%s, %d)malloc buff fail !\n", __FUNCTION__, __LINE__);
		return false;
	}

    /* ����鵽AppԤ����־Ϊfalseʱ���˳�ѭ�����ر��߳� */
	while(true)
	{
	    /* �ȴ� stitch_handle_thd �߳��������� */
		sem_wait(&pstDock->sem_encode);

		if (pstDock->app_preview_flag)
		{
			/*���ñ������ -- �ֱ���: 1920*1080*/
			memset(&stCodec, 0, sizeof(stCodec_param));
			stCodec.width    = 1920;
			stCodec.height   = 1080;
			stCodec.fps      = 25;
			stCodec.use_nvmm = 1;
		}
		else if (pstDock->video_stitch_flag)
		{
			/*���ñ������ -- �ֱ���: 2048*1024 */
			memset(&stCodec, 0, sizeof(stCodec_param));
			stCodec.width    = 2048;
			stCodec.height   = 1024;
			stCodec.fps      = 25;
			stCodec.use_nvmm = 1;
		}

		//��ȡƴ�Ӻ��֡���ݣ�����Ӳ������ӿ�
		bRet = EvEncodeFrame(stCodec, pstDock->pStitchToEncodeBuffer, STITCH_PREVIEW_FREAM_LEN, pEncodeOutBuf, &nLen);
		if (!bRet)
		{
			msg("(%s, %d)EvEncodeFrame error !!!\n", __FUNCTION__, __LINE__);
			continue;
		}

	    /* ����д�AppԤ����־ */
	    if (pstDock->app_preview_flag)
	    {
            /* ���App������ɱ�־ */
			if (pstDock->app_send_done_flag)
			{
				//�����������ݵ�ַ������app
				memcpy(pstDock->pAppPreviewBuffer, pEncodeOutBuf, nLen);
				pstDock->stAppFrameHead.uiLen = nLen;
				/* [add_code]Ҫ��д֡ͷ */

				/* ����appԤ���߳� app_preview_thd */
				sem_post(&pstDock->sem_app_preview);
			}
			/* ������ɱ����֡���� */
		}

	    /* ����д�¼��ƴ�ӱ�־ */
        if (pstDock->video_stitch_flag)
        {
    		//FFmpegת����������¼���ļ�(ƴ�Ӻ��H264 -> Mp4�ļ�)
    		char *path = "../file/media/media.mp4"; //ʽ��

    		FFmpegH264ToMP4(pEncodeOutBuf, nLen, path);
        }

		pstDock->encode_done_flag = true; /*���ñ�����ɱ�־*/

        /* AppԤ����¼��ƴ�Ӷ��ر�ʱ�˳�ѭ�����ر��߳� */
		if (!pstDock->app_preview_flag && !pstDock->video_stitch_flag)
    		break;
	}

	free(pEncodeOutBuf);
	free(pstDock->pAppPreviewBuffer);
	pthread_exit("thread encode_handle_thd done!");
}

