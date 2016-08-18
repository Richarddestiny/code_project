/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : decode.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��15��
  ����޸�   :
  ��������   : ���봦��H264��ʽ�����YUV420��ʽ
  �����б�   :
              create_decode_thread
              decode1_handle_thd
  �޸���ʷ   :
  1.��    ��   : 2016��4��15��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "decode.h"

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
static pthread_mutex_t m_decode_count_mutex;
static pthread_t m_decode_thread[DECODE_THREAD_NUM];   /* ��·�����̵߳�ID */
static u8 m_decoded_count = 0;    /* ��·�����������ѽ���·������ */

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

 /* һ·����ͷ�������ݵ���󳤶� */
#define MAX_CAMERA_INPUT_DATA_LEN    ((1280 * 1080 * 3) / 2)



/*****************************************************************************
 �� �� ��  : iDecodeFrame
 ��������  : ����ӿ�
 ˵    ��  : ���ڽ���ģ���Դ������̲߳��в��������⣬��ʱ�������:ÿ�������̵߳�������һ�ݶ����Ľ���API
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
 �� �� ��  : decode_handle_thd
 ��������  : �����߳�
 �������  :
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��16��
    ��    ��   : jiangxiaohui

    �޸�����   : �����ɺ���

*****************************************************************************/
static void *decode_handle_thd(void *arg)
{
	int i = 0;
	bool bRet = false;
	stDock *pstDock = (stDock *)arg;
	char *pDecodeOutBuf = NULL; /* һ·����ͷ���ݽ������������ݴ洢buffer */
	u32 nLen = 0;
	stCodec_param stCodec;

	if(NULL == pstDock)
	{
		msg("(%s, %d)param error !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	/* ����������buff�ռ� */
	pDecodeOutBuf = (char*)malloc(sizeof(char) * MAX_CAMERA_INPUT_DATA_LEN);
	if (NULL == pDecodeOutBuf)
	{
		msg("(%s, %d)malloc fail !\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	/* ȷ����ǰ�߳� */
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

		/*���ý������*/
		memset(&stCodec, 0, sizeof(stCodec_param));
		stCodec.width    = 1280;
		stCodec.height   = 1080;
		stCodec.fps      = 25;
		stCodec.use_nvmm = 1;

        /* �ȴ���������߳� ball_data_thd ������ */
		sem_wait(&pstDock->sem_decode[i]);

        /* ��ȡһ·����ͷ����, ����Ӳ��������� */
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
	        /* m_decoded_count �����ӵ���ʱ����ʾ�������һ֡���������� */
			pthread_mutex_lock(&m_decode_count_mutex);
			m_decoded_count = 0;
			pthread_mutex_unlock(&m_decode_count_mutex);

            /* �����һ֡ƴ���Ѿ���� */
			if (pstDock->stitch_done_flag)
			{
				//�����������ݿ���������ƴ�Ӵ��������buff,׼��ȥ��ƴ�Ӵ���
				//memcpy(g_pDecodebuff[i].pszFrameBuffer, pDecodeOutBuf, nLen);
				pstDock->pDecodeToStitchBuffer[i] = pDecodeOutBuf;  //��ƴ������bufָ�� pDecodeOutBuf.

				//����ƴ�Ӵ����߳� stitch_handle_thd
				sem_post(&pstDock->sem_stitch);

	            /* ���ý�����ɱ�־ */
				pstDock->decode_done_flag = true;
			}
			else
			{
				//�������������
				memset(pDecodeOutBuf, 0, MAX_CAMERA_INPUT_DATA_LEN);
			}
		}

		if (pstDock->stop_data_flag)
		{
			break;
		}
	}

	free(pDecodeOutBuf);

	//�ͷ����ڽ����������buffer����Ŀռ�
	free(pstDock->pDecodeToStitchBuffer[i]);

	pthread_exit("thread done");
}



/*****************************************************************************
 �� �� ��  : create_decode_thread
 ��������  : ���������߳� (4�������̣߳�ÿ���߳̽���һ·����)�� ��Ԥ��ǰ���߳� preview_front_thd ����
 �������  : void
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��15��
    ��    ��   : ��С��
    �޸�����   : �����ɺ���

*****************************************************************************/
bool create_decode_thread(stDock *pstDock)
{
	bool bRet = true;
    int res = 0;
	int i = 0;

	/*����ģ���ʼ��,���еĽ���֮ǰֻ��Ҫ��ʼ��һ�μ���*/
	OMX_Init();

	//Ϊ�������������buffer����ռ�,����ʼ��
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

	/* ����4�������߳� */
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

	/*��������ģ��, �� OMX_Init() ��Ӧ*/
	//OMX_Deinit();

	for (i = 0; i < DECODE_THREAD_NUM; i++)
	{
	    /* �������߳�Ϊ����״̬ */
	    pthread_detach(pstDock->decodeThread[i]);
	}

    return bRet;
}



/*****************************************************************************
 �� �� ��  : decode_module_Init
 ��������  : ����ģ���ʼ��
 �������  : void
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��16��
    ��    ��   : ��С��

    �޸�����   : �����ɺ���

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
 �� �� ��  : decode_module_uninit
 ��������  : ����ģ�����
 �������  : void
 �������  : ��
 �� �� ֵ  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ��С��

    �޸�����   : �����ɺ���

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

