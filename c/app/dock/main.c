/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.c
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��14��
  ����޸�   :
  ��������   : ���̺߳���ʵ��
  �����б�   :
              main
  �޸���ʷ   :
  1.��    ��   : 2016��4��14��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * �����Զ���ͷ�ļ�                             *
 *----------------------------------------------*/
#include "include/main.h"
#include "include/dbg.h"
#include "include/mcu.h"
#include "include/app.h"
#include "include/ball.h"
#include "include/stitch.h"
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
static stDock m_stDock;

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : update_volt_info
 ��������  : ���µ�����Ϣʵ�ֺ���
 �������  : stDock *pstDock
             u8 ucVoltInfo
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void update_volt_info(stDock *pstDock, u8 ucVoltInfo)
{
    u8 ucVoltCent;
    u8 ucChargeFlag;

    /* ��7λΪ�����ٷ��� */
    ucVoltCent = ucVoltInfo & 0x7f;

    /* ��1λΪ�Ƿ����־ */
    ucChargeFlag = (ucVoltInfo >> 7) & 0x1;

    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucVoltCent = ucVoltCent;
    pstDock->ucChargeFlag = ucChargeFlag;
    pthread_mutex_unlock(&pstDock->mutex);
}

/*****************************************************************************
 �� �� ��  : set_sys_status
 ��������  : ����ϵͳ״̬ʵ�ֺ���
 �������  : stDock *pstDock
             SysStatusVaule ucStatus
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_sys_status(stDock *pstDock, SysStatusVaule ucStatus)
{
    int ret;

    /* ����ϵͳ״̬����(�������óɳ�ʼ��״̬) */
    if ((!ucStatus) || (ucStatus > SYS_STATUS_ERROR))
    {
        msg("System status value:%d error!\n", ucStatus);
    }

    /* ����ȫ�ֱ���ϵͳ״̬ */
    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucSysStatus = ucStatus;
    pthread_mutex_unlock(&pstDock->mutex);

    /* ���õ�Դ��״̬ */
    set_power_status(pstDock);
}

/*****************************************************************************
 �� �� ��  : change_shoot_mode
 ��������  : �л�����ģʽʵ�ֺ���
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
void change_shoot_mode(stDock *pstDock)
{
    u8 ucNewMode;

    /* ��ǰϵͳ״̬Ϊ����¼��״̬���л�����ģʽʧ�� */
    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        /* ����¼��ʱ�л�����ģʽ���������״̬ */
        set_sys_status(pstDock, SYS_STATUS_ERROR);
        return;
    }

    /* ���ݵ�ǰģʽ���л�����ģʽ */
    if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
    {
        ucNewMode = SHOOT_MODE_VIDEO;
    }
    else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
    {
        ucNewMode = SHOOT_MODE_PHOTO;
    }
    else
    {
        msg("Change shoot Mode value: %d error!\n", pstDock->ucShootMode);
        return;
    }

    /* ����ȫ�ֱ�������ģʽ */
    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucShootMode = ucNewMode;
    pthread_mutex_unlock(&pstDock->mutex);

    /* ����ģʽ��״̬ */
    set_mode_status(pstDock);

	/*������������߳� ,�������ģʽ�л�����*/
    send_ball_cmd_pkt(BALL_CMD_CHANGE_MODE, 0);
}

/*****************************************************************************
 �� �� ��  : dock_check_adjust_file
 ��������  : Dock����궨�ļ�ʵ�ֺ���
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
int dock_check_adjust_file(stDock *pstDock)
{
    int ret = 0;
    int result = 1;

    /* ��Ӧλ�ò��ұ궨�ļ� */

    if (!ret)
    {
        /* ��ѯ���0��ʾ�ҵ��궨�ļ� */
        result = 0;
    }

    return result;
}

/*****************************************************************************
 �� �� ��  : live_init
 ��������  : ֱ����ʼ��ʵ�ֺ���
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
void live_init(stDock *pstDock)
{
    /* ����ֱ�������߳� */

    /* ����ֱ�������߳� */
}

/*****************************************************************************
 �� �� ��  : follow_up_init
 ��������  : Dock�������ʼ������ʵ��
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
void dock_follow_up_init(stDock *pstDock)
{
    /* ֱ����ʼ�� */
    live_init(pstDock);
    dbg("(Recv MCU Thread) live init done\n");

    /* App��ʼ�� */
    app_init(pstDock);
    dbg("(Recv MCU Thread) app init done\n");

    /* ����ģ���ʼ����ɣ��������״̬ */
    set_sys_status(pstDock, SYS_STATUS_READY);
    g_init_debug_switch = DEBUG_OFF;
    dbg("(Recv MCU Thread) All system init done, system status ready!\n\n\n");
}

