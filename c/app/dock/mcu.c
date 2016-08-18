/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : mcu.c
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��15��
  ����޸�   :
  ��������   : MCU��غ���ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "include/dbg.h"
#include "include/mcu.h"
//#include "include/photo.h"
//include "include/video.h"

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern u8 g_init_debug_switch;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
/* �����͵�MCU����ṹ����� */
static struct mcu_packet m_stMcuCmdPkt;

/* MCU��ʼ������ź��� */
static sem_t m_semMcuInitDone;

/* ����MCU�����ź��� */
static sem_t m_semSendMcuPkt;

#if !MCU_DEBUG_SWITCH
/* ģ��MCU�ϱ��ź��� */
static sem_t m_semSimulateMcuUp;
#endif

/*****************************************************************************
 �� �� ��  : mcu_type_to_str
 ��������  : MCU��������ת���ַ���˵��ʵ�ֺ���
 �������  : u8 ucType
 �������  : ��
 �� �� ֵ  : char
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
char *mcu_type_to_str(u8 ucType)
{
    static char *acStr[] = {"keep alive", "volt query", "mode change",
    "shoot operate", "query volt", "change mode", "change power"};

    return acStr[ucType];
}

/*****************************************************************************
 �� �� ��  : send_mcu_thd
 ��������  : ����MCU�߳�ʵ�ֺ���
 �������  : stConnectInfo *pConnectInfo
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void *send_mcu_thd(void *arg)
{
    int ret;
    ssize_t ulSendBytes;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    int iConnfd = pConnInfo->iConnfd;
    stDock *pstDock = pConnInfo->pstDock;

    dbg("(Send MCU Thread) Enter %s thread\n", "send_mcu_thd");

    /* �����г����˳�ʱ����ѭ�����˳����˳������������� */
    while (true)
    {
        /* �ȴ�����MCU����ʱ���� */
        if (g_init_debug_switch)
            dbg("(Send MCU Thread) Sleep, Waiting Send MCU command\n\n");
        sem_wait(&m_semSendMcuPkt);

        if (g_init_debug_switch)
            dbg("(Send MCU Thread) Send MCU %s command\n", mcu_type_to_str(m_stMcuCmdPkt.type));

#if MCU_DEBUG_SWITCH
        /* ����MCU����� */
        ulSendBytes = send(iConnfd, &m_stMcuCmdPkt, sizeof(m_stMcuCmdPkt), 0);
    	if(ulSendBytes <= 0)
    	{
            int errno;
            char *err_msg = strerror(errno);

    		msg("Send MCU cmd packet failed, Error no: %d (%s)\n", errno, err_msg);
    	}
    	else
    	{
    		dbg("(Send MCU Thread) Send %d bytes MCU cmd packet\n\n", ulSendBytes);
    	}
#else
        ulSendBytes = 8;
        if (MCU_CMD_QUERY_VOLT == m_stMcuCmdPkt.type)
        {
            m_stMcuCmdPkt.type = MCU_ACK_VOLT_QUERY;
            m_stMcuCmdPkt.data = 100;
            sem_post(&m_semSimulateMcuUp);
        }
#endif
        if (ulSendBytes > 0)
        {
            if (g_init_debug_switch)
            {
                dbg("(Send MCU Thread) Send MCU %s command success, send bytes: %d\n", mcu_type_to_str(m_stMcuCmdPkt.type), (u32)ulSendBytes);
                dbg("\t\t\tSend packet magic: %s type: %d data: %d\n", m_stMcuCmdPkt.magic, m_stMcuCmdPkt.type, m_stMcuCmdPkt.data);
            }
        }
    }

    close(iConnfd);
    pthread_exit("Thread send_mcu_thd quit\n");
}

