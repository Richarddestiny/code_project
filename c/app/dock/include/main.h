/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : main.h
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��15��
  ����޸�   :
  ��������   : main.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��15��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

/*----------------------------------------------*
 * ������׼ͷ�ļ�                               *
 *----------------------------------------------*/
#include <stdio.h>
#include <memory.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <net/if.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ������������                                 *
 *----------------------------------------------*/
typedef unsigned char       u8;     /* �޷���8λ���� */
typedef unsigned short      u16;    /* �޷���16λ���� */
typedef unsigned int        u32;    /* �޷���32λ���� */
typedef unsigned long long  u64;    /* �޷���64λ���� */

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/* ���IP��ַ�ַ������� */
#define MAX_IP_ADDR_LEN         16

/* ���ͬʱ������ */
#define MAX_CONNECT_NUM         128

/* ����ͷ��������·�� */
#define CAMERA_DATA_INPUT_NUM   4

/* �����߳���Ŀ */
#define DECODE_THREAD_NUM       4

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
#define SHOOT_MODE_PHOTO    0 /* ����ģʽ */
#define SHOOT_MODE_VIDEO    1 /* ¼��ģʽ */

#define STATUS_STOP         0 /* ֹͣ״̬ */
#define STATUS_DOING        1 /* ���ڽ���״̬ */

/*----------------------------------------------*
 * ö�����Ͷ���                                 *
 *----------------------------------------------*/
typedef enum {
	SYS_STATUS_INIT  = 0,   /* ��ʼ��״̬ */
	SYS_STATUS_READY = 1,   /* ����״̬ */
	SYS_STATUS_VIDEO = 2,   /* ����¼��״̬ */
	SYS_STATUS_PHOTO = 3,   /* ��������״̬ */
	SYS_STATUS_ERROR = 4,   /* ����״̬ */
} SysStatusVaule;

/*----------------------------------------------*
 * �ṹ�����Ͷ���                               *
 *----------------------------------------------*/

 /* AppʵʱԤ��֡ͷ */
typedef struct {
    u32 uiID;       /* ֡ID */
    u32 uiLen;      /* ֡����: ��ÿ�ζ����ܲ�ͬ�� */
    u32 uiInterval; /* I֡����I֡��ʶ��0��I֡��ʶ��1~6��I֡��ࣩ */
} frame_head;

