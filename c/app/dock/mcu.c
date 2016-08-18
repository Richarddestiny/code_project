/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : mcu.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月15日
  最近修改   :
  功能描述   : MCU相关函数实现
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
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "include/dbg.h"
#include "include/mcu.h"
//#include "include/photo.h"
//include "include/video.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern u8 g_init_debug_switch;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
/* 待发送的MCU命令结构体变量 */
static struct mcu_packet m_stMcuCmdPkt;

/* MCU初始化完成信号量 */
static sem_t m_semMcuInitDone;

/* 发送MCU报文信号量 */
static sem_t m_semSendMcuPkt;

#if !MCU_DEBUG_SWITCH
/* 模拟MCU上报信号量 */
static sem_t m_semSimulateMcuUp;
#endif

/*****************************************************************************
 函 数 名  : mcu_type_to_str
 功能描述  : MCU报文类型转换字符串说明实现函数
 输入参数  : u8 ucType
 输出参数  : 无
 返 回 值  : char
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月21日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
char *mcu_type_to_str(u8 ucType)
{
    static char *acStr[] = {"keep alive", "volt query", "mode change",
    "shoot operate", "query volt", "change mode", "change power"};

    return acStr[ucType];
}

/*****************************************************************************
 函 数 名  : send_mcu_thd
 功能描述  : 发送MCU线程实现函数
 输入参数  : stConnectInfo *pConnectInfo
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void *send_mcu_thd(void *arg)
{
    int ret;
    ssize_t ulSendBytes;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    int iConnfd = pConnInfo->iConnfd;
    stDock *pstDock = pConnInfo->pstDock;

    dbg("(Send MCU Thread) Enter %s thread\n", "send_mcu_thd");

    /* 当所有程序退出时，此循环才退出，退出条件后续完善 */
    while (true)
    {
        /* 等待发送MCU命令时唤醒 */
        if (g_init_debug_switch)
            dbg("(Send MCU Thread) Sleep, Waiting Send MCU command\n\n");
        sem_wait(&m_semSendMcuPkt);

        if (g_init_debug_switch)
            dbg("(Send MCU Thread) Send MCU %s command\n", mcu_type_to_str(m_stMcuCmdPkt.type));

#if MCU_DEBUG_SWITCH
        /* 发送MCU命令报文 */
        ulSendBytes = send(iConnfd, &m_stMcuCmdPkt, sizeof(m_stMcuCmdPkt), 0);
    	if(ulSendBytes <= 0)
    	{
            int errno;
            char *err_msg = strerror(errno);

    		msg("Send MCU cmd packet failed, Error no: %d (%s)\n", errno, err_msg);
    	}
    	else
    	{
    		dbg("(Send MCU Thread) Send %d bytes MCU cmd packet\n\n", ulSendBytes);
    	}
#else
        ulSendBytes = 8;
        if (MCU_CMD_QUERY_VOLT == m_stMcuCmdPkt.type)
        {
            m_stMcuCmdPkt.type = MCU_ACK_VOLT_QUERY;
            m_stMcuCmdPkt.data = 100;
            sem_post(&m_semSimulateMcuUp);
        }
#endif
        if (ulSendBytes > 0)
        {
            if (g_init_debug_switch)
            {
                dbg("(Send MCU Thread) Send MCU %s command success, send bytes: %d\n", mcu_type_to_str(m_stMcuCmdPkt.type), (u32)ulSendBytes);
                dbg("\t\t\tSend packet magic: %s type: %d data: %d\n", m_stMcuCmdPkt.magic, m_stMcuCmdPkt.type, m_stMcuCmdPkt.data);
            }
        }
    }

    close(iConnfd);
    pthread_exit("Thread send_mcu_thd quit\n");
}

