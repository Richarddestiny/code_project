/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : app.c
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��15��
  ����޸�   :
  ��������   : App��غ���ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "include/app.h"

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
/* App�����б� */
static app_cmd m_app_cmd_list[MAX_APP_CMD_NUM + 1] = {
    {APP_CMD_QUERY_VOLT,      	APP_ACK_QUERY_VOLT,     query_volt_handle},         /* ��ѯ�������� */
    {APP_CMD_QUERY_DATE,      	APP_ACK_QUERY_DATE,     query_date_handle},         /* ��ѯ�������� */
    {APP_CMD_QUERY_STATUS,    	APP_ACK_QUERY_STATUS,   query_status_handle},       /* ��ѯ״̬���� */
    {APP_CMD_QUERY_PARA,      	APP_ACK_QUERY_PARA,     query_para_handle},         /* ��ѯ�������� */
    {APP_CMD_QUERY_CAPACITY,  	APP_ACK_QUERY_CAPACITY, query_capacity_handle},     /* ��ѯ�������� */
    {APP_CMD_QUERY_TIME,      	APP_ACK_QUERY_TIME,     query_time_handle},         /* ��ѯʱ������ */
    {APP_CMD_QUERY_PROG,      	APP_ACK_QUERY_PROG,     query_prog_handle},         /* ��ѯ�������� */
    {APP_CMD_CHANGE_MODE,     	APP_ACK_QUERY_STATUS,   change_mode_handle},        /* ģʽ�л����� */
    {APP_CMD_SHOOT_OPERATE,   	APP_ACK_QUERY_STATUS,   shoot_operare_handle},      /* ����������� */
    {APP_CMD_PREVIEW_OPERATE, 	APP_ACK_QUERY_STATUS,   preview_operare_handle},    /* Ԥ���������� */
    {APP_CMD_SPLICE_VIDEO,    	APP_ACK_QUERY_PROG,     splice_video_handle},       /* ¼��ƴ������ */
    {APP_CMD_SET_PARA,          APP_ACK_QUERY_PARA,     set_para_handle},           /* ���ò������� */
    {0,                         0,                      NULL},
};

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define APP_PREVIEW_DEBUG       1