/*****************************************************************************
 �� �� ��  : send_mcu_cmd_pkt
 ��������  : ����MCU�����ʵ�ֺ���
 �������  : u8 ucCmd
             u8 ucData
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��20��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void send_mcu_cmd_pkt(u8 ucCmd, u8 ucData)
{
    /* ��ʼ��MCU����� */
    memset(&m_stMcuCmdPkt, 0, sizeof(m_stMcuCmdPkt));

    /* ������������ */
	strcpy(m_stMcuCmdPkt.magic, "Mcmd");
    m_stMcuCmdPkt.type = ucCmd;
    m_stMcuCmdPkt.data = ucData;

    /* ���ѵȴ��ķ���MCU�̣߳�����MCU���� */
    if (g_init_debug_switch)
        dbg("\t\t\tWake up send_mcu_thd Thread\n");
    sem_post(&m_semSendMcuPkt);
}

/*****************************************************************************
 �� �� ��  : recv_mcu_thd
 ��������  : ����MCU�߳�ʵ�ֺ���
 �������  : stConnectInfo *pConnectInfo
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void *recv_mcu_thd(void *arg)
{
    ssize_t ulRecvBytes;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    stDock *pstDock = pConnInfo->pstDock;
    struct mcu_packet stRecvMcuPkt;

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Enter %s thread\n", "recv_mcu_thd");

    /* ������ֱ�����յ�һ��MCU���� */
    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Blocking, Recieve MCU packet\n\n");
    memset((void *)&stRecvMcuPkt, 0, sizeof(struct mcu_packet));
#if MCU_DEBUG_SWITCH
    ulRecvBytes = recv(pConnInfo->iConnfd, &stRecvMcuPkt, sizeof(struct mcu_packet), 0);
    if (0 == ulRecvBytes)
    {
        msg("(Recv MCU Thread) Client close connect: %d\n", pConnInfo->iConnfd);
        dbg("packet magic: %s type: %d data: %d\n", stRecvMcuPkt.magic, stRecvMcuPkt.type, stRecvMcuPkt.data);
        goto end;
    }
    else if (ulRecvBytes < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Recv MCU Thread) Recieve data failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
#else
    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Simulate recieve MCU %s ack packet\n", mcu_type_to_str(m_stMcuCmdPkt.type));
    ulRecvBytes = 8;
	strcpy(stRecvMcuPkt.magic, "Mack");
    stRecvMcuPkt.type = m_stMcuCmdPkt.type;
    stRecvMcuPkt.data = m_stMcuCmdPkt.data;