/*****************************************************************************
 �� �� ��  : get_dock
 ��������  : ��ȡDock�ṹ��ʵ�ֺ���
 �������  : void
 �������  : ��
 �� �� ֵ  : stDock
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��27��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
stDock *get_dock(void)
{
    return &m_stDock;
}

/*****************************************************************************
 �� �� ��  : dock_init
 ��������  : Dock���ʼ��ʵ�ֺ���
 �������  : void
 �������  : stDock *
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��18��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
static void dock_init(void)
{
    int i;
    stDock *pstDock = get_dock();

    memset((void *)pstDock, 0, sizeof(stDock));

    /* Dock��ṹ������ʼ�� */
    pthread_mutex_init(&pstDock->mutex, NULL);

    /* ȫ��״̬��ʼ�� */
    pstDock->ucSysStatus = SYS_STATUS_INIT;
    pstDock->ucShootMode = SHOOT_MODE_PHOTO;

    /* [add_code]���������ʼ�� */

    /* Ԥ����־λ��ʼ�� */
    pstDock->stop_data_flag = STATUS_STOP;      /* Ԥ��ǰ�˿���ֹͣ��־ */
    pstDock->HDMI_preview_flag = STATUS_STOP;
    pstDock->app_preview_flag = STATUS_STOP;

    /* ��ɱ�־��ʼ�� */
    pstDock->decode_done_flag = false;
    pstDock->stitch_done_flag = false;
    pstDock->encode_done_flag = false;
    pstDock->HDMI_send_done_flag = false;
    pstDock->app_send_done_flag = false;

    /* �ź�����ʼ�� */
    sem_init(&pstDock->semRecordTime, 0, 0);
	for (i = 0; i < DECODE_THREAD_NUM; i++)
	{
		sem_init(&pstDock->sem_decode[i], 0, 0);
	}
	sem_init(&pstDock->sem_stitch, 0, 0);
	sem_init(&pstDock->sem_encode, 0, 0);
	sem_init(&pstDock->sem_HDMI_preview, 0, 0);
	sem_init(&pstDock->sem_app_preview, 0, 0);
}

/*****************************************************************************
 �� �� ��  : get_netif_ip
 ��������  : ��ȡ����ӿ�IP��ַʵ�ֺ���
 �������  : char* pIfName
             char *pIpBuf
 �������  : ��
 �� �� ֵ  : char
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��24��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
char * get_netif_ip(char* pIfName, char *pIpBuf)
{
    int ret, iSockfd;
    struct ifreq temp;
    struct sockaddr_in *pstSockAddr;

    sprintf(temp.ifr_name, "%s", pIfName);
    iSockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (iSockfd < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create Socket, Error no: %d (%s)\n", errno, err_msg);
        return NULL;
    }

    ret = ioctl(iSockfd, SIOCGIFADDR, &temp);
    if(ret < 0) {
        msg("Set %s ioctl SIOCGIFADDR failed\n", pIfName);
        close(iSockfd);
        return NULL;
    }

    pstSockAddr = (struct sockaddr_in *)&(temp.ifr_addr);
    strcpy(pIpBuf, inet_ntoa(pstSockAddr->sin_addr));

    close(iSockfd);
    return pIpBuf;
}

/*****************************************************************************
 �� �� ��  : system_init
 ��������  : ϵͳ��ʼ��ʵ�ֺ�����ͬʱ��Ϊ��ʼ�������߳�ʹ��
 �������  : void *arg
 �������  : ��
 �� �� ֵ  : void *
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void *system_init(void *arg)
{
    stDock *pstDock = get_dock();

    /* Dock��ʼ�� */
    dock_init();

    /* MCU��ʼ�� */
    mcu_init(pstDock);

    /* �����ʼ�� */
    ball_init(pstDock);

    while (true)
    {
        /* �����ȴ�5s */
        sleep(5);

        /* ����MCU������(��ѯ��������) */
        send_mcu_cmd_pkt(MCU_CMD_QUERY_VOLT, 0);

        /* ������������� */
        send_ball_cmd_pkt(BALL_CMD_KEEP_ALIVE, 0);

        /* HDMI�ӿ��Ȱβ��� */
        hdmi_status_check(pstDock);
    }
}

/*****************************************************************************
 �� �� ��  : main
 ��������  : ���߳�ʵ�ֺ���
 �������  : int argc
             char **argv
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��14��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
int main(int argc, char **argv)
{
    /* ����ϵͳ��־ */
    //set_system_log();

    /* �򿪵��Կ���̨ */
    dbg_console();

    /* ϵͳ��ʼ�� */
    system_init(NULL);
}

