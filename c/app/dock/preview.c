/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : preview.c
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��22��
  ����޸�   :
  ��������   : ʵʱԤ����غ���ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��22��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "include/preview.h"

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
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : preview_fornt_thd
 ��������  : Ԥ��ǰ���߳�ʵ�ֺ���
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void *preview_fornt_thd(void *arg)
{
	pthread_t stitchThread;
    stDock *pstDock = (stDock *)arg;

    dbg("(Preview Fornt Thread) Enter preview_fornt_thd\n\n");

    /* ������������߳� */
    pthread_create(&pstDock->BallDataThread, NULL, ball_data_thd, (void *)pstDock);

    /* �����ĸ����봦���߳� */
    create_decode_thread(pstDock);

    /* ����ƴ�Ӵ����߳� */
    pthread_create(&stitchThread, NULL, stitch_handle_thd, (void *)pstDock);

    return NULL;
}

/*****************************************************************************
 �� �� ��  : hdmi_preview_thd
 ��������  : Ԥ�������߳�ʵ�ֺ���
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void *hdmi_preview_thd(void *arg)
{
    int fp = 0;
    u32 ulScreenSize;
    char *fbp;
    stDock *pstDock = (stDock *)arg;

    dbg("(Preview Handle Thread) Enter preview_handle_thd\n\n");

    /* ��HDMI�豸�ļ� */
    fp = open("/dev/fb1", O_RDWR);
    if (fp < 0)
    {
        msg("Error: Can not open framebuffer device FB1\n");
        return;
    }

    /* ӳ���ڴ浽�򿪵��豸�ļ��� */
    ulScreenSize = (HDMI_PREVIEW_WIDTH * HDMI_PREVIEW_HEIGHT * 3) / 2;
	fbp = (char *)mmap(0, ulScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	if (fbp == NULL)
	{
		msg("Error: failed to map framebuffer device to memory\n");
		close(fp);
		return;
	}

    /* ����鵽HDMI�ӿڱ��γ�ʱ���˳�ѭ�����ر��߳� */
    while (pstDock->HDMI_preview_flag)
    {
        /* �ȴ�ƴ����Ϻ��� */
		sem_wait(&pstDock->sem_HDMI_preview);

        /* ����HDMIԤ�����ݵ��ӿ��� */
        memcpy(fbp, pstDock->pHdmiPreviewBuffer, ulScreenSize);

        /* ����HDMIԤ��������ɱ�־ */
        pstDock->HDMI_send_done_flag = true;
    }

    munmap(fbp, ulScreenSize);
    close(fp);
    dbg("(Preview Handle Thread) Quit preview_handle_thd\n\n");
}

/*****************************************************************************
 �� �� ��  : hdmi_status_check
 ��������  : HDMI�ӿ�����״̬���ʵ�ֺ���
 �������  : void
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void hdmi_status_check(stDock *pstDock)
{
    //char *ret;
    int ret;
    char str[100];
    bool bCutStatus = false;
    static bool bOldStatus = false;     /* Ĭ������ǰδ����HDMI */

    /* �򿪼������ļ� */
    //freopen("/sys/kernel/debug/tegra_hdmi/regs", "r", stdin);
    //freopen("/datadisk/ee/regs", "r", stdin);

    /* ��ȡ��ǰHDMI�ӿ�״̬ */

    //ret = gets(str);
    if (g_hdmi_debug_switch)
    {
        if (g_init_debug_switch)
            dbg("(main Thread) Get hdmi status connect\n");
        bCutStatus = true;
    }
    else
    {
        if (g_init_debug_switch)
            dbg("(main Thread) Get hdmi status disconnect\n");
        bCutStatus = false;
    }

    if(bCutStatus == bOldStatus)
    {
        /* ����ǰδ����(�����)�͵�ǰ�ӿ�״̬δ�仯 */
    	return;
    }

	/* ��ǰHDMI�ӿ�״̬���ͱ仯(�Ȱβ�) */
    if (bCutStatus)
    {   /* ��⵽HDMI�ӿڲ��� */
        pthread_t PreviewHandleThdID;

        pstDock->HDMI_preview_flag = true;

        /* ��⵽AppʵʱԤ��δ�� */
        if (false == pstDock->app_preview_flag)
        {
            pthread_t PreviewForntThdID;

            /* ����Ԥ��ǰ���߳� */
            dbg("(main Thread) Create preview_fornt_thd\n\n");
            ret = pthread_create(&PreviewForntThdID, NULL, preview_fornt_thd, (void *)pstDock);
            if (ret)
            {
                int errno;
                char *err_msg = strerror(errno);

                msg("Can't Create thread %s, Error no: %d (%s)\n", "preview_fornt_thd", errno, err_msg);
                return;
            }
            /* �������߳�Ϊ����״̬ */
            pthread_detach(PreviewForntThdID);
        }

        /* ����HDMIԤ���߳� */
        dbg("(main Thread) Create preview_handle_thd\n\n");
        ret = pthread_create(&PreviewHandleThdID, NULL, hdmi_preview_thd, (void *)pstDock);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "preview_fornt_thd", errno, err_msg);
            return;
        }
        /* �������߳�Ϊ����״̬ */
        pthread_detach(PreviewHandleThdID);

        /* �ȴ�������� */
        dbg("(main Thread) delay 1s for waiting hdmi_preview_thd\n\n");
        sleep(1);

        /* ���������ʼԤ������ */
        send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);
    }
    else
    {   /* ��⵽HDMI�ӿڰγ� */
        pstDock->HDMI_preview_flag = false;

        if (false == pstDock->app_preview_flag)
        {
            pstDock->stop_data_flag = false;
        }

        /* �������ֹͣԤ������ */
        send_ball_cmd_pkt(BALL_CMD_STOP_PREVIEW, 0);
    }

    /* ����HDMI�ӿ�״̬ */
    bOldStatus = bCutStatus;
}