/*****************************************************************************
 函 数 名  : send_mcu_cmd_pkt
 功能描述  : 发送MCU命令报文实现函数
 输入参数  : u8 ucCmd
             u8 ucData
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void send_mcu_cmd_pkt(u8 ucCmd, u8 ucData)
{
    /* 初始化MCU命令报文 */
    memset(&m_stMcuCmdPkt, 0, sizeof(m_stMcuCmdPkt));

    /* 填充命令报文内容 */
	strcpy(m_stMcuCmdPkt.magic, "Mcmd");
    m_stMcuCmdPkt.type = ucCmd;
    m_stMcuCmdPkt.data = ucData;

    /* 唤醒等待的发送MCU线程，发送MCU命令 */
    if (g_init_debug_switch)
        dbg("\t\t\tWake up send_mcu_thd Thread\n");
    sem_post(&m_semSendMcuPkt);
}

/*****************************************************************************
 函 数 名  : recv_mcu_thd
 功能描述  : 接收MCU线程实现函数
 输入参数  : stConnectInfo *pConnectInfo
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void *recv_mcu_thd(void *arg)
{
    ssize_t ulRecvBytes;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    stDock *pstDock = pConnInfo->pstDock;
    struct mcu_packet stRecvMcuPkt;

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Enter %s thread\n", "recv_mcu_thd");

    /* 阻塞，直到接收到一个MCU报文 */
    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Blocking, Recieve MCU packet\n\n");
    memset((void *)&stRecvMcuPkt, 0, sizeof(struct mcu_packet));
#if MCU_DEBUG_SWITCH
    ulRecvBytes = recv(pConnInfo->iConnfd, &stRecvMcuPkt, sizeof(struct mcu_packet), 0);
    if (0 == ulRecvBytes)
    {
        msg("(Recv MCU Thread) Client close connect: %d\n", pConnInfo->iConnfd);
        dbg("packet magic: %s type: %d data: %d\n", stRecvMcuPkt.magic, stRecvMcuPkt.type, stRecvMcuPkt.data);
        goto end;
    }
    else if (ulRecvBytes < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Recv MCU Thread) Recieve data failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
#else
    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Simulate recieve MCU %s ack packet\n", mcu_type_to_str(m_stMcuCmdPkt.type));
    ulRecvBytes = 8;
	strcpy(stRecvMcuPkt.magic, "Mack");
    stRecvMcuPkt.type = m_stMcuCmdPkt.type;
    stRecvMcuPkt.data = m_stMcuCmdPkt.data;
