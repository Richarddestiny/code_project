/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : ball.h
  �� �� ��   : ����
  ��    ��   : ¬��
  ��������   : 2016��4��20��
  ����޸�   :
  ��������   : ball.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��20��
    ��    ��   : ¬��
    �޸�����   : �����ļ�

******************************************************************************/

#ifndef __BALL_H__
#define __BALL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include "main.h"

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
#define EVOCAM_STREAM               "EvoS"
#define SOCK_BC_IP_ADDR             "192.168.0.169"
#define SOCK_BC_PORT                6666
#define SOCK_BD_PORT                7777
#define MAGIC_CMD				    "Bcmd"
#define MAGIC_BACK				    "Back"

#define BALL_MAX_MAGIC_LEN          8

#define PICTURE_SIZE				 1280 * 1080 * 1.5 * 4		//////����ͼƬ��С(4���ܴ�С)


/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
/* Dock���͸��������������£� */
#define BALL_CMD_KEEP_ALIVE 		0   /* ������ */
#define BALL_CMD_DATE_OPERATE		1   /* ���ڲ������� */
#define BALL_CMD_CHANGE_MODE		2   /* �л�ģʽ���� */
#define BALL_CMD_START_PREVIEW		3   /* ��ʼԤ������ */
#define BALL_CMD_STOP_PREVIEW		4   /* ֹͣԤ������ */
#define BALL_CMD_START_VIDEO		5   /* ��ʼ¼������ */
#define BALL_CMD_STOP_VIDEO			6   /* ֹͣ¼������ */
#define BALL_CMD_READ_VIDEO_TIME 	7   /* ��ȡ¼��ʱ������ */
#define BALL_CMD_TAKE_PHOTO			8   /* �������� */
#define BALL_CMD_SET_EXP_COMP		9   /* �����عⲹ������ */
#define BALL_CMD_SET_WT_BAL_MODE	10  /* ���ð�ƽ��ģʽ���� */

#define NOTIFY_CTRL_VIDEO_COMPLETE  20 /*֪ͨ�����߳̽�264¼���ļ�����ת��*/

/* ������͸�Dock�Ļ�Ӧ�������£� */
#define BALL_ACK_READ_DATE			11   /* ��ȡ���ڻ�Ӧ */
#define BALL_ACK_READ_VIDEO_TIME	12   /* ��ȡ¼��ʱ���Ӧ */
#define BALL_ACK_TAKE_PHOTO			13   /* ���ջ�Ӧ */

/*----------------------------------------------*
 * �ṹ�����Ͷ���                               *
 *----------------------------------------------*/
struct ball_packet
{
	char magic[BALL_MAX_MAGIC_LEN]; /* ʶ��key������Ϊ��Bcmd����ӦΪ��Back */
	u32 type;					    /* ������߻�Ӧ���� */
	u32 data;					    /* ������߻�Ӧ���� */
};

////������͸�dock���֡��Ϣ
struct frame_info
{
	u32 seq_no;     /* ֡���к� */
	u32 i_frame;    /* I֡��ʶ��i_frame == 1����ǰ֡Ϊ�ؼ�֡��I֡�� */
	u32 timestamp;  /* ֡ʱ�������ȷ������msע���� ��������Ƶͬ�� */
	u32 len[4];		/* ��·����ͷ����֡���ȣ�����ͼ�е�nerstr.len[0~3] */
};

////����������ݰ��Ľṹ
struct dock {
	char magic_key[5];
	int cmd_type;
	int data_size;
	char buf[1024];
};

////�洢decode��buffer
struct ball_frame_buffer
{
	int nLen[4];					////4֡���ݳ���
	char* pszFrameBuffer[4];		////4·֡��Ϣ
};

////����ƽ��ֵ
typedef enum
{
	EXP_COMP_AUTO   = 0,	 //�Զ�ģʽ
	EXP_COMP_P2_0EV = 1,
	EXP_COMP_P1_7EV = 2,
	EXP_COMP_P1_3EV = 3,
	EXP_COMP_P1_0EV = 4,
	EXP_COMP_P0_7EV = 5,
	EXP_COMP_P0_3EV = 6,
	EXP_COMP_P0_0EV = 7,
	EXP_COMP_N0_3EV = 8,
	EXP_COMP_N0_7EV = 9,
	EXP_COMP_N1_0EV = 10,
	EXP_COMP_N1_3EV = 11,
	EXP_COMP_N1_7EV = 12,
	EXP_COMP_N2_0EV = 13,
}exp_comp_value;

////��ƽ��ģʽ
typedef enum
{
	WB_MODE_AUTO = 0,      //�Զ�
	WB_MODE_SUNSHINE = 1, //�չ�
	WB_MODE_CLOUDY	   = 2, //����
	WB_MODE_SHADOW	   = 3, //��Ӱ
	WB_MODE_WT_LIGHT = 4, //�׳��
	WB_MODE_FL_LIGHT = 5, //ӫ���
	WB_MODE_SUNLIGHT = 6,  //�չ��
}white_bal_mode;

extern struct ball_frame_buffer gFrameBuffer;	////�洢decode��buffer

/*----------------------------------------------*
 * ���⺯��ԭ��˵��                             *
 *----------------------------------------------*/

/*****************************************************************************
 �� �� ��  : ball_data_thd
 ��������  : ��������߳�ʵ�ֺ���
*****************************************************************************/
extern void *ball_data_thd(void *arg);

extern void ball_init(stDock *pstDock);
extern int send_ball_cmd_pkt( u32 ulCmd, u32 ulData);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __BALL_H__ */