/*****************************************************************************
 �� �� ��  : app_preview_thd
 ��������  : AppԤ���߳�ʵ�ֺ���
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void *
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void *app_preview_thd(void *arg)
{
    int ret, iSock_ad, iConnfd;
    ssize_t ulSendBytes;
    stDock *pstDock = (stDock *)arg;
    struct sockaddr_in stSockAdAddr;
    struct sockaddr stClient;
    socklen_t iClientAddrLen = sizeof(stClient);

    dbg("(App Preview Thread) Enter %s\n", __func__);

    /* ����App����ʹ�õ�Socket: sock_ad */
    iSock_ad = socket(AF_INET, SOCK_STREAM, 0);
    if (iSock_ad < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_ad", errno, err_msg);
        pthread_exit("Thread app_preview_thd quit\ns");
        return NULL;
    }
    dbg("(App Preview Thread) Create Socket %s OK, Socket: %d\n", "sock_ad", iSock_ad);

    /* ���÷���˵�IP��ַ�Ͷ˿ں� */
    memset(&stSockAdAddr, 0, sizeof(stSockAdAddr));
    stSockAdAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SOCK_AD_IP_ADDR, &stSockAdAddr.sin_addr);
    stSockAdAddr.sin_port = htons(SOCK_AD_PORT);

    /* ��IP��ַ�Ͷ˿� */
    ret = bind(iSock_ad, (struct sockaddr *)&stSockAdAddr, sizeof(stSockAdAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", SOCK_AD_IP_ADDR, SOCK_AD_PORT, errno, err_msg);
        goto end;
    }
    dbg("(App Preview Thread) Bind to %s:%d success\n", SOCK_AD_IP_ADDR, SOCK_AD_PORT);

    if (g_app_preview_debug_switch)
    {
        /* �����˿ڣ�������1������ */
        ret = listen(iSock_ad, 0);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(App Preview Thread) Listen %s failed, Error no: %d (%s)\n", "sock_ad", errno, err_msg);
            goto end;
        }

        /* ������ֱ���ڼ������н��ܵ�һ������ */
        dbg("(Accept App Thread) Blocking, Accept App data connect\n\n");
        iConnfd = accept(iSock_ad, (struct sockaddr *)&stClient, &iClientAddrLen);
        if (iConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(App Preview Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            goto end;
        }
        dbg("(App Preview Thread) Accept a connect: %d\n", iConnfd);
    }


    while (true)
    {
        /* ����AppԤ�����ݷ�����ɱ�־ */
        pstDock->app_send_done_flag = true;

        /* ˯�ߣ�����Ԥ������ʱ, �ɱ����߳� encode_handle_thd ���� */
        //dbg("(App Preview Thread) Sleep, Waiting Send preview data\n\n");
        sem_wait(&pstDock->sem_app_preview);

        //dbg("(App Preview Thread) wake up, Send preview data\n");

        /* ����Ԥ��֡ͷ */
       // if (g_app_preview_debug_switch)
       //     ulSendBytes = sizeof(frame_head);
       // else
	        ulSendBytes = send(iConnfd, &pstDock->stAppFrameHead, sizeof(frame_head), 0);
    	if(ulSendBytes <= 0)
    	{
	        int errno;
            char *err_msg = strerror(errno);

    		msg("Send app preview Frame Head failed, Error no: %d (%s)\n", errno, err_msg);
    		continue;
    	}
    	else
    	{
    		dbg("(App Preview Thread) Send %d bytes app preview Frame Head\n", ulSendBytes);
    		dbg("\t\t\tFrame ID: %u len: %u Interval: %u\n", pstDock->stAppFrameHead.uiID,
    		    pstDock->stAppFrameHead.uiLen, pstDock->stAppFrameHead.uiInterval);
    	}

        /* ����Ԥ������ */
        if (g_app_preview_debug_switch)
            ulSendBytes = pstDock->stAppFrameHead.uiLen;
        else
    	    ulSendBytes = send(iConnfd, pstDock->pAppPreviewBuffer, pstDock->stAppFrameHead.uiLen, 0);
    	if(ulSendBytes <= 0)
    	{
	        int errno;
            char *err_msg = strerror(errno);

    		msg("(App Preview Thread) Send app preview Frame data failed, Error no: %d (%s)\n", errno, err_msg);
    		continue;
    	}
    	else
    	{
    		dbg("Send %d bytes app preview Frame data\n\n", ulSendBytes);
    	}
    }

end:
    /* �ر�sock_ad���ر��߳� */
    close(iSock_ad);
    pthread_exit("Thread app_preview_thd quit\n");
    return NULL;
}