#endif
    if (ulRecvBytes > 0)
    {
        if (g_init_debug_switch)
            dbg("(Recv MCU Thread) Recieve MCU %s command success, bytes: %d\n", mcu_type_to_str(stRecvMcuPkt.type), (u32)ulRecvBytes);
        //msg("packet magic: %s type: %d data: %d\n", stRecvMcuPkt.magic, stRecvMcuPkt.type, stRecvMcuPkt.data);
    }

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Check recieve packet magic: %s\n", stRecvMcuPkt.magic);
    /* ��鱨�ĵ�ʶ��key */
    if (0 != strncmp(stRecvMcuPkt.magic, "Mack", MCU_MAX_MAGIC_LEN))
    {
        msg("(Recv MCU Thread) Recieve error magic packet, magic:%s.\n", stRecvMcuPkt.magic);
        goto end;
    }

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Switch by packet type: %s\n", mcu_type_to_str(stRecvMcuPkt.type));
    /* ���ݱ��ĵ����ʹ��� */
    switch (stRecvMcuPkt.type)
    {
        /* ������ѯ�����Ӧ */
        case MCU_ACK_VOLT_QUERY:
            /* ���µ�����Ϣ */
            update_volt_info(pstDock, stRecvMcuPkt.data);

            if (SYS_STATUS_INIT == pstDock->ucSysStatus)
            {
                static u8 ucFirstFlag = 1;
                static u8 ucWaitBallDoneCount = 0;

                /* ��һ���յ�MCU���͵ĵ�����ѯ�����Ӧ */
                if (ucFirstFlag)
                {
                    msg("(Recv MCU Thread) First time recieve MCU volt query packet\n");
                    ucFirstFlag = 0;

                    /* ���ѵȴ������߳� system_init */
                    dbg("(Recv MCU Thread) Wake up main Thread\n");
                    sem_post(&m_semMcuInitDone);

                    /* �ȴ������ʼ������ */
                    dbg("(Recv MCU Thread) delay 2s for waiting ball init done\n\n");
                    sleep(2);
                }

                dbg("(Recv MCU Thread) check adjust file\n");
                /* ���궨�ļ� */
                if (dock_check_adjust_file(pstDock))
                {
                    static u8 ucUnadjustedFlag = 1;

                    /* ��һ�η���δ�궨����������ģʽ�߳� */
                    if (ucUnadjustedFlag)
                    {
                        msg("First time dock is unadjusted, enter factory mode.\n");
                        //[add_code]�˴�����Ӵ�������ģʽ�̵߳Ĵ���
                    }

                    /* δ�궨�����ֱ�ӷ��� �л���Դ������ */
                    send_mcu_cmd_pkt(MCU_CMD_CHANG_POWER_LIGHT, LED_STATUS_OB_4);
                }

                /* ���3������ͬ����־ */
                dbg("(Recv MCU Thread) check Ball date sync flag: %d\n", pstDock->bBallDateSyncFlag);
                if (pstDock->bBallDateSyncFlag)
                {
                    dbg("(Recv MCU Thread) Ball init done\n");

                    /* ������ʼ�����̴��� */
                    dock_follow_up_init(pstDock);
                }
                else
                {
                    ucWaitBallDoneCount++;
                    if (ucWaitBallDoneCount > 3)
                    {
                        /* ����δͬ���������ʼ��ʧ�� */
                        set_sys_status(pstDock, SYS_STATUS_ERROR);
                    }
                }
            }
            break;

        /* ģʽ�л�����֪ͨ */
        case MCU_OP_MODE_CHANG:
            change_shoot_mode(pstDock);
            break;

        /* �������֪ͨ */
        case MCU_OP_SHOOT_OPERA:
            if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
            {
                //photo_opera_handle(pstDock);
            }
            else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
            {
                //video_opera_handle(pstDock);
            }
            else
            {
                msg("Recieve MCU Shoot Mode value: %d error!\n", pstDock->ucShootMode);
            }
            break;

        default:
            msg("Recieve MCU packet type %d error.\n", stRecvMcuPkt.type);
    }

end:
    close(pConnInfo->iConnfd);
    free(pConnInfo);
    return NULL;
}

/*****************************************************************************
 �� �� ��  : mcu_accept_loop
 ��������  : ѭ���ȴ���������ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  : ��
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void mcu_accept_loop(stDock *pstDock)
{
    int ret;

    /* �����˿ڣ����ͬʱ����128������ */
    ret = listen(pstDock->iSock_mm, MAX_CONNECT_NUM);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Accept MCU Thread) Listen %s failed, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        return;
    }

    /* �����г����˳�ʱ����ѭ�����˳����˳������������� */
    while (true)
    {
        int iNewConnfd;
        struct sockaddr stNewClientAddr;
        socklen_t newAddrLen = sizeof(stNewClientAddr);
        stConnectInfo *pConnInfo;
        pthread_t RecvMcuThdID;

#if MCU_DEBUG_SWITCH
        /* ����һ�����ձ��ĵ����� */
        iNewConnfd = accept(pstDock->iSock_mm, (struct sockaddr *)&stNewClientAddr, &newAddrLen);
        if (iNewConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept MCU Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            continue;
        }
#else
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Blocking, Accept MCU connect\n\n");
        iNewConnfd = 23;
        sem_wait(&m_semSimulateMcuUp);
#endif
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Accept a MCU connect: %d\n", iNewConnfd);

        /* ������ձ���������Ϣ */
        pConnInfo = calloc(1, sizeof(stConnectInfo));
        pConnInfo->pstDock = pstDock;
        pConnInfo->iConnfd = iNewConnfd;

        /* ��������MCU�߳̽��ղ��Ҵ����� */
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Create thread recv_mcu_thd\n");
        ret = pthread_create(&RecvMcuThdID, NULL, recv_mcu_thd, (void *)pConnInfo);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept MCU Thread) Can't Create thread %s, Error no: %d (%s)\n", "recv_mcu_thd", errno, err_msg);
            continue;
        }
        /* �������߳�Ϊ����״̬ */
        pthread_detach(RecvMcuThdID);
    }
}

