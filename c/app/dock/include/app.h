/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : app.h
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月19日
  最近修改   :
  功能描述   : app.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月19日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

#ifndef __APP_H__
#define __APP_H__

/*----------------------------------------------*
 * 包含头文件                                   *
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

 /* App数据使用的Socket: sock_ad IP地址和端口 */
#define SOCK_AD_IP_ADDR             "192.168.3.1"
#define SOCK_AD_PORT                9999

 /* App命令使用的Socket: sock_ac IP地址和端口 */
#define SOCK_AC_IP_ADDR             "192.168.3.1"
#define SOCK_AC_PORT                8888

#define APP_MAX_MAGIC_LEN           7   /* App报文最大魔法字（识别key）长度 */

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/* App发送给Dock板的命令定义如下：*/
#define APP_CMD_QUERY_VOLT      1   /* 查询电量命令 */
#define APP_CMD_QUERY_DATE      2   /* 查询日期命令 */
#define APP_CMD_QUERY_STATUS    3   /* 查询状态命令 */
#define APP_CMD_QUERY_PARA      4   /* 查询参数命令 */
#define APP_CMD_QUERY_CAPACITY  5   /* 查询容量命令 */
#define APP_CMD_QUERY_TIME      6   /* 查询时间命令 */
#define APP_CMD_QUERY_PROG      7   /* 查询进度命令 */
#define APP_CMD_CHANGE_MODE     8   /* 模式切换命令 */
#define APP_CMD_SHOOT_OPERATE   9   /* 拍摄操作命令 */
#define APP_CMD_PREVIEW_OPERATE 10  /* 预览操作命令 */
#define APP_CMD_SPLICE_VIDEO    11  /* 录像拼接命令 */
#define APP_CMD_SET_PARA        12  /* 设置参数命令 */

/* 最大App命令数 */
#define MAX_APP_CMD_NUM         12

/* Dock板发送给App的回应定义如下：*/
#define APP_ACK_QUERY_VOLT      1   /* 电量查询回复 */
#define APP_ACK_QUERY_DATE      2   /* 日期查询回复 */
#define APP_ACK_QUERY_STATUS    3   /* 状态查询回复 */
#define APP_ACK_QUERY_PARA      4   /* 参数查询回复 */
#define APP_ACK_QUERY_CAPACITY  5   /* 容量查询回复 */
#define APP_ACK_QUERY_TIME      6   /* 时间查询回复 */
#define APP_ACK_QUERY_PROG      7   /* 进度查询回复 */

/*----------------------------------------------*
 * 公用体类型定义                               *
 *----------------------------------------------*/

/* 分别按8位、16位、32位和64位来操作的整型公用体 */
typedef union {
    u8 acData[8];                   /* 按8位操作 */
    u16 asData[4];                  /* 按16位操作 */
    u32 aiData[2];                  /* 按32位操作 */
    u64 ullData;                    /* 按64位整体操作 */
} unData;

/*----------------------------------------------*
 * 结构体类型定义                               *
 *----------------------------------------------*/

/* App通用报文格式 */
struct app_packet
{
    u8 magic[APP_MAX_MAGIC_LEN];    /* 识别key，命令：Acmd，回复：Aack */
    u8 type;                        /* 命令和回复的类型 */
    unData data;                /* 命令和回复的数据 */
};

/* 调试命令结构体定义 */
typedef struct {
    u8 cmd_type;                    /* App发送过来的命令类型 */
    u8 ack_type;                    /* Dock板需要返回给App的回复类型 */
    u64 (*cmd_func)(stDock *, u64); /* App命令处理函数 */
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
 函 数 名  : app_init
 功能描述  : App初始化实现函数
*****************************************************************************/
extern void app_init(stDock *pstDock);


#ifdef  __cplusplus
}
#endif

#endif /* __APP_H__ */



