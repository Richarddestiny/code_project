/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : preview.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月22日
  最近修改   :
  功能描述   : 实时预览相关函数实现
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月22日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "include/preview.h"

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

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : preview_fornt_thd
 功能描述  : 预览前端线程实现函数
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void *preview_fornt_thd(void *arg)
{
	pthread_t stitchThread;
    stDock *pstDock = (stDock *)arg;

    dbg("(Preview Fornt Thread) Enter preview_fornt_thd\n\n");

    /* 创建球机数据线程 */
    pthread_create(&pstDock->BallDataThread, NULL, ball_data_thd, (void *)pstDock);

    /* 创建四个解码处理线程 */
    create_decode_thread(pstDock);

    /* 创建拼接处理线程 */
    pthread_create(&stitchThread, NULL, stitch_handle_thd, (void *)pstDock);

    return NULL;
}

/*****************************************************************************
 函 数 名  : hdmi_preview_thd
 功能描述  : 预览处理线程实现函数
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void *hdmi_preview_thd(void *arg)
{
    int fp = 0;
    u32 ulScreenSize;
    char *fbp;
    stDock *pstDock = (stDock *)arg;

    dbg("(Preview Handle Thread) Enter preview_handle_thd\n\n");

    /* 打开HDMI设备文件 */
    fp = open("/dev/fb1", O_RDWR);
    if (fp < 0)
    {
        msg("Error: Can not open framebuffer device FB1\n");
        return;
    }

    /* 映射内存到打开的设备文件上 */
    ulScreenSize = (HDMI_PREVIEW_WIDTH * HDMI_PREVIEW_HEIGHT * 3) / 2;
	fbp = (char *)mmap(0, ulScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	if (fbp == NULL)
	{
		msg("Error: failed to map framebuffer device to memory\n");
		close(fp);
		return;
	}

    /* 当检查到HDMI接口被拔出时，退出循环，关闭线程 */
    while (pstDock->HDMI_preview_flag)
    {
        /* 等待拼接完毕后唤醒 */
		sem_wait(&pstDock->sem_HDMI_preview);

        /* 发送HDMI预览数据到接口上 */
        memcpy(fbp, pstDock->pHdmiPreviewBuffer, ulScreenSize);

        /* 设置HDMI预览发送完成标志 */
        pstDock->HDMI_send_done_flag = true;
    }

    munmap(fbp, ulScreenSize);
    close(fp);
    dbg("(Preview Handle Thread) Quit preview_handle_thd\n\n");
}

/*****************************************************************************
 函 数 名  : hdmi_status_check
 功能描述  : HDMI接口连接状态检查实现函数
 输入参数  : void
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void hdmi_status_check(stDock *pstDock)
{
    //char *ret;
    int ret;
    char str[100];
    bool bCutStatus = false;
    static bool bOldStatus = false;     /* 默认启动前未接入HDMI */

    /* 打开检验用文件 */
    //freopen("/sys/kernel/debug/tegra_hdmi/regs", "r", stdin);
    //freopen("/datadisk/ee/regs", "r", stdin);

    /* 获取当前HDMI接口状态 */

    //ret = gets(str);
    if (g_hdmi_debug_switch)
    {
        if (g_init_debug_switch)
            dbg("(main Thread) Get hdmi status connect\n");
        bCutStatus = true;
    }
    else
    {
        if (g_init_debug_switch)
            dbg("(main Thread) Get hdmi status disconnect\n");
        bCutStatus = false;
    }

    if(bCutStatus == bOldStatus)
    {
        /* 启动前未接入(冷插入)和当前接口状态未变化 */
    	return;
    }

	/* 当前HDMI接口状态发送变化(热拔插) */
    if (bCutStatus)
    {   /* 检测到HDMI接口插入 */
        pthread_t PreviewHandleThdID;

        pstDock->HDMI_preview_flag = true;

        /* 检测到App实时预览未打开 */
        if (false == pstDock->app_preview_flag)
        {
            pthread_t PreviewForntThdID;

            /* 创建预览前端线程 */
            dbg("(main Thread) Create preview_fornt_thd\n\n");
            ret = pthread_create(&PreviewForntThdID, NULL, preview_fornt_thd, (void *)pstDock);
            if (ret)
            {
                int errno;
                char *err_msg = strerror(errno);

                msg("Can't Create thread %s, Error no: %d (%s)\n", "preview_fornt_thd", errno, err_msg);
                return;
            }
            /* 设置子线程为脱离状态 */
            pthread_detach(PreviewForntThdID);
        }

        /* 创建HDMI预览线程 */
        dbg("(main Thread) Create preview_handle_thd\n\n");
        ret = pthread_create(&PreviewHandleThdID, NULL, hdmi_preview_thd, (void *)pstDock);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "preview_fornt_thd", errno, err_msg);
            return;
        }
        /* 设置子线程为脱离状态 */
        pthread_detach(PreviewHandleThdID);

        /* 等待球机处理 */
        dbg("(main Thread) delay 1s for waiting hdmi_preview_thd\n\n");
        sleep(1);

        /* 发送球机开始预览命令 */
        send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);
    }
    else
    {   /* 检测到HDMI接口拔出 */
        pstDock->HDMI_preview_flag = false;

        if (false == pstDock->app_preview_flag)
        {
            pstDock->stop_data_flag = false;
        }

        /* 发送球机停止预览命令 */
        send_ball_cmd_pkt(BALL_CMD_STOP_PREVIEW, 0);
    }

    /* 保存HDMI接口状态 */
    bOldStatus = bCutStatus;
}