/*****************************************************************************
 �� �� ��  : accept_mcu_thd
 ��������  : ����MCU�����߳�ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void *accept_mcu_thd(void *arg)
{
    int ret, iConnfd;
    char acIpAddr[MAX_IP_ADDR_LEN] = {0};
    stDock *pstDock = (stDock *)arg;
    pthread_t SendMCUThdID;
    pthread_t RecvMCUThdID;
    struct sockaddr stClient;
    socklen_t iClientAddrLen = sizeof(stClient);
    stConnectInfo *pConnInfo;
	struct sockaddr_in stSockMmAddr;

    dbg("(Accept MCU Thread) Enter %s thread\n", "accept_mcu_thd");

    /* ����MCUʹ�õ�Socket: sock_mm */
    pstDock->iSock_mm = socket(AF_INET, SOCK_STREAM, 0);
    if (pstDock->iSock_mm < 0)
    {
        int errno;
        char *err_msg = strerror(errno);
        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        exit(2);
    }
    dbg("(Accept MCU Thread) Create Socket %s OK, Socket: %d\n", "sock_mm", pstDock->iSock_mm);

    /* ��ȡ eth0 �ӿ�IP��ַ */
    if (NULL == get_netif_ip("eth0", acIpAddr))
    {
        msg("Can't get %s IP address, set default IP address: %s\n", "eth0", SOCK_MM_IP_ADDR);
        strncpy(acIpAddr, SOCK_MM_IP_ADDR, MAX_IP_ADDR_LEN);
    }

    /* ���÷���˵�IP��ַ�Ͷ˿ں� */
    memset(&stSockMmAddr, 0, sizeof(stSockMmAddr));
    stSockMmAddr.sin_family = AF_INET;
    inet_pton(AF_INET, acIpAddr, &stSockMmAddr.sin_addr);
    stSockMmAddr.sin_port = htons(SOCK_MM_PORT);

    /* ��IP��ַ�Ͷ˿� */
    ret = bind(pstDock->iSock_mm, (struct sockaddr *)&stSockMmAddr, sizeof(stSockMmAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", acIpAddr, SOCK_MM_PORT, errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Bind to %s:%d success\n", acIpAddr, SOCK_MM_PORT);

#if MCU_DEBUG_SWITCH
    /* �����˿ڣ�������1������ */
    ret = listen(pstDock->iSock_mm, 0);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Listen %s failed, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Listen to %s:%d\n", acIpAddr, SOCK_MM_PORT);

    /* ������ֱ���ڼ������н��ܵ���һ������ */
    iConnfd = accept(pstDock->iSock_mm, (struct sockaddr *)&stClient, &iClientAddrLen);
    if (iConnfd < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Accept a connect: %d\n", iConnfd);
#else
    iConnfd = 123;
#endif

    /* ����������Ϣ */
    pConnInfo = calloc(1, sizeof(stConnectInfo));
    pConnInfo->pstDock = pstDock;
    pConnInfo->iConnfd = iConnfd;

    /* ��������MCU�߳� send_mcu_thd */
    dbg("(Accept MCU Thread) Create thread %s\n\n", "send_mcu_thd");
    ret = pthread_create(&SendMCUThdID, NULL, send_mcu_thd, (void *)pConnInfo);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "send_mcu_thd", errno, err_msg);
        goto end;
    }
    /* �������߳�Ϊ����״̬ */
    pthread_detach(SendMCUThdID);

    /* �ȴ�����MCU�̴߳������  */
    sleep(1);

    /* ���Ͳ�ѯ�������� */
    dbg("(Accept MCU Thread) Send Query Vole command\n");
    send_mcu_cmd_pkt(MCU_CMD_QUERY_VOLT, 0);

    /* ѭ���ȴ��������� */
    mcu_accept_loop(pstDock);