#endif
    if (ulRecvBytes > 0)
    {
        if (g_init_debug_switch)
            dbg("(Recv MCU Thread) Recieve MCU %s command success, bytes: %d\n", mcu_type_to_str(stRecvMcuPkt.type), (u32)ulRecvBytes);
        //msg("packet magic: %s type: %d data: %d\n", stRecvMcuPkt.magic, stRecvMcuPkt.type, stRecvMcuPkt.data);
    }

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Check recieve packet magic: %s\n", stRecvMcuPkt.magic);
    /* 检查报文的识别key */
    if (0 != strncmp(stRecvMcuPkt.magic, "Mack", MCU_MAX_MAGIC_LEN))
    {
        msg("(Recv MCU Thread) Recieve error magic packet, magic:%s.\n", stRecvMcuPkt.magic);
        goto end;
    }

    if (g_init_debug_switch)
        dbg("(Recv MCU Thread) Switch by packet type: %s\n", mcu_type_to_str(stRecvMcuPkt.type));
    /* 根据报文的类型处理 */
    switch (stRecvMcuPkt.type)
    {
        /* 电量查询结果回应 */
        case MCU_ACK_VOLT_QUERY:
            /* 更新电量信息 */
            update_volt_info(pstDock, stRecvMcuPkt.data);

            if (SYS_STATUS_INIT == pstDock->ucSysStatus)
            {
                static u8 ucFirstFlag = 1;
                static u8 ucWaitBallDoneCount = 0;

                /* 第一次收到MCU发送的电量查询结果回应 */
                if (ucFirstFlag)
                {
                    msg("(Recv MCU Thread) First time recieve MCU volt query packet\n");
                    ucFirstFlag = 0;

                    /* 唤醒等待的主线程 system_init */
                    dbg("(Recv MCU Thread) Wake up main Thread\n");
                    sem_post(&m_semMcuInitDone);

                    /* 等待球机初始化处理 */
                    dbg("(Recv MCU Thread) delay 2s for waiting ball init done\n\n");
                    sleep(2);
                }

                dbg("(Recv MCU Thread) check adjust file\n");
                /* 检查标定文件 */
                if (dock_check_adjust_file(pstDock))
                {
                    static u8 ucUnadjustedFlag = 1;

                    /* 第一次发现未标定，创建工厂模式线程 */
                    if (ucUnadjustedFlag)
                    {
                        msg("First time dock is unadjusted, enter factory mode.\n");
                        //[add_code]此处需添加创建工厂模式线程的代码
                    }

                    /* 未标定情况，直接发送 切换电源灯命令 */
                    send_mcu_cmd_pkt(MCU_CMD_CHANG_POWER_LIGHT, LED_STATUS_OB_4);
                }

                /* 检查3次日期同步标志 */
                dbg("(Recv MCU Thread) check Ball date sync flag: %d\n", pstDock->bBallDateSyncFlag);
                if (pstDock->bBallDateSyncFlag)
                {
                    dbg("(Recv MCU Thread) Ball init done\n");

                    /* 后续初始化流程处理 */
                    dock_follow_up_init(pstDock);
                }
                else
                {
                    ucWaitBallDoneCount++;
                    if (ucWaitBallDoneCount > 3)
                    {
                        /* 日期未同步，球机初始化失败 */
                        set_sys_status(pstDock, SYS_STATUS_ERROR);
                    }
                }
            }
            break;

        /* 模式切换操作通知 */
        case MCU_OP_MODE_CHANG:
            change_shoot_mode(pstDock);
            break;

        /* 拍摄操作通知 */
        case MCU_OP_SHOOT_OPERA:
            if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
            {
                //photo_opera_handle(pstDock);
            }
            else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
            {
                //video_opera_handle(pstDock);
            }
            else
            {
                msg("Recieve MCU Shoot Mode value: %d error!\n", pstDock->ucShootMode);
            }
            break;

        default:
            msg("Recieve MCU packet type %d error.\n", stRecvMcuPkt.type);
    }

end:
    close(pConnInfo->iConnfd);
    free(pConnInfo);
    return NULL;
}

/*****************************************************************************
 函 数 名  : mcu_accept_loop
 功能描述  : 循环等待接受连接实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  : 无
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void mcu_accept_loop(stDock *pstDock)
{
    int ret;

    /* 监听端口，最多同时接受128个连接 */
    ret = listen(pstDock->iSock_mm, MAX_CONNECT_NUM);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Accept MCU Thread) Listen %s failed, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        return;
    }

    /* 当所有程序退出时，此循环才退出，退出条件后续完善 */
    while (true)
    {
        int iNewConnfd;
        struct sockaddr stNewClientAddr;
        socklen_t newAddrLen = sizeof(stNewClientAddr);
        stConnectInfo *pConnInfo;
        pthread_t RecvMcuThdID;

#if MCU_DEBUG_SWITCH
        /* 接受一个接收报文的连接 */
        iNewConnfd = accept(pstDock->iSock_mm, (struct sockaddr *)&stNewClientAddr, &newAddrLen);
        if (iNewConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept MCU Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            continue;
        }
#else
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Blocking, Accept MCU connect\n\n");
        iNewConnfd = 23;
        sem_wait(&m_semSimulateMcuUp);
#endif
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Accept a MCU connect: %d\n", iNewConnfd);

        /* 保存接收报文连接信息 */
        pConnInfo = calloc(1, sizeof(stConnectInfo));
        pConnInfo->pstDock = pstDock;
        pConnInfo->iConnfd = iNewConnfd;

        /* 创建接收MCU线程接收并且处理报文 */
        if (g_init_debug_switch)
            dbg("(Accept MCU Thread) Create thread recv_mcu_thd\n");
        ret = pthread_create(&RecvMcuThdID, NULL, recv_mcu_thd, (void *)pConnInfo);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept MCU Thread) Can't Create thread %s, Error no: %d (%s)\n", "recv_mcu_thd", errno, err_msg);
            continue;
        }
        /* 设置子线程为脱离状态 */
        pthread_detach(RecvMcuThdID);
    }
}