/*****************************************************************************
 �� �� ��  : app_type_to_str
 ��������  : App����ͻظ���������ת���ַ���˵��ʵ�ֺ���
 �������  : u8 ucType
 �������  : ��
 �� �� ֵ  : char
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��23��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
char *app_type_to_str(u8 ucType)
{
    static char *acStr[] = {"unknown", "query volt", "query date", "query status",
    "query parameter", "query capacity", "query time", "query progress", "change mode",
    "shoot operate", "preview operate", "splice video", "set parameter"};

    return acStr[ucType];
}

/*****************************************************************************
 �� �� ��  : query_volt_handle
 ��������  : App��ѯ���������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : u64
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��25��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_volt_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ��Ϊ���߳��Ѿ��ڲ��ϲ�ѯ�������������ȡ������Ϣ���� */
    unOutputData.acData[0] = pstDock->ucVoltCent;   /* �����ٷ�����0~100�� 20��ʾ��ǰ����20%�� */
    unOutputData.acData[1] = pstDock->ucChargeFlag; /* �Ƿ��ڳ���־��0��δ��磻1������У� */

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_date_handle
 ��������  : App��ѯ���������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : u64
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��25��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_date_handle(stDock *pstDock, u64 ullInputData)
{
    u32 uiTimeStamp;
    time_t CurrTime;
    unData unOutputData;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    uiTimeStamp = ((unData)ullInputData).aiData[0];

    /* ����ʱ�����Ϊ0����ʾ�������ʱ�䣬����0 */
    if (uiTimeStamp)
    {
		/* �������ڲ������ͬ������������� */
	    send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, uiTimeStamp);
	    return 0;
    }

    /* ����ʱ���Ϊ0����ʾ��ȡDock�嵱ǰʱ�� */
    time(&CurrTime);
    unOutputData.aiData[0] = CurrTime;

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_status_handle
 ��������  : App��ѯ״̬�����ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : static
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��26��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_status_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ��ȡϵͳ״̬��[0]����ģʽ��[1]¼��״̬��[2]Ԥ��״̬��[3]������ */
    unOutputData.acData[0] = pstDock->ucShootMode;
    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        unOutputData.acData[1] = STATUS_DOING;
    }
    else if (SYS_STATUS_READY == pstDock->ucSysStatus)
    {
        unOutputData.acData[1] = STATUS_STOP;
    }

    unOutputData.acData[2] = pstDock->app_preview_flag;
    unOutputData.acData[3] = pstDock->ucErrorCode;

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_para_handle
 ��������  : App��ѯ���������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : static
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��26��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_para_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ��ȡ���������[0]�عⲹ��ֵ��[1]��ƽ��ģʽ��[2]�ֱ��� */
    unOutputData.aiData[0] = pstDock->ucExpCompValue;
    unOutputData.aiData[1] = pstDock->ucWhiteBalMode;
    unOutputData.aiData[2] = pstDock->ucResolution;

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_capacity_handle
 ��������  : App��ѯ���������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : static
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��26��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_capacity_handle(stDock *pstDock, u64 ullInputData)
{
    u16 usAvailCapacity;
    u64 ullBlockNum;
    u64 ullAvailableDisk;
    unData unOutputData;
    struct statfs diskInfo;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ��ȡʣ������ */
    statfs("/usr/data/", &diskInfo);
    ullBlockNum = diskInfo.f_bsize;        //ÿ��block��������ֽ���
    ullAvailableDisk = diskInfo.f_bavail * ullBlockNum;   //���ÿռ��С
    unOutputData.asData[0] = ullAvailableDisk >> 20;

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_time_handle
 ��������  : App��ѯʱ�������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��30��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_time_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;
    u16 usAvailCapacity;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ��ȡʣ����ÿռ䣬��λ��MB */
    unOutputData.ullData = query_capacity_handle(pstDock, 0);
    usAvailCapacity = unOutputData.asData[0];

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ʣ��¼��ʱ����㹫ʽ */
    /* remain_time = avail_capacity / (fps * frame_max_size * 4) */
    /* FPSΪ25֡/�룬���֡��Сȡ����ֵ50KB������ʣ����ÿռ���Ҫת����KB */
    unOutputData.aiData[1] = ((u32)usAvailCapacity << 10) / (25 * 50 * 4);

	/* ���Ͷ�ȡ¼��ʱ������ */
    send_ball_cmd_pkt(BALL_CMD_READ_VIDEO_TIME, 0);

    /* ˯�ߵȴ������߳� app_cmd_handle_thd ���� */
	sem_wait(&pstDock->semRecordTime);

    unOutputData.aiData[0] = pstDock->uiRecordTime;

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : query_prog_handle
 ��������  :  App��ѯ���������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��30��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 query_prog_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* ��ʼ��������� */
    unOutputData.ullData = 0;

    /* ƴ�ӽ��Ȱٷ������㹫ʽ */
    /* ƴ�ӽ��Ȱٷ��� = (current_frame_count * 100) / (fps * video_sec) */
    unOutputData.acData[0] = (pstDock->uiFrameCount * 100) / (25 * pstDock->uiVideoSecond);

    return unOutputData.ullData;
}

/*****************************************************************************
 �� �� ��  : change_mode_handle
 ��������  : Appģʽ�л������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��30��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 change_mode_handle(stDock *pstDock, u64 ullInputData)
{
    /* �л�����ģʽ���� */
    change_shoot_mode(pstDock);

    /* ����״̬��ѯ�ظ� */
    return query_status_handle(pstDock, 0);;
}

