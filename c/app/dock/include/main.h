/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : main.h
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月15日
  最近修改   :
  功能描述   : main.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

/*----------------------------------------------*
 * 包含标准头文件                               *
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
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 变量类型声明                                 *
 *----------------------------------------------*/
typedef unsigned char       u8;     /* 无符号8位整型 */
typedef unsigned short      u16;    /* 无符号16位整型 */
typedef unsigned int        u32;    /* 无符号32位整型 */
typedef unsigned long long  u64;    /* 无符号64位整型 */

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/* 最大IP地址字符串长度 */
#define MAX_IP_ADDR_LEN         16

/* 最大同时连接数 */
#define MAX_CONNECT_NUM         128

/* 摄像头数据输入路数 */
#define CAMERA_DATA_INPUT_NUM   4

/* 解码线程数目 */
#define DECODE_THREAD_NUM       4

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define SHOOT_MODE_PHOTO    0 /* 拍照模式 */
#define SHOOT_MODE_VIDEO    1 /* 录像模式 */

#define STATUS_STOP         0 /* 停止状态 */
#define STATUS_DOING        1 /* 正在进行状态 */

/*----------------------------------------------*
 * 枚举类型定义                                 *
 *----------------------------------------------*/
typedef enum {
	SYS_STATUS_INIT  = 0,   /* 初始化状态 */
	SYS_STATUS_READY = 1,   /* 就绪状态 */
	SYS_STATUS_VIDEO = 2,   /* 正在录像状态 */
	SYS_STATUS_PHOTO = 3,   /* 正在拍照状态 */
	SYS_STATUS_ERROR = 4,   /* 出错状态 */
} SysStatusVaule;

/*----------------------------------------------*
 * 结构体类型定义                               *
 *----------------------------------------------*/

 /* App实时预览帧头 */
typedef struct {
    u32 uiID;       /* 帧ID */
    u32 uiLen;      /* 帧长度: （每次都可能不同） */
    u32 uiInterval; /* I帧间距和I帧标识（0：I帧标识；1~6：I帧间距） */
} frame_head;

/* Dock板 */
typedef struct {
    pthread_mutex_t mutex;  /* 本结构体内的值，所有改动的地方都锁 */

    /* Dock板状态类全局变量 */
    u8 ucSysStatus;         /* 系统状态: 取值范围见 SysStatusVaule */
    u8 ucShootMode;         /* 拍摄模式（0：录像模式；1：拍照模式） */
    u8 ucErrorCode;         /* 错误码（0：成功；非0：执行错误码） */
    u8 ucVoltCent;          /* 电量百分数（0~100， 20表示当前电量20%） */
    u8 ucChargeFlag;        /* 是否在充电标志（0：未充电；1：充电中） */
    u8 ucExpCompValue;      /* 曝光补偿值：±2EV，共13个档位 */
    u8 ucWhiteBalMode;      /* 白平衡模式：提供7种模式选择 */
    u8 ucResolution;        /* 分辨率（0：1440*720；1：2048*1024） */

    /* MCU相关全局变量 */
    int iSock_mm;

    /* 球机相关全局变量 */
    bool bBallDateSyncFlag;         /* 球机日期同步标志(初始化完成标志) */
    pthread_t BallDataThread;

    /* App相关全局变量 */
    u32 uiRecordTime;       /* 从录像开始到当前时间的秒数 */
	sem_t semRecordTime;    /* 查询时间命令 处理线程 app_cmd_handle_thd 等待的信号量，由 ball_ctrl_thd 唤醒 */

    /* 数据流停止标志 */
    bool stop_data_flag;

    /* 球机数据接收 -> 解码 全局变量 */
	pthread_t decodeThread[DECODE_THREAD_NUM];  /* 四路解码线程的ID */
	sem_t sem_recv_photo_data;                  /* 等待接收拍照数据信号量 */

    /* 解码 -> 拼接 全局变量 */
	sem_t sem_decode[DECODE_THREAD_NUM];                /* 四路解码线程等待信号量，接收到数据后唤醒 */
	bool decode_done_flag;                              /* 完成的一帧数据解码完成标志 */
	char *pDecodeToStitchBuffer[CAMERA_DATA_INPUT_NUM]; /* 解码线程给拼接线程的四路解码数据输入 */

    /* 拼接 -> 编码 全局变量 */
	sem_t sem_stitch;               /* 拼接线程等待的信号量，解码完成后唤醒 */
	bool stitch_done_flag;          /* 拼接完成标志 */
    char *pStitchToEncodeBuffer;    /* 拼接线程给编码线程的数据 */

    /* 编码 -> FFmpeg 全局变量 */
    sem_t sem_encode;               /* 编码线程等待的信号量，拼接完成后唤醒 */
    bool encode_done_flag;          /* 编码完成标志 */

	/* 拼接 -> HDMI预览 全局变量 */
    bool HDMI_preview_flag;         /* HDMI预览开始和结束标志 */
	sem_t sem_HDMI_preview;         /* HDMI预览等待的信号量，由拼接完成后唤醒 */
    bool HDMI_send_done_flag;       /* HDMI预览数据发送完成标志 */
    char *pHdmiPreviewBuffer;       /* 拼接线程给HDMI的预览数据 */

    /* 编码 -> App预览 全局变量 */
    bool app_preview_flag;          /* App预览标志（STATUS_STOP：停止预览；STATUS_DOING：正在预览） */
	sem_t sem_app_preview;          /* App预览等待的信号量，编码完成后唤醒 */
    bool app_send_done_flag;        /* App预览数据发送完成标志 */
    char *pAppPreviewBuffer;        /* 编码线程给App的预览数据 */
    frame_head stAppFrameHead;      /* App预览数据帧头 */

    /* 录像拼接 全局变量 */
    bool video_stitch_flag;         /* 录像拼接标志 */
    u32 uiVideoSecond;              /* 当前拼接的录像文件总共时间长度，单位：秒 */
    u32 uiFrameCount;               /* 当前拼接的录像文件已经拼接完成的帧数 */

} stDock;

/* 连接信息结构体 */
typedef struct
{
    stDock *pstDock;
    int     iConnfd;
} stConnectInfo;

/*****************************************************************************
 函 数 名  : set_sys_status
 功能描述  : 设置系统状态实现函数
*****************************************************************************/
extern void set_sys_status(stDock *pstDock, SysStatusVaule ucStatus);

/*****************************************************************************
 函 数 名  : get_netif_ip
 功能描述  : 获取网络接口IP地址实现函数
*****************************************************************************/
extern char * get_netif_ip(char* pIfName, char *pIpBuf);

/*****************************************************************************
 函 数 名  : get_dock
 功能描述  : 获取Dock结构体实现函数
*****************************************************************************/
extern stDock *get_dock(void);

/*****************************************************************************
 函 数 名  : system_init
 功能描述  : 系统初始化实现函数，同时作为初始化调试线程使用
*****************************************************************************/
extern void *system_init(void *arg);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MAIN_H__ */