end:
    /* �ر�sock_ac���ر��߳� */
    close(pstDock->iSock_mm);
    pthread_exit("Thread accept_mcu_thd quit\n");
    return NULL;
}

/*****************************************************************************
 �� �� ��  : mcu_init
 ��������  : MCU��ʼ��ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void mcu_init(stDock *pstDock)
{
    int ret;
    pthread_t AcceptMCUThdID;

    /* ��ʼ��MCUģ���ڲ�ʹ�õ��ź��� */
    sem_init(&m_semSendMcuPkt, 0, 0);
    sem_init(&m_semMcuInitDone, 0, 0);
#if !MCU_DEBUG_SWITCH
    sem_init(&m_semSimulateMcuUp, 0, 0);
#endif

    /* ��������MCU�����߳� accept_mcu_thd */
    dbg("(main Thread) Create thread %s\n", "accept_mcu_thd");
    ret = pthread_create(&AcceptMCUThdID, NULL, accept_mcu_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);
        msg("Can't Create thread %s, Error no: %d (%s)\n", "accept_mcu_thd", errno, err_msg);
        return;
    }
    /* �������߳�Ϊ����״̬ */
    pthread_detach(AcceptMCUThdID);

    /* �ȴ�MCU��ʼ����� */
    dbg("(main Thread) Sleep, Waiting MCU init done\n\n");
    sem_wait(&m_semMcuInitDone);

    dbg("(main Thread) Wake up, MCU init done!\n");
}

/*****************************************************************************
 �� �� ��  : set_power_status
 ��������  : ���õ�Դ��״̬ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_power_status(stDock *pstDock)
{
    u8 ucPowerStatus;

    /* ����ϵͳ״̬��ȡ��Դ��״̬ */
    switch (pstDock->ucSysStatus)
    {
        /* ����״̬ */
        case SYS_STATUS_READY:
            if (pstDock->ucChargeFlag)
            {   /* ����� */
                ucPowerStatus = LED_STATUS_OS;
                break;
            }

            if (pstDock->ucVoltCent < 20)
            {   /* �������� */
                ucPowerStatus = LED_STATUS_RS;
            }
            else
            {   /* �������� */
                ucPowerStatus = LED_STATUS_GS;
            }
            break;

        /* ����¼��״̬ */
        case SYS_STATUS_VIDEO:
        /* ��������״̬������¼��״̬��ͬ */
        case SYS_STATUS_PHOTO:
            if (pstDock->ucChargeFlag)
            {   /* ����� */
                ucPowerStatus = LED_STATUS_OB_1;
                break;
            }

            if (pstDock->ucVoltCent < 20)
            {   /* �������� */
                ucPowerStatus = LED_STATUS_RB_1;
            }
            else
            {   /* �������� */
                ucPowerStatus = LED_STATUS_GB_1;
            }
            break;

        /* ����״̬ */
        case SYS_STATUS_ERROR:
            ucPowerStatus = LED_STATUS_RB_4;
            break;
        default:
            msg("System status value:%d error!\n", pstDock->ucSysStatus);
    }

    /* ���� �л���Դ������ */
    send_mcu_cmd_pkt(MCU_CMD_CHANG_POWER_LIGHT, ucPowerStatus);

}

/*****************************************************************************
 �� �� ��  : set_power_status
 ��������  : ����ģʽ��״̬ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_mode_status(stDock *pstDock)
{
    u8 ucModeStatus;

    /* ��������ģʽ��ȡģʽ��״̬ */
    if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
    {
        ucModeStatus = LED_STATUS_GS;
    }
    else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
    {
        ucModeStatus = LED_STATUS_OS;
    }
    else
    {
        msg("Change shoot Mode value: %d error!\n", pstDock->ucShootMode);
        return;
    }

    /* ���� �л�ģʽ������ */
    send_mcu_cmd_pkt(MCU_CMD_CHANG_MODE_LIGHT, ucModeStatus);
}