/*****************************************************************************
 �� �� ��  : shoot_operare_handle
 ��������  : App������������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��30��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 shoot_operare_handle(stDock *pstDock, u64 ullInputData)
{
    if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
    {
        photo_opera_handle(pstDock);
    }
    else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
    {
        video_opera_handle(pstDock);
    }
    else
    {
        msg("Recieve MCU Shoot Mode value: %d error!\n", pstDock->ucShootMode);
    }

    /* ����״̬��ѯ�ظ� */
    return query_status_handle(pstDock, 0);;
}

/*****************************************************************************
 �� �� ��  : preview_operare_handle
 ��������  : AppԤ�����������ʵ�ֺ���
 �������  : stDock *pstDock
             u64 ullInputData
 �������  : ��
 �� �� ֵ  : static
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��25��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
u64 preview_operare_handle(stDock *pstDock, u64 ullInputData)
{
    int ret;
    unData unOutputData;

    dbg("(App cmd handle Thread) App preview operare \n");

    if (STATUS_STOP == pstDock->app_preview_flag)
    {   /* AppԤ��ֹͣʱ */
        pthread_t EncodeHandleThdID;

        /* ��AppԤ�� */
        pstDock->app_preview_flag = STATUS_DOING;

        if (DEBUG_ON == g_app_preview_debug_switch)
        {
            pthread_t BallDataThdID;

            dbg("(App cmd handle Thread) Open App preview\n");

            pstDock->stop_data_flag = STATUS_DOING;
            pstDock->app_send_done_flag = true;

            /* ������������߳� */
            dbg("(App cmd handle Thread) Create ball_data_thd\n\n");
            ret = pthread_create(&BallDataThdID, NULL, ball_data_thd, (void *)pstDock);
            if (ret)
            {
                int errno;
                char *err_msg = strerror(errno);

                msg("Can't Create thread %s, Error no: %d (%s)\n", "ball_data_thd", errno, err_msg);
                return ret;
            }
            /* �������߳�Ϊ����״̬ */
            pthread_detach(BallDataThdID);

            /* ���Ϳ�ʼԤ�������� */
            dbg("(App cmd handle Thread) Send %d command to Ball\n", BALL_CMD_START_PREVIEW);
            send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);

            return 0;
        }

        /* ��⵽HDMIԤ��δ�� */
        if (false == pstDock->HDMI_preview_flag)
        {
            pthread_t PreviewForntThdID;

            /* ����Ԥ��ǰ���߳� */
            dbg("(App cmd handle Thread) Create preview_fornt_thd\n\n");
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

        /* �������봦���߳� */
        dbg("(App cmd handle Thread) Create encode_handle_thd\n\n");
        ret = pthread_create(&EncodeHandleThdID, NULL, encode_handle_thd, (void *)pstDock);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "encode_handle_thd", errno, err_msg);
            return;
        }
        /* �������߳�Ϊ����״̬ */
        pthread_detach(EncodeHandleThdID);

        /* �ȴ�������� */
        dbg("(App cmd handle Thread) delay 1s for waiting hdmi_preview_thd\n\n");
        sleep(1);

        /* ���������ʼԤ������ */
        send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);
    }
    else if (STATUS_DOING == pstDock->app_preview_flag)
    {   /* AppԤ�����ڽ���ʱ */

        /* ֹͣAppԤ�� */
        pstDock->app_preview_flag = STATUS_STOP;

        /* ��⵽HDMIԤ��δ�� */
        if (STATUS_STOP == pstDock->HDMI_preview_flag)
        {
            pstDock->stop_data_flag = STATUS_STOP;
        }

        /* ����ֹͣԤ�������� */
        dbg("(App cmd handle Thread) Send %d command to Ball\n", BALL_CMD_STOP_PREVIEW);
        send_ball_cmd_pkt(BALL_CMD_STOP_PREVIEW, 0);
    }

    if (DEBUG_ON == g_app_preview_debug_switch)
        return 0;
