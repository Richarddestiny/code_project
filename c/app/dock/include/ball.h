/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : ball.h
  版 本 号   : 初稿
  作    者   : 卢磊
  生成日期   : 2016年4月20日
  最近修改   :
  功能描述   : ball.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月20日
    作    者   : 卢磊
    修改内容   : 创建文件

******************************************************************************/

#ifndef __BALL_H__
#define __BALL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * 包含头文件                                   *
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
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/
#define EVOCAM_STREAM               "EvoS"
#define SOCK_BC_IP_ADDR             "192.168.0.169"
#define SOCK_BC_PORT                6666
#define SOCK_BD_PORT                7777
#define MAGIC_CMD				    "Bcmd"
#define MAGIC_BACK				    "Back"

#define BALL_MAX_MAGIC_LEN          8

#define PICTURE_SIZE				 1280 * 1080 * 1.5 * 4		//////拍照图片大小(4幅总大小)


/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
/* Dock发送给球机的命令定义如下： */
#define BALL_CMD_KEEP_ALIVE 		0   /* 心跳包 */
#define BALL_CMD_DATE_OPERATE		1   /* 日期操作命令 */
#define BALL_CMD_CHANGE_MODE		2   /* 切换模式命令 */
#define BALL_CMD_START_PREVIEW		3   /* 开始预览命令 */
#define BALL_CMD_STOP_PREVIEW		4   /* 停止预览命令 */
#define BALL_CMD_START_VIDEO		5   /* 开始录像命令 */
#define BALL_CMD_STOP_VIDEO			6   /* 停止录像命令 */
#define BALL_CMD_READ_VIDEO_TIME 	7   /* 读取录制时间命令 */
#define BALL_CMD_TAKE_PHOTO			8   /* 拍照命令 */
#define BALL_CMD_SET_EXP_COMP		9   /* 设置曝光补偿命令 */
#define BALL_CMD_SET_WT_BAL_MODE	10  /* 设置白平衡模式命令 */

#define NOTIFY_CTRL_VIDEO_COMPLETE  20 /*通知控制线程将264录像文件进行转化*/

/* 球机发送给Dock的回应定义如下： */
#define BALL_ACK_READ_DATE			11   /* 读取日期回应 */
#define BALL_ACK_READ_VIDEO_TIME	12   /* 读取录制时间回应 */
#define BALL_ACK_TAKE_PHOTO			13   /* 拍照回应 */

/*----------------------------------------------*
 * 结构体类型定义                               *
 *----------------------------------------------*/
struct ball_packet
{
	char magic[BALL_MAX_MAGIC_LEN]; /* 识别key：命令为：Bcmd；回应为：Back */
	u32 type;					    /* 命令或者回应类型 */
	u32 data;					    /* 命令或者回应数据 */
};

////球机发送给dock板的帧信息
struct frame_info
{
	u32 seq_no;     /* 帧序列号 */
	u32 i_frame;    /* I帧标识，i_frame == 1：当前帧为关键帧（I帧） */
	u32 timestamp;  /* 帧时间戳（精确到毫秒ms注）， 用于音视频同步 */
	u32 len[4];		/* 四路摄像头编码帧长度，即下图中的nerstr.len[0~3] */
};

////球机接收数据包的结构
struct dock {
	char magic_key[5];
	int cmd_type;
	int data_size;
	char buf[1024];
};

////存储decode的buffer
struct ball_frame_buffer
{
	int nLen[4];					////4帧数据长度
	char* pszFrameBuffer[4];		////4路帧信息
};

////补光平衡值
typedef enum
{
	EXP_COMP_AUTO   = 0,	 //自动模式
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

////白平衡模式
typedef enum
{
	WB_MODE_AUTO = 0,      //自动
	WB_MODE_SUNSHINE = 1, //日光
	WB_MODE_CLOUDY	   = 2, //阴天
	WB_MODE_SHADOW	   = 3, //阴影
	WB_MODE_WT_LIGHT = 4, //白炽灯
	WB_MODE_FL_LIGHT = 5, //荧光灯
	WB_MODE_SUNLIGHT = 6,  //日光灯
}white_bal_mode;

extern struct ball_frame_buffer gFrameBuffer;	////存储decode的buffer

/*----------------------------------------------*
 * 对外函数原型说明                             *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : ball_data_thd
 功能描述  : 球机数据线程实现函数
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

