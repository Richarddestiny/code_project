/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : dbg.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月15日
  最近修改   :
  功能描述   : 调试相关函数实现
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "include/main.h"
#include "include/mcu.h"
#include "include/ball.h"
#include "include/dbg.h"
#include "include/preview.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/
void *ball_data_thd(void *arg);
void *ball_ctrl_thd(void *arg);


/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/
static void dbg_cmd_help_func(char *param);
static void dbg_cmd_test_func(char *param);
static void dbg_cmd_exit_func(char *param);
static void dbg_cmd_init_func(char *param);
static void dbg_cmd_hdmi_func(char *param);
static void dbg_cmd_mcu_func(char *param);
static void dbg_cmd_app_func(char *param);
static void dbg_cmd_asw_func(char *param);

static void ll_ball_dock_test2();
static void ll_ball_dock_test();
static void ll_ball_dock_test3();



/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define MAX_DBG_CMD_NUM     100
#define MAX_DBG_CMD_LEN     128

/*----------------------------------------------*
 * 宏函数定义                                   *
 *----------------------------------------------*/
 #define dbgmsg(fmt, arg...) \
        do { \
                printf("DBG> "fmt, ##arg);\
        } while (0)

/*----------------------------------------------*
 * 结构体类型定义                               *
 *----------------------------------------------*/
/* 调试命令结构体定义 */
typedef struct {
        char cmd_name[64];
        char usage[128];
        void (*cmd_func)(char *param);
} dbg_cmd;

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
u8 g_debug_switch = DEBUG_OFF;
u8 g_init_debug_switch = DEBUG_ON;
u8 g_hdmi_debug_switch = DEBUG_OFF;
u8 g_app_preview_debug_switch = DEBUG_OFF;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/* 调试命令列表 */
static dbg_cmd m_cmd_list[MAX_DBG_CMD_NUM + 1] = {
    {"help", "0 - help [param]\t// help", dbg_cmd_help_func},
    {"test", "1 - test [param]\t// unit test", dbg_cmd_test_func},
    {"exit", "2 - exit [param]\t// exit dbg", dbg_cmd_exit_func},
    {"init", "3 - init [param]\t// init dbg", dbg_cmd_init_func},
    {"hdmi", "4 - hdmi [param]\t// HDMI dbg", dbg_cmd_hdmi_func},
    {"mcu", "5 - mcu  [param]\t// MCU dbg", dbg_cmd_mcu_func},
	{"app", "6 - app  [param]\t// app func test", dbg_cmd_app_func},
	{"asw", "7 - asw  NULL\t// app preview dbg switch", dbg_cmd_asw_func},
    {"NULL", "NULL", NULL},
};

/*****************************************************************************
 函 数 名  : set_system_log
 功能描述  : 设置系统日志
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void set_system_log(void)
{
    char cmdbuf[64];
    int nouse = 0;

    nouse = nouse;
    memset(cmdbuf, 0, sizeof(cmdbuf));
    sprintf(cmdbuf, "su -c \"chmod 664 /var/log/syslog\"");
    nouse = system(cmdbuf);

    memset(cmdbuf, 0, sizeof(cmdbuf));
    sprintf(cmdbuf, "> /var/log/syslog");
    nouse = system(cmdbuf);
}

/*****************************************************************************
 函 数 名  : dbg_cmd_help_func
 功能描述  : 调试命令-帮助实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_help_func(char *param)
{
    int i = 0;

    while (m_cmd_list[i].cmd_func) {
        printf("%s\n", m_cmd_list[i].usage);
        i++;
    }
    printf("\n");
}

/*****************************************************************************
 函 数 名  : dbg_cmd_UnitTest_func
 功能描述  : 调试命令-单元测试实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_test_func(char *param)
{
    int num;
    struct tm *pTime;
    u32 uiTimeStamp;
    unData unOutputData, unInputData;
    stDock *pstDock = get_dock();

    msg("unit test: %s\n", param);
    if (NULL == param)
        return;

    num = atoi(param);
    /* 可以在此处放入单元测试函数 */
	ll_ball_dock_test2();

    unInputData.aiData[0] = num;
    unOutputData.ullData = query_date_handle(pstDock, unInputData.ullData);
    uiTimeStamp = unOutputData.aiData[0];

	stime( &uiTimeStamp);
	pTime = localtime(&uiTimeStamp);
	if (num)
	    msg("Set time to : %s\n", asctime(pTime) );
	else
        msg("Get Time : %s\n", asctime(pTime));
}