/* Dock�� */
typedef struct {
    pthread_mutex_t mutex;  /* ���ṹ���ڵ�ֵ�����иĶ��ĵط����� */

    /* Dock��״̬��ȫ�ֱ��� */
    u8 ucSysStatus;         /* ϵͳ״̬: ȡֵ��Χ�� SysStatusVaule */
    u8 ucShootMode;         /* ����ģʽ��0��¼��ģʽ��1������ģʽ�� */
    u8 ucErrorCode;         /* �����루0���ɹ�����0��ִ�д����룩 */
    u8 ucVoltCent;          /* �����ٷ�����0~100�� 20��ʾ��ǰ����20%�� */
    u8 ucChargeFlag;        /* �Ƿ��ڳ���־��0��δ��磻1������У� */
    u8 ucExpCompValue;      /* �عⲹ��ֵ����2EV����13����λ */
    u8 ucWhiteBalMode;      /* ��ƽ��ģʽ���ṩ7��ģʽѡ�� */
    u8 ucResolution;        /* �ֱ��ʣ�0��1440*720��1��2048*1024�� */

    /* MCU���ȫ�ֱ��� */
    int iSock_mm;

    /* ������ȫ�ֱ��� */
    bool bBallDateSyncFlag;         /* �������ͬ����־(��ʼ����ɱ�־) */
    pthread_t BallDataThread;

    /* App���ȫ�ֱ��� */
    u32 uiRecordTime;       /* ��¼��ʼ����ǰʱ������� */
	sem_t semRecordTime;    /* ��ѯʱ������ �����߳� app_cmd_handle_thd �ȴ����ź������� ball_ctrl_thd ���� */

    /* ������ֹͣ��־ */
    bool stop_data_flag;

    /* ������ݽ��� -> ���� ȫ�ֱ��� */
	pthread_t decodeThread[DECODE_THREAD_NUM];  /* ��·�����̵߳�ID */
	sem_t sem_recv_photo_data;                  /* �ȴ��������������ź��� */

    /* ���� -> ƴ�� ȫ�ֱ��� */
	sem_t sem_decode[DECODE_THREAD_NUM];                /* ��·�����̵߳ȴ��ź��������յ����ݺ��� */
	bool decode_done_flag;                              /* ��ɵ�һ֡���ݽ�����ɱ�־ */
	char *pDecodeToStitchBuffer[CAMERA_DATA_INPUT_NUM]; /* �����̸߳�ƴ���̵߳���·������������ */

    /* ƴ�� -> ���� ȫ�ֱ��� */
	sem_t sem_stitch;               /* ƴ���̵߳ȴ����ź�����������ɺ��� */
	bool stitch_done_flag;          /* ƴ����ɱ�־ */
    char *pStitchToEncodeBuffer;    /* ƴ���̸߳������̵߳����� */

    /* ���� -> FFmpeg ȫ�ֱ��� */
    sem_t sem_encode;               /* �����̵߳ȴ����ź�����ƴ����ɺ��� */
    bool encode_done_flag;          /* ������ɱ�־ */

	/* ƴ�� -> HDMIԤ�� ȫ�ֱ��� */
    bool HDMI_preview_flag;         /* HDMIԤ����ʼ�ͽ�����־ */
	sem_t sem_HDMI_preview;         /* HDMIԤ���ȴ����ź�������ƴ����ɺ��� */
    bool HDMI_send_done_flag;       /* HDMIԤ�����ݷ�����ɱ�־ */
    char *pHdmiPreviewBuffer;       /* ƴ���̸߳�HDMI��Ԥ������ */

    /* ���� -> AppԤ�� ȫ�ֱ��� */
    bool app_preview_flag;          /* AppԤ����־��STATUS_STOP��ֹͣԤ����STATUS_DOING������Ԥ���� */
	sem_t sem_app_preview;          /* AppԤ���ȴ����ź�����������ɺ��� */
    bool app_send_done_flag;        /* AppԤ�����ݷ�����ɱ�־ */
    char *pAppPreviewBuffer;        /* �����̸߳�App��Ԥ������ */
    frame_head stAppFrameHead;      /* AppԤ������֡ͷ */

    /* ¼��ƴ�� ȫ�ֱ��� */
    bool video_stitch_flag;         /* ¼��ƴ�ӱ�־ */
    u32 uiVideoSecond;              /* ��ǰƴ�ӵ�¼���ļ��ܹ�ʱ�䳤�ȣ���λ���� */
    u32 uiFrameCount;               /* ��ǰƴ�ӵ�¼���ļ��Ѿ�ƴ����ɵ�֡�� */

} stDock;

/* ������Ϣ�ṹ�� */
typedef struct
{
    stDock *pstDock;
    int     iConnfd;
} stConnectInfo;

/*****************************************************************************
 �� �� ��  : set_sys_status
 ��������  : ����ϵͳ״̬ʵ�ֺ���
*****************************************************************************/
extern void set_sys_status(stDock *pstDock, SysStatusVaule ucStatus);

/*****************************************************************************
 �� �� ��  : get_netif_ip
 ��������  : ��ȡ����ӿ�IP��ַʵ�ֺ���
*****************************************************************************/
extern char * get_netif_ip(char* pIfName, char *pIpBuf);

/*****************************************************************************
 �� �� ��  : get_dock
 ��������  : ��ȡDock�ṹ��ʵ�ֺ���
*****************************************************************************/
extern stDock *get_dock(void);

/*****************************************************************************
 �� �� ��  : system_init
 ��������  : ϵͳ��ʼ��ʵ�ֺ�����ͬʱ��Ϊ��ʼ�������߳�ʹ��
*****************************************************************************/
extern void *system_init(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */

