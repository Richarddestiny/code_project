/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : app.h
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��19��
  ����޸�   :
  ��������   : app.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��19��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __APP_H__
#define __APP_H__

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <memory.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/statfs.h>
#include <netinet/in.h>
#include "main.h"
#include "dbg.h"
#include "ball.h"
#include "preview.h"
#include "encode.h"

#ifdef  __cplusplus
extern "C" {
#endif


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

 /* App����ʹ�õ�Socket: sock_ad IP��ַ�Ͷ˿� */
#define SOCK_AD_IP_ADDR             "192.168.3.1"
#define SOCK_AD_PORT                9999

 /* App����ʹ�õ�Socket: sock_ac IP��ַ�Ͷ˿� */
#define SOCK_AC_IP_ADDR             "192.168.3.1"
#define SOCK_AC_PORT                8888

#define APP_MAX_MAGIC_LEN           7   /* App�������ħ���֣�ʶ��key������ */

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

/* App���͸�Dock�����������£�*/
#define APP_CMD_QUERY_VOLT      1   /* ��ѯ�������� */
#define APP_CMD_QUERY_DATE      2   /* ��ѯ�������� */
#define APP_CMD_QUERY_STATUS    3   /* ��ѯ״̬���� */
#define APP_CMD_QUERY_PARA      4   /* ��ѯ�������� */
#define APP_CMD_QUERY_CAPACITY  5   /* ��ѯ�������� */
#define APP_CMD_QUERY_TIME      6   /* ��ѯʱ������ */
#define APP_CMD_QUERY_PROG      7   /* ��ѯ�������� */
#define APP_CMD_CHANGE_MODE     8   /* ģʽ�л����� */
#define APP_CMD_SHOOT_OPERATE   9   /* ����������� */
#define APP_CMD_PREVIEW_OPERATE 10  /* Ԥ���������� */
#define APP_CMD_SPLICE_VIDEO    11  /* ¼��ƴ������ */
#define APP_CMD_SET_PARA        12  /* ���ò������� */

/* ���App������ */
#define MAX_APP_CMD_NUM         12

/* Dock�巢�͸�App�Ļ�Ӧ�������£�*/
#define APP_ACK_QUERY_VOLT      1   /* ������ѯ�ظ� */
#define APP_ACK_QUERY_DATE      2   /* ���ڲ�ѯ�ظ� */
#define APP_ACK_QUERY_STATUS    3   /* ״̬��ѯ�ظ� */
#define APP_ACK_QUERY_PARA      4   /* ������ѯ�ظ� */
#define APP_ACK_QUERY_CAPACITY  5   /* ������ѯ�ظ� */
#define APP_ACK_QUERY_TIME      6   /* ʱ���ѯ�ظ� */
#define APP_ACK_QUERY_PROG      7   /* ���Ȳ�ѯ�ظ� */

/*----------------------------------------------*
 * ���������Ͷ���                               *
 *----------------------------------------------*/

/* �ֱ�8λ��16λ��32λ��64λ�����������͹����� */
typedef union {
    u8 acData[8];                   /* ��8λ���� */
    u16 asData[4];                  /* ��16λ���� */
    u32 aiData[2];                  /* ��32λ���� */
    u64 ullData;                    /* ��64λ������� */
} unData;

/*----------------------------------------------*
 * �ṹ�����Ͷ���                               *
 *----------------------------------------------*/

/* Appͨ�ñ��ĸ�ʽ */
struct app_packet
{
    u8 magic[APP_MAX_MAGIC_LEN];    /* ʶ��key�����Acmd���ظ���Aack */
    u8 type;                        /* ����ͻظ������� */
    unData data;                /* ����ͻظ������� */
};

/* ��������ṹ�嶨�� */
typedef struct {
    u8 cmd_type;                    /* App���͹������������� */
    u8 ack_type;                    /* Dock����Ҫ���ظ�App�Ļظ����� */
    u64 (*cmd_func)(stDock *, u64); /* App������� */
} app_cmd;

extern u64 query_volt_handle(stDock *, u64);
extern u64 query_date_handle(stDock *, u64);
extern u64 query_status_handle(stDock *, u64);
extern u64 query_para_handle(stDock *, u64);
extern u64 query_capacity_handle(stDock *, u64);
extern u64 query_time_handle(stDock *, u64);
extern u64 query_prog_handle(stDock *, u64);
extern u64 change_mode_handle(stDock *, u64);
extern u64 shoot_operare_handle(stDock *, u64);
extern u64 preview_operare_handle(stDock *, u64);
extern u64 splice_video_handle(stDock *, u64);
extern u64 set_para_handle(stDock *, u64);

/*****************************************************************************
 �� �� ��  : app_init
 ��������  : App��ʼ��ʵ�ֺ���
*****************************************************************************/
extern void app_init(stDock *pstDock);


#ifdef  __cplusplus
}
#endif

#endif /* __APP_H__ */