/*****************************************************************************
 函 数 名  : accept_mcu_thd
 功能描述  : 接收MCU连接线程实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月18日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
void *accept_mcu_thd(void *arg)
{
    int ret, iConnfd;
    char acIpAddr[MAX_IP_ADDR_LEN] = {0};
    stDock *pstDock = (stDock *)arg;
    pthread_t SendMCUThdID;
    pthread_t RecvMCUThdID;
    struct sockaddr stClient;
    socklen_t iClientAddrLen = sizeof(stClient);
    stConnectInfo *pConnInfo;
	struct sockaddr_in stSockMmAddr;

    dbg("(Accept MCU Thread) Enter %s thread\n", "accept_mcu_thd");

    /* 创建MCU使用的Socket: sock_mm */
    pstDock->iSock_mm = socket(AF_INET, SOCK_STREAM, 0);
    if (pstDock->iSock_mm < 0)
    {
        int errno;
        char *err_msg = strerror(errno);
        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        exit(2);
    }
    dbg("(Accept MCU Thread) Create Socket %s OK, Socket: %d\n", "sock_mm", pstDock->iSock_mm);

    /* 获取 eth0 接口IP地址 */
    if (NULL == get_netif_ip("eth0", acIpAddr))
    {
        msg("Can't get %s IP address, set default IP address: %s\n", "eth0", SOCK_MM_IP_ADDR);
        strncpy(acIpAddr, SOCK_MM_IP_ADDR, MAX_IP_ADDR_LEN);
    }

    /* 设置服务端的IP地址和端口号 */
    memset(&stSockMmAddr, 0, sizeof(stSockMmAddr));
    stSockMmAddr.sin_family = AF_INET;
    inet_pton(AF_INET, acIpAddr, &stSockMmAddr.sin_addr);
    stSockMmAddr.sin_port = htons(SOCK_MM_PORT);

    /* 绑定IP地址和端口 */
    ret = bind(pstDock->iSock_mm, (struct sockaddr *)&stSockMmAddr, sizeof(stSockMmAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", acIpAddr, SOCK_MM_PORT, errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Bind to %s:%d success\n", acIpAddr, SOCK_MM_PORT);

#if MCU_DEBUG_SWITCH
    /* 监听端口，仅接受1个连接 */
    ret = listen(pstDock->iSock_mm, 0);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Listen %s failed, Error no: %d (%s)\n", "sock_mm", errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Listen to %s:%d\n", acIpAddr, SOCK_MM_PORT);

    /* 阻塞，直到在监听队列接受到第一个连接 */
    iConnfd = accept(pstDock->iSock_mm, (struct sockaddr *)&stClient, &iClientAddrLen);
    if (iConnfd < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
    dbg("(Accept MCU Thread) Accept a connect: %d\n", iConnfd);
#else
    iConnfd = 123;
#endif

    /* 保存连接信息 */
    pConnInfo = calloc(1, sizeof(stConnectInfo));
    pConnInfo->pstDock = pstDock;
    pConnInfo->iConnfd = iConnfd;

    /* 创建发送MCU线程 send_mcu_thd */
    dbg("(Accept MCU Thread) Create thread %s\n\n", "send_mcu_thd");
    ret = pthread_create(&SendMCUThdID, NULL, send_mcu_thd, (void *)pConnInfo);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "send_mcu_thd", errno, err_msg);
        goto end;
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(SendMCUThdID);

    /* 等待发送MCU线程创建完成  */
    sleep(1);

    /* 发送查询电量命令 */
    dbg("(Accept MCU Thread) Send Query Vole command\n");
    send_mcu_cmd_pkt(MCU_CMD_QUERY_VOLT, 0);

    /* 循环等待接受连接 */
    mcu_accept_loop(pstDock);