ack:
    /* ��ȡϵͳ״̬��[0]����ģʽ��[1]¼��״̬��[2]Ԥ��״̬��[3]������ */
    unOutputData.aiData[0] = pstDock->ucShootMode;

    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        unOutputData.aiData[1] = STATUS_DOING;
    }
    else if (SYS_STATUS_READY == pstDock->ucSysStatus)
    {
        unOutputData.aiData[1] = STATUS_STOP;
    }
    else
    {
        /* [add_code]������Ӵ����� */
    }

    unOutputData.aiData[2] = pstDock->app_preview_flag;
    unOutputData.aiData[3] = pstDock->ucErrorCode;

    return unOutputData.ullData;
}

u64 splice_video_handle(stDock *pstDock, u64 ullInputData)
{

}

u64 set_para_handle(stDock *pstDock, u64 ullInputData)
{

}

/*****************************************************************************
 �� �� ��  : app_cmd_handle_thd
 ��������  : App������߳�ʵ�ֺ���
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void *
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void *app_cmd_handle_thd(void *arg)
{
    int i;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    stDock *pstDock = pConnInfo->pstDock;
    ssize_t ulRecvBytes, ulSendBytes;
    struct app_packet stRecvAppPkt, stSendAppPkt;

    dbg("(App cmd handle Thread) Enter %s\n", __func__);

    /* ����App���� */
    memset((void *)&stRecvAppPkt, 0, sizeof(struct app_packet));
    ulRecvBytes = recv(pConnInfo->iConnfd, &stRecvAppPkt, sizeof(struct app_packet), 0);
    if (0 == ulRecvBytes)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(App cmd handle Thread) Client close connect, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
    else if (ulRecvBytes < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(App cmd handle Thread) Recieve data failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }

    dbg("(App cmd handle Thread) Recv App %s command success, recieved %d bytes\n", app_type_to_str(stRecvAppPkt.type), (u32)ulRecvBytes);
    dbg("Recv packet, Magic: %s Type: %d, Data: %lld\n", stRecvAppPkt.magic, stRecvAppPkt.type, stRecvAppPkt.data.ullData);

    /* ��鱨�ĵ�ʶ��key */
    if (strncmp(stRecvAppPkt.magic, "Acmd", APP_MAX_MAGIC_LEN))
    {
        msg("(App cmd handle Thread) Recieve error magic packet, magic: %s\n", stRecvAppPkt.magic);
        goto end;
    }

    /* ���ݽ��յı��ĵ����ʹ���ͬʱ���û�Ӧ�������� */
    for (i = 0; i < MAX_APP_CMD_NUM; i++) {
        /* ������������ƥ�� */
        if (m_app_cmd_list[i].cmd_type != stRecvAppPkt.type)
            continue;

        /* ������������ */
        strcpy(stSendAppPkt.magic, "Aack");
        stSendAppPkt.type = m_app_cmd_list[i].ack_type;

        /* ���ö�Ӧ���������ִ��������ظ����� */
        stSendAppPkt.data.ullData = m_app_cmd_list[i].cmd_func(pstDock, stRecvAppPkt.data.ullData);
    }

    /* ����App�ظ����� */
    ulSendBytes = send(pConnInfo->iConnfd, &stSendAppPkt, sizeof(stSendAppPkt), 0);
	if(ulSendBytes <= 0)
	{
        int errno;
        char *err_msg = strerror(errno);

		msg("Send App ack packet failed, Error no: %d (%s)\n", errno, err_msg);
	}
	else
	{
		dbg("(App cmd handle Thread) Send App ack success, Send %d bytes\n\n", ulSendBytes);
		dbg("\tpacket magic: %s\n\ttype: %s\n", stSendAppPkt.magic, app_type_to_str(stSendAppPkt.type));
	}

end:
    close(pConnInfo->iConnfd);
    free(pConnInfo);
    return NULL;
}