/*****************************************************************************
 函 数 名  : dbg_cmd_exit_func
 功能描述  : 调试命令-退出实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_exit_func(char *param)
{
    exit(0);
}

/*****************************************************************************
 函 数 名  : dbg_cmd_init_func
 功能描述  : 调试命令-初始化实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_init_func(char *param)
{
    int ret;
    pthread_t MainThdID;

    dbg("(main Thread) Enter init debug\n");

    /* 另起线程作为主线程 system_init */
    ret = pthread_create(&MainThdID, NULL, system_init, NULL);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);
        msg("Can't Create thread %s, Error no: %d (%s)\n", "system_init", errno, err_msg);
        return;
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(MainThdID);
}

/*****************************************************************************
 函 数 名  : dbg_cmd_hdmi_func
 功能描述  : 调试命令-HDMI实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_hdmi_func(char *param)
{
    char acPrtInfo[6];
    char *str[] = {"disconnect", "connect"};

    if (NULL == param)
        strcpy(acPrtInfo, "off");
    else
        strcpy(acPrtInfo, "on");

    msg("HDMI debug: HDMI status: %s\n", acPrtInfo);
    /* 获取当前HDMI接口状态 */
    if (strncmp(acPrtInfo, "on", 2) == 0)
    {
        g_hdmi_debug_switch = DEBUG_ON;
        dbg("(main Thread) Set hdmi status %s\n", str[g_hdmi_debug_switch]);
    }
    else
    {
        g_hdmi_debug_switch = DEBUG_OFF;
        dbg("(main Thread) Set hdmi status %s\n", str[g_hdmi_debug_switch]);
    }
}

/*****************************************************************************
 函 数 名  : dbg_cmd_mcu_func
 功能描述  : 调试命令-MCU实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月23日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_mcu_func(char *param)
{
    u8 ucType = MCU_CMD_CHANG_POWER_LIGHT;
    u8 ucData = 0;
    int num = 0;

    msg("MCU debug test: %s\n", param);

    if (NULL == param)
        return;

    num = atoi(param);
    /* 发送 到MCU的命令 */
    switch (num)
    {
        case 1:
            ucType = MCU_CMD_QUERY_VOLT;
            break;
        case 2:
            ucType = MCU_CMD_CHANG_MODE_LIGHT;
            ucData = LED_STATUS_GS;
            break;
        case 3:
            ucType = MCU_CMD_CHANG_MODE_LIGHT;
            ucData = LED_STATUS_OS;
            break;
        case 4:
            ucData = LED_STATUS_GS;
            break;
        case 5:
            ucData = LED_STATUS_OS;
            break;
        case 6:
            ucData = LED_STATUS_RS;
            break;
        case 7:
            ucData = LED_STATUS_GB_1;
            break;
        case 8:
            ucData = LED_STATUS_OB_1;
            break;
        case 9:
            ucData = LED_STATUS_RB_1;
            break;
        case 10:
            ucData = LED_STATUS_GB_4;
            break;
        case 11:
            ucData = LED_STATUS_RB_4;
            break;
        case 12:
            ucData = LED_STATUS_OB_4;
            break;
        default:
            return;
    }

    //msg("Send MCU %s command, data: %d\n", mcu_type_to_str((u8)ucType), ucData);
    send_mcu_cmd_pkt(ucType, ucData);
}