end:
    /* 关闭sock_ac，关闭线程 */
    close(pstDock->iSock_mm);
    pthread_exit("Thread accept_mcu_thd quit\n");
    return NULL;
}

/*****************************************************************************
 函 数 名  : mcu_init
 功能描述  : MCU初始化实现函数
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
void mcu_init(stDock *pstDock)
{
    int ret;
    pthread_t AcceptMCUThdID;

    /* 初始化MCU模块内部使用的信号量 */
    sem_init(&m_semSendMcuPkt, 0, 0);
    sem_init(&m_semMcuInitDone, 0, 0);
#if !MCU_DEBUG_SWITCH
    sem_init(&m_semSimulateMcuUp, 0, 0);
#endif

    /* 创建接收MCU连接线程 accept_mcu_thd */
    dbg("(main Thread) Create thread %s\n", "accept_mcu_thd");
    ret = pthread_create(&AcceptMCUThdID, NULL, accept_mcu_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);
        msg("Can't Create thread %s, Error no: %d (%s)\n", "accept_mcu_thd", errno, err_msg);
        return;
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(AcceptMCUThdID);

    /* 等待MCU初始化完成 */
    dbg("(main Thread) Sleep, Waiting MCU init done\n\n");
    sem_wait(&m_semMcuInitDone);

    dbg("(main Thread) Wake up, MCU init done!\n");
}

/*****************************************************************************
 函 数 名  : set_power_status
 功能描述  : 设置电源灯状态实现函数
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
void set_power_status(stDock *pstDock)
{
    u8 ucPowerStatus;

    /* 根据系统状态获取电源灯状态 */
    switch (pstDock->ucSysStatus)
    {
        /* 就绪状态 */
        case SYS_STATUS_READY:
            if (pstDock->ucChargeFlag)
            {   /* 充电中 */
                ucPowerStatus = LED_STATUS_OS;
                break;
            }

            if (pstDock->ucVoltCent < 20)
            {   /* 电量不足 */
                ucPowerStatus = LED_STATUS_RS;
            }
            else
            {   /* 电量充足 */
                ucPowerStatus = LED_STATUS_GS;
            }
            break;

        /* 正在录像状态 */
        case SYS_STATUS_VIDEO:
        /* 正在拍照状态与正在录像状态相同 */
        case SYS_STATUS_PHOTO:
            if (pstDock->ucChargeFlag)
            {   /* 充电中 */
                ucPowerStatus = LED_STATUS_OB_1;
                break;
            }

            if (pstDock->ucVoltCent < 20)
            {   /* 电量不足 */
                ucPowerStatus = LED_STATUS_RB_1;
            }
            else
            {   /* 电量充足 */
                ucPowerStatus = LED_STATUS_GB_1;
            }
            break;

        /* 出错状态 */
        case SYS_STATUS_ERROR:
            ucPowerStatus = LED_STATUS_RB_4;
            break;
        default:
            msg("System status value:%d error!\n", pstDock->ucSysStatus);
    }

    /* 发送 切换电源灯命令 */
    send_mcu_cmd_pkt(MCU_CMD_CHANG_POWER_LIGHT, ucPowerStatus);

}

/*****************************************************************************
 函 数 名  : set_power_status
 功能描述  : 设置模式灯状态实现函数
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
void set_mode_status(stDock *pstDock)
{
    u8 ucModeStatus;

    /* 根据拍摄模式获取模式灯状态 */
    if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
    {
        ucModeStatus = LED_STATUS_GS;
    }
    else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
    {
        ucModeStatus = LED_STATUS_OS;
    }
    else
    {
        msg("Change shoot Mode value: %d error!\n", pstDock->ucShootMode);
        return;
    }

    /* 发送 切换模式灯命令 */
    send_mcu_cmd_pkt(MCU_CMD_CHANG_MODE_LIGHT, ucModeStatus);
}