/*****************************************************************************
 �� �� ��  : accept_app_thd
 ��������  : ����App�����߳�ʵ�ֺ���
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void *
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void *accept_app_thd(void *arg)
{
    int ret, iSock_ac;
    stDock *pstDock = (stDock *)arg;
    struct sockaddr_in stSockAcAddr;

    dbg("(Accept App Thread) Enter %s\n", __func__);

    /* ����App����ʹ�õ�Socket: sock_ac */
    iSock_ac = socket(AF_INET, SOCK_STREAM, 0);
    if (iSock_ac < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_ac", errno, err_msg);
        pthread_exit("Thread accept_app_thd quit\ns");
        return NULL;
    }
    dbg("(Accept App Thread) Create Socket %s OK, Socket: %d\n", "sock_ac", iSock_ac);

    /* ���÷���˵�IP��ַ�Ͷ˿ں� */
    memset(&stSockAcAddr, 0, sizeof(stSockAcAddr));
    stSockAcAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SOCK_AC_IP_ADDR, &stSockAcAddr.sin_addr);
    stSockAcAddr.sin_port = htons(SOCK_AC_PORT);

    /* ��IP��ַ�Ͷ˿� */
    ret = bind(iSock_ac, (struct sockaddr *)&stSockAcAddr, sizeof(stSockAcAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", SOCK_AC_IP_ADDR, SOCK_AC_PORT, errno, err_msg);
        goto end;
    }
    dbg("(Accept App Thread) Bind to %s:%d success\n", SOCK_AC_IP_ADDR, SOCK_AC_PORT);

    /* �����˿ڣ����ͬʱ����128������ */
    ret = listen(iSock_ac, 128);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Accept App Thread) Listen %s failed, Error no: %d (%s)\n", "sock_ac", errno, err_msg);
        goto end;
    }

    /* �����г����˳�ʱ����ѭ�����˳����˳������������� */
    while (true)
    {
        int iNewConnfd;
        struct sockaddr stNewClientAddr;
        socklen_t iClientAddrLen = sizeof(stNewClientAddr);
        pthread_t AppCmdThdID;
        stConnectInfo *pConnInfo;

        /* ������ֱ���ڼ������н��ܵ�һ������ */
        dbg("(Accept App Thread) Blocking, Accept App command connect\n\n");
        iNewConnfd = accept(iSock_ac, (struct sockaddr *)&stNewClientAddr, &iClientAddrLen);
        if (iNewConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept App Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            continue;
        }
        dbg("(Accept App Thread) Accept a connect: %d\n", iNewConnfd);

        /* ������ձ���������Ϣ */
        pConnInfo = calloc(1, sizeof(stConnectInfo));
        pConnInfo->pstDock = pstDock;
        pConnInfo->iConnfd = iNewConnfd;

        /* ����App������߳̽��ղ��Ҵ����� */
        dbg("(Accept App Thread) Create thread app_cmd_handle_thd \n");
        ret = pthread_create(&AppCmdThdID, NULL, app_cmd_handle_thd, (void *)pConnInfo);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "app_cmd_handle_thd", errno, err_msg);
            continue;
        }
        /* �������߳�Ϊ����״̬ */
        pthread_detach(AppCmdThdID);
    }

end:
    /* �ر�sock_ac���ر��߳� */
    close(iSock_ac);
    pthread_exit("Thread accept_app_thd quit\n");
    return NULL;
}

/*****************************************************************************
 �� �� ��  : app_init
 ��������  : App��ʼ��ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��20��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void app_init(stDock *pstDock)
{
    int ret;
    pthread_t AppPreviewThdID;
    pthread_t AcceptAppThdID;

    /* ����AppԤ���߳� app_preview_thd */
    dbg("(Recv MCU Thread) Create thread app_preview_thd\n\n");
    ret = pthread_create(&AppPreviewThdID, NULL, app_preview_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "app_preview_thd", errno, err_msg);
        return;
    }
    /* �������߳�Ϊ����״̬ */
    pthread_detach(AppPreviewThdID);

    /* ��������App�����߳� accept_app_thd */
    dbg("(Recv MCU Thread) Create thread accept_mcu_thd\n\n");
    ret = pthread_create(&AcceptAppThdID, NULL, accept_app_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "accept_mcu_thd", errno, err_msg);
        return;
    }
    /* �������߳�Ϊ����״̬ */
    pthread_detach(AcceptAppThdID);
}