/*****************************************************************************
 函 数 名  : dbg_cmd_app_func
 功能描述  : 调试命令-app实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月25日
    作    者   : 蒋小辉
    修改内容   : 新生成函数
  2.日    期   : 2016年4月25日
    作    者   : 彭斌全
    修改内容   : 改成另起线程模式，不会阻塞调试控制台

*****************************************************************************/
static void dbg_cmd_app_func(char *param)
{
    int ret, num;
    unData unInputData, unOutputData;
    stDock *pstDock = get_dock();
    static char *acStr[] = {"unknown", "query volt", "query date", "query status",
    "query parameter", "query capacity", "query time", "query progress", "change mode",
    "shoot operate", "preview operate", "splice video", "set parameter"};

    dbg("(main Thread) Enter app func test\n");

    if (NULL == param)
    {
        dbg("Not find any parameters\n");
        return;
    }

    num = atoi(param);
    msg("App %s func test\n", acStr[num]);

    /* 发送 到MCU的命令 */
    switch (num)
    {
        case APP_CMD_QUERY_VOLT: /* 查询电量命令 */
            unOutputData.ullData = query_volt_handle(pstDock, 0);
            dbg("Get volt info: VoltCent: %d  ChargeFlag: %d\n", unOutputData.acData[0], unOutputData.acData[1]);
            break;
        case APP_CMD_QUERY_DATE: /* 日期操作命令 */
        {
			struct tm *pTime;
            unInputData.aiData[0] = 1461985800;

            pTime = localtime((time_t *)&unInputData.aiData[0]);
            dbg("Set Ball TimeStamp: %#x(%s)\n\n", unInputData.aiData[0], asctime(pTime));
            if (query_date_handle(pstDock, unInputData.ullData))
                dbg("Set Ball TimeStamp failed!\n");

            unOutputData.ullData = query_date_handle(pstDock, 0);
            pTime = localtime((time_t *)&unOutputData.aiData[0]);
            dbg("Get Dock TimeStamp: %#x(%s)\n", unOutputData.aiData[0], asctime(pTime));
            break;
        }
        case APP_CMD_QUERY_STATUS: /* 查询状态命令 */
            unOutputData.ullData = query_status_handle(pstDock, 0);
            dbg("Get shoot_mode: %d\n", unOutputData.acData[0]);
            dbg("Get video_status: %d\n", unOutputData.acData[1]);
            dbg("Get preview_status: %d\n", unOutputData.acData[2]);
            dbg("Get error_code: %d\n", unOutputData.acData[3]);
            break;
        case APP_CMD_QUERY_PARA: /* 查询参数命令 */
            unOutputData.ullData = query_para_handle(pstDock, unInputData.ullData);
            dbg("Get ExpCompValue: %d  WhiteBalMode: %d Resolution: %d\n",
                unOutputData.acData[0], unOutputData.acData[1], unOutputData.acData[2]);
            break;
        case APP_CMD_QUERY_CAPACITY: /* 查询容量命令 */
            unOutputData.ullData = query_capacity_handle(pstDock, 0);
            dbg("Get Avail Capacity: %d MB\n", unOutputData.asData[0]);
            break;
        case APP_CMD_QUERY_TIME: /* 查询时间命令 */
            unOutputData.ullData = query_time_handle(pstDock, 0);
            dbg("Get record Time: %u s\n", unOutputData.aiData[0]);
            dbg("Get remain Time: %u s\n", unOutputData.aiData[1]);
            break;
        case APP_CMD_QUERY_PROG: /* 查询进度命令 */
            pstDock->uiFrameCount = 4567;
            pstDock->uiVideoSecond = 300;
            unOutputData.ullData = query_prog_handle(pstDock, 0);
            dbg("Get progress Cent: %u%%\n", unOutputData.acData[0]);
            break;
        case APP_CMD_CHANGE_MODE: /* 模式切换命令 */
            change_mode_handle(pstDock, 0);
            dbg("Changet to shoot_mode: %d\n", pstDock->ucShootMode);
            break;
        case APP_CMD_SHOOT_OPERATE: /* 拍摄操作命令 */
            unOutputData.ullData = shoot_operare_handle(pstDock, unInputData.ullData);
            break;
        case APP_CMD_PREVIEW_OPERATE: /* 预览操作命令 */
            unOutputData.ullData = preview_operare_handle(pstDock, unInputData.ullData);
            break;
        case APP_CMD_SPLICE_VIDEO: /* 录像拼接命令 */
            unOutputData.ullData = splice_video_handle(pstDock, unInputData.ullData);
            break;
        case APP_CMD_SET_PARA: /* 设置参数命令 */
            unOutputData.ullData = set_para_handle(pstDock, unInputData.ullData);
            break;
        default:
            msg("app func %d not found\n", num);
            return;
    }
}

/*****************************************************************************
 函 数 名  : dbg_cmd_asw_func
 功能描述  : 调试命令 - App实时预览实现函数
 输入参数  : char *param
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月27日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_asw_func(char *param)
{
    u8 ucAppPreviewSwitch;
    char acPrtInfo[6];
    char *str[] = {"Close", "Open"};
    stDock *pstDock = get_dock();

    g_app_preview_debug_switch = DEBUG_ON;
    if (NULL == param)
    {
        strcpy(acPrtInfo, str[pstDock->app_preview_flag]);
        dbg("(main Thread) When App preview is %s\n", acPrtInfo);

        if (pstDock->app_preview_flag == STATUS_STOP)
        {   /* App预览已经关闭，打开App预览 */
            ucAppPreviewSwitch = STATUS_DOING;
        }
        else if (pstDock->app_preview_flag == STATUS_DOING)
        {   /* App预览已经打开，关闭App预览 */
            ucAppPreviewSwitch = STATUS_STOP;
        }
        dbg("(main Thread) %s App preview\n", str[ucAppPreviewSwitch]);
        preview_operare_handle(pstDock, 0);
    }
    else
    {
        dbg("No need for any parameters\n");
    }
}

/*****************************************************************************
 函 数 名  : dbg_cmd_handle
 功能描述  : 调试命令处理实现函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dbg_cmd_handle(void)
{
    int i = 0;
    int num = 0;
    char cmd_info[MAX_DBG_CMD_LEN];
    char *pcmd = NULL;
    char *param = NULL;
    char *nouse;

    while (1) {
        dbgmsg("");
        memset(cmd_info, 0, sizeof(cmd_info));

        /* 命令格式: cmd param1 param2 ... */
        /* 如果输错了可以用Ctrk-Backspace删除字符 */
        nouse = fgets(cmd_info, sizeof(cmd_info), stdin);
        if (strlen(cmd_info) == 1 && cmd_info[0] == '\n')
            continue;   /* 跳过只敲回车键 */
        else if (isdigit(cmd_info[0])) {
            /* 首个字符为数字表示是序号 */
            num = atoi(cmd_info);
            m_cmd_list[num].cmd_func(NULL);
            continue;
        }

        cmd_info[strlen(cmd_info) - 1] = '\0';
        for (i = 0; i < strlen(cmd_info); i++) {
            if (cmd_info[i] == ' ' || cmd_info[i] == '\0')
                break;
        }

        if (cmd_info[i] == '\0') {
            param = NULL;
            pcmd = cmd_info;
        } else if (cmd_info[i] == ' ') {
            cmd_info[i] = '\0';
            pcmd =  cmd_info;
            param = &cmd_info[i + 1];
        }

        for (i = 0; i < MAX_DBG_CMD_NUM; i++) {
            if (m_cmd_list[i].cmd_func == NULL)
                break;
            if (strcmp(m_cmd_list[i].cmd_name, pcmd) == 0)
                break;
        }

        if (m_cmd_list[i].cmd_func)
            m_cmd_list[i].cmd_func(param);
        else
            dbgmsg("cmd %s not found\n", pcmd);
    }
}

