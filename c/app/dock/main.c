/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : main.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月14日
  最近修改   :
  功能描述   : 主线程函数实现
  函数列表   :
              main
  修改历史   :
  1.日    期   : 2016年4月14日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含自定义头文件                             *
 *----------------------------------------------*/
#include "include/main.h"
#include "include/dbg.h"
#include "include/mcu.h"
#include "include/app.h"
#include "include/ball.h"
#include "include/stitch.h"
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
static stDock m_stDock;

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

/*****************************************************************************
 函 数 名  : update_volt_info
 功能描述  : 更新电量信息实现函数
 输入参数  : stDock *pstDock
             u8 ucVoltInfo
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月19日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void update_volt_info(stDock *pstDock, u8 ucVoltInfo)
{
    u8 ucVoltCent;
    u8 ucChargeFlag;

    /* 低7位为电量百分数 */
    ucVoltCent = ucVoltInfo & 0x7f;

    /* 高1位为是否充电标志 */
    ucChargeFlag = (ucVoltInfo >> 7) & 0x1;

    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucVoltCent = ucVoltCent;
    pstDock->ucChargeFlag = ucChargeFlag;
    pthread_mutex_unlock(&pstDock->mutex);
}

/*****************************************************************************
 函 数 名  : set_sys_status
 功能描述  : 设置系统状态实现函数
 输入参数  : stDock *pstDock
             SysStatusVaule ucStatus
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void set_sys_status(stDock *pstDock, SysStatusVaule ucStatus)
{
    int ret;

    /* 错误系统状态处理(不能设置成初始化状态) */
    if ((!ucStatus) || (ucStatus > SYS_STATUS_ERROR))
    {
        msg("System status value:%d error!\n", ucStatus);
    }

    /* 设置全局变量系统状态 */
    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucSysStatus = ucStatus;
    pthread_mutex_unlock(&pstDock->mutex);

    /* 设置电源灯状态 */
    set_power_status(pstDock);
}

/*****************************************************************************
 函 数 名  : change_shoot_mode
 功能描述  : 切换拍摄模式实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月19日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void change_shoot_mode(stDock *pstDock)
{
    u8 ucNewMode;

    /* 当前系统状态为正在录像状态，切换拍摄模式失败 */
    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        /* 正在录像时切换拍摄模式，进入出错状态 */
        set_sys_status(pstDock, SYS_STATUS_ERROR);
        return;
    }

    /* 根据当前模式，切换拍摄模式 */
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

    /* 设置全局变量拍摄模式 */
    pthread_mutex_lock(&pstDock->mutex);
    pstDock->ucShootMode = ucNewMode;
    pthread_mutex_unlock(&pstDock->mutex);

    /* 设置模式灯状态 */
    set_mode_status(pstDock);

	/*唤醒球机控制线程 ,发送球机模式切换命令*/
    send_ball_cmd_pkt(BALL_CMD_CHANGE_MODE, 0);
}

/*****************************************************************************
 函 数 名  : dock_check_adjust_file
 功能描述  : Dock板检查标定文件实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
int dock_check_adjust_file(stDock *pstDock)
{
    int ret = 0;
    int result = 1;

    /* 对应位置查找标定文件 */

    if (!ret)
    {
        /* 查询结果0表示找到标定文件 */
        result = 0;
    }

    return result;
}

/*****************************************************************************
 函 数 名  : live_init
 功能描述  : 直播初始化实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void live_init(stDock *pstDock)
{
    /* 创建直播控制线程 */

    /* 创建直播数据线程 */
}

/*****************************************************************************
 函 数 名  : follow_up_init
 功能描述  : Dock板后续初始化流程实现
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void dock_follow_up_init(stDock *pstDock)
{
    /* 直播初始化 */
    live_init(pstDock);
    dbg("(Recv MCU Thread) live init done\n");

    /* App初始化 */
    app_init(pstDock);
    dbg("(Recv MCU Thread) app init done\n");

    /* 所有模块初始化完成，进入就绪状态 */
    set_sys_status(pstDock, SYS_STATUS_READY);
    g_init_debug_switch = DEBUG_OFF;
    dbg("(Recv MCU Thread) All system init done, system status ready!\n\n\n");
}

/*****************************************************************************
 函 数 名  : get_dock
 功能描述  : 获取Dock结构体实现函数
 输入参数  : void
 输出参数  : 无
 返 回 值  : stDock
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月27日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
stDock *get_dock(void)
{
    return &m_stDock;
}

/*****************************************************************************
 函 数 名  : dock_init
 功能描述  : Dock板初始化实现函数
 输入参数  : void
 输出参数  : stDock *
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void dock_init(void)
{
    int i;
    stDock *pstDock = get_dock();

    memset((void *)pstDock, 0, sizeof(stDock));

    /* Dock板结构体锁初始化 */
    pthread_mutex_init(&pstDock->mutex, NULL);

    /* 全局状态初始化 */
    pstDock->ucSysStatus = SYS_STATUS_INIT;
    pstDock->ucShootMode = SHOOT_MODE_PHOTO;

    /* [add_code]拍摄参数初始化 */

    /* 预览标志位初始化 */
    pstDock->stop_data_flag = STATUS_STOP;      /* 预览前端开启停止标志 */
    pstDock->HDMI_preview_flag = STATUS_STOP;
    pstDock->app_preview_flag = STATUS_STOP;

    /* 完成标志初始化 */
    pstDock->decode_done_flag = false;
    pstDock->stitch_done_flag = false;
    pstDock->encode_done_flag = false;
    pstDock->HDMI_send_done_flag = false;
    pstDock->app_send_done_flag = false;

    /* 信号量初始化 */
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
 函 数 名  : get_netif_ip
 功能描述  : 获取网络接口IP地址实现函数
 输入参数  : char* pIfName
             char *pIpBuf
 输出参数  : 无
 返 回 值  : char
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月24日
    作    者   : 彭斌全
    修改内容   : 新生成函数

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
 函 数 名  : system_init
 功能描述  : 系统初始化实现函数，同时作为初始化调试线程使用
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void *
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void *system_init(void *arg)
{
    stDock *pstDock = get_dock();

    /* Dock初始化 */
    dock_init();

    /* MCU初始化 */
    mcu_init(pstDock);

    /* 球机初始化 */
    ball_init(pstDock);

    while (true)
    {
        /* 心跳等待5s */
        sleep(5);

        /* 发送MCU心跳包(查询电量命令) */
        send_mcu_cmd_pkt(MCU_CMD_QUERY_VOLT, 0);

        /* 发送球机心跳包 */
        send_ball_cmd_pkt(BALL_CMD_KEEP_ALIVE, 0);

        /* HDMI接口热拔插检测 */
        hdmi_status_check(pstDock);
    }
}

/*****************************************************************************
 函 数 名  : main
 功能描述  : 主线程实现函数
 输入参数  : int argc
             char **argv
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月14日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
int main(int argc, char **argv)
{
    /* 设置系统日志 */
    //set_system_log();

    /* 打开调试控制台 */
    dbg_console();

    /* 系统初始化 */
    system_init(NULL);
}