/*****************************************************************************
 函 数 名  : dbg_console
 功能描述  : 调试控制台实现函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void dbg_console(void)
{
    char *value = NULL;

    /* 获取环境变量的值 */
    value = getenv("DBG_DOCK");
    if (value && (strncmp(value, "on", 2) == 0)) {
        msg("DBG_DOCK is on: enter dbg console.\n");
        g_debug_switch = DEBUG_ON;
        dbg_cmd_help_func(NULL);
        dbg_cmd_handle();
    }

    msg("DBG_DOCK=on is not set, So skip dbg_console.\n");
}

/*****************************************************************************
 函 数 名  : ll_ball_dock_test
 功能描述  : 调试控制台实现函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 卢磊
    修改内容   : 新生成函数

*****************************************************************************/
void ll_ball_dock_test()
{
	//创建控制线程
	dbg("enter ball_dock_test function\n");
	stDock Dock;
	int nCmd;
	int nData = 0;
	int nLoop = 0;
	int i = 0;
	pthread_t nId;

	pthread_create(&nId, NULL, ball_ctrl_thd, (void *)&Dock);
	dbg("create ctrl thread successful\n");

	sleep(2);
	nData = 1;
	printf("input loop time >>>");
	scanf("%d", &nLoop);
	for(i = 0; i < nLoop; i++)
	{
		dbg("**********begin %d***********\n", i);
		sleep(1);
		//创建数据线程
		pthread_create(&nId, NULL, ball_data_thd, (void *)&Dock);
		dbg("create data thread successful\n");

		sleep(2);
		//查询时间
		send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, 0);

		sleep(2);
		////设置时间0x5708ffff
		send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, 0x5708ffff);

		sleep(2);
		//开始预览*******************
		send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);


		//sleep(5);
		//拍照
		//send_ball_cmd_pkt(BALL_CMD_TAKE_PHOTO, 0);

		//sleep(5);
		//录像
		//send_ball_cmd_pkt(BALL_CMD_START_VIDEO, 0);

		//sleep(5);
		//查询录像时间
		//send_ball_cmd_pkt(BALL_CMD_READ_VIDEO_TIME, 0);

		//sleep(5);
		//停止录像
		//send_ball_cmd_pkt(BALL_CMD_STOP_VIDEO, 0);

		sleep(5);
		//设置曝光
		//if(nData > 13)
			//nData = 0;
		//send_ball_cmd_pkt(BALL_CMD_SET_EXP_COMP, nData++);


		//sleep(3);
		//停止预览*********************
		send_ball_cmd_pkt(BALL_CMD_STOP_PREVIEW, 0);

		dbg("**********end %d***********\n", i);
		sleep(2);
	}

	getchar();
}

void ll_ball_dock_test2()
{
	//创建控制线程
	dbg("enter ball_dock_test function\n");
	stDock Dock;
	int nCmd;
	int nData = 0;
	pthread_t nId;

	pthread_create(&nId, NULL, ball_ctrl_thd, (void *)&Dock);
	dbg("create ctrl thread successful\n");

	sleep(2);

	nData = 0;
	while(1)
	{
		printf("cmd >>>");
		scanf("%d", &nCmd);

		if(nCmd == -1)
		{
			//创建数据线程
			pthread_create(&nId, NULL, ball_data_thd, (void *)&Dock);
			sleep(2);
			continue;
		}

		if(nData > 13)
			nData = 0;
		send_ball_cmd_pkt(nCmd, nData++);

		sleep(2);
	}
}

void ll_ball_dock_test3()
{
	int nSelect = 1;

	dbg("1:manual  2:auto\n");
	dbg("you select?:");

	if(nSelect == 1)
	{
		ll_ball_dock_test2();
	}
	else
	{
		ll_ball_dock_test();
	}
}



