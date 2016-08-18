/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : app.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月15日
  最近修改   :
  功能描述   : App相关函数实现
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "include/app.h"

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
/* App命令列表 */
static app_cmd m_app_cmd_list[MAX_APP_CMD_NUM + 1] = {
    {APP_CMD_QUERY_VOLT,      	APP_ACK_QUERY_VOLT,     query_volt_handle},         /* 查询电量命令 */
    {APP_CMD_QUERY_DATE,      	APP_ACK_QUERY_DATE,     query_date_handle},         /* 查询日期命令 */
    {APP_CMD_QUERY_STATUS,    	APP_ACK_QUERY_STATUS,   query_status_handle},       /* 查询状态命令 */
    {APP_CMD_QUERY_PARA,      	APP_ACK_QUERY_PARA,     query_para_handle},         /* 查询参数命令 */
    {APP_CMD_QUERY_CAPACITY,  	APP_ACK_QUERY_CAPACITY, query_capacity_handle},     /* 查询容量命令 */
    {APP_CMD_QUERY_TIME,      	APP_ACK_QUERY_TIME,     query_time_handle},         /* 查询时间命令 */
    {APP_CMD_QUERY_PROG,      	APP_ACK_QUERY_PROG,     query_prog_handle},         /* 查询进度命令 */
    {APP_CMD_CHANGE_MODE,     	APP_ACK_QUERY_STATUS,   change_mode_handle},        /* 模式切换命令 */
    {APP_CMD_SHOOT_OPERATE,   	APP_ACK_QUERY_STATUS,   shoot_operare_handle},      /* 拍摄操作命令 */
    {APP_CMD_PREVIEW_OPERATE, 	APP_ACK_QUERY_STATUS,   preview_operare_handle},    /* 预览操作命令 */
    {APP_CMD_SPLICE_VIDEO,    	APP_ACK_QUERY_PROG,     splice_video_handle},       /* 录像拼接命令 */
    {APP_CMD_SET_PARA,          APP_ACK_QUERY_PARA,     set_para_handle},           /* 设置参数命令 */
    {0,                         0,                      NULL},
};

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
#define APP_PREVIEW_DEBUG       1

/*****************************************************************************
 函 数 名  : app_preview_thd
 功能描述  : App预览线程实现函数
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void *
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void *app_preview_thd(void *arg)
{
    int ret, iSock_ad, iConnfd;
    ssize_t ulSendBytes;
    stDock *pstDock = (stDock *)arg;
    struct sockaddr_in stSockAdAddr;
    struct sockaddr stClient;
    socklen_t iClientAddrLen = sizeof(stClient);

    dbg("(App Preview Thread) Enter %s\n", __func__);

    /* 创建App数据使用的Socket: sock_ad */
    iSock_ad = socket(AF_INET, SOCK_STREAM, 0);
    if (iSock_ad < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_ad", errno, err_msg);
        pthread_exit("Thread app_preview_thd quit\ns");
        return NULL;
    }
    dbg("(App Preview Thread) Create Socket %s OK, Socket: %d\n", "sock_ad", iSock_ad);

    /* 设置服务端的IP地址和端口号 */
    memset(&stSockAdAddr, 0, sizeof(stSockAdAddr));
    stSockAdAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SOCK_AD_IP_ADDR, &stSockAdAddr.sin_addr);
    stSockAdAddr.sin_port = htons(SOCK_AD_PORT);

    /* 绑定IP地址和端口 */
    ret = bind(iSock_ad, (struct sockaddr *)&stSockAdAddr, sizeof(stSockAdAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", SOCK_AD_IP_ADDR, SOCK_AD_PORT, errno, err_msg);
        goto end;
    }
    dbg("(App Preview Thread) Bind to %s:%d success\n", SOCK_AD_IP_ADDR, SOCK_AD_PORT);

    if (g_app_preview_debug_switch)
    {
        /* 监听端口，仅接受1个连接 */
        ret = listen(iSock_ad, 0);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(App Preview Thread) Listen %s failed, Error no: %d (%s)\n", "sock_ad", errno, err_msg);
            goto end;
        }

        /* 阻塞，直到在监听队列接受到一个连接 */
        dbg("(Accept App Thread) Blocking, Accept App data connect\n\n");
        iConnfd = accept(iSock_ad, (struct sockaddr *)&stClient, &iClientAddrLen);
        if (iConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(App Preview Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            goto end;
        }
        dbg("(App Preview Thread) Accept a connect: %d\n", iConnfd);
    }


    while (true)
    {
        /* 设置App预览数据发送完成标志 */
        pstDock->app_send_done_flag = true;

        /* 睡眠，发送预览数据时, 由编码线程 encode_handle_thd 唤醒 */
        //dbg("(App Preview Thread) Sleep, Waiting Send preview data\n\n");
        sem_wait(&pstDock->sem_app_preview);

        //dbg("(App Preview Thread) wake up, Send preview data\n");

        /* 发送预览帧头 */
       // if (g_app_preview_debug_switch)
       //     ulSendBytes = sizeof(frame_head);
       // else
	        ulSendBytes = send(iConnfd, &pstDock->stAppFrameHead, sizeof(frame_head), 0);
    	if(ulSendBytes <= 0)
    	{
	        int errno;
            char *err_msg = strerror(errno);

    		msg("Send app preview Frame Head failed, Error no: %d (%s)\n", errno, err_msg);
    		continue;
    	}
    	else
    	{
    		dbg("(App Preview Thread) Send %d bytes app preview Frame Head\n", ulSendBytes);
    		dbg("\t\t\tFrame ID: %u len: %u Interval: %u\n", pstDock->stAppFrameHead.uiID,
    		    pstDock->stAppFrameHead.uiLen, pstDock->stAppFrameHead.uiInterval);
    	}

        /* 发送预览数据 */
        if (g_app_preview_debug_switch)
            ulSendBytes = pstDock->stAppFrameHead.uiLen;
        else
    	    ulSendBytes = send(iConnfd, pstDock->pAppPreviewBuffer, pstDock->stAppFrameHead.uiLen, 0);
    	if(ulSendBytes <= 0)
    	{
	        int errno;
            char *err_msg = strerror(errno);

    		msg("(App Preview Thread) Send app preview Frame data failed, Error no: %d (%s)\n", errno, err_msg);
    		continue;
    	}
    	else
    	{
    		dbg("Send %d bytes app preview Frame data\n\n", ulSendBytes);
    	}
    }

end:
    /* 关闭sock_ad，关闭线程 */
    close(iSock_ad);
    pthread_exit("Thread app_preview_thd quit\n");
    return NULL;
}

/*****************************************************************************
 函 数 名  : app_type_to_str
 功能描述  : App命令和回复报文类型转换字符串说明实现函数
 输入参数  : u8 ucType
 输出参数  : 无
 返 回 值  : char
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月23日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
char *app_type_to_str(u8 ucType)
{
    static char *acStr[] = {"unknown", "query volt", "query date", "query status",
    "query parameter", "query capacity", "query time", "query progress", "change mode",
    "shoot operate", "preview operate", "splice video", "set parameter"};

    return acStr[ucType];
}

/*****************************************************************************
 函 数 名  : query_volt_handle
 功能描述  : App查询电量命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : u64
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月25日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_volt_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 因为主线程已经在不断查询电量，这里仅读取电量信息即可 */
    unOutputData.acData[0] = pstDock->ucVoltCent;   /* 电量百分数（0~100， 20表示当前电量20%） */
    unOutputData.acData[1] = pstDock->ucChargeFlag; /* 是否在充电标志（0：未充电；1：充电中） */

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_date_handle
 功能描述  : App查询日期命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : u64
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月25日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_date_handle(stDock *pstDock, u64 ullInputData)
{
    u32 uiTimeStamp;
    time_t CurrTime;
    unData unOutputData;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    uiTimeStamp = ((unData)ullInputData).aiData[0];

    /* 输入时间戳不为0，表示设置球机时间，返回0 */
    if (uiTimeStamp)
    {
		/* 发送日期操作命令，同步设置球机日期 */
	    send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, uiTimeStamp);
	    return 0;
    }

    /* 输入时间戳为0，表示读取Dock板当前时间 */
    time(&CurrTime);
    unOutputData.aiData[0] = CurrTime;

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_status_handle
 功能描述  : App查询状态命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月26日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_status_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 获取系统状态：[0]拍摄模式；[1]录像状态；[2]预览状态；[3]错误码 */
    unOutputData.acData[0] = pstDock->ucShootMode;
    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        unOutputData.acData[1] = STATUS_DOING;
    }
    else if (SYS_STATUS_READY == pstDock->ucSysStatus)
    {
        unOutputData.acData[1] = STATUS_STOP;
    }

    unOutputData.acData[2] = pstDock->app_preview_flag;
    unOutputData.acData[3] = pstDock->ucErrorCode;

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_para_handle
 功能描述  : App查询参数命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月26日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_para_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 获取拍摄参数：[0]曝光补偿值；[1]白平衡模式；[2]分辨率 */
    unOutputData.aiData[0] = pstDock->ucExpCompValue;
    unOutputData.aiData[1] = pstDock->ucWhiteBalMode;
    unOutputData.aiData[2] = pstDock->ucResolution;

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_capacity_handle
 功能描述  : App查询容量命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月26日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_capacity_handle(stDock *pstDock, u64 ullInputData)
{
    u16 usAvailCapacity;
    u64 ullBlockNum;
    u64 ullAvailableDisk;
    unData unOutputData;
    struct statfs diskInfo;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 获取剩余容量 */
    statfs("/usr/data/", &diskInfo);
    ullBlockNum = diskInfo.f_bsize;        //每个block里包含的字节数
    ullAvailableDisk = diskInfo.f_bavail * ullBlockNum;   //可用空间大小
    unOutputData.asData[0] = ullAvailableDisk >> 20;

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_time_handle
 功能描述  : App查询时间命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月30日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_time_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;
    u16 usAvailCapacity;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 获取剩余可用空间，单位：MB */
    unOutputData.ullData = query_capacity_handle(pstDock, 0);
    usAvailCapacity = unOutputData.asData[0];

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 剩余录像时间计算公式 */
    /* remain_time = avail_capacity / (fps * frame_max_size * 4) */
    /* FPS为25帧/秒，最大帧大小取经验值50KB，所以剩余可用空间需要转换成KB */
    unOutputData.aiData[1] = ((u32)usAvailCapacity << 10) / (25 * 50 * 4);

	/* 发送读取录制时间命令 */
    send_ball_cmd_pkt(BALL_CMD_READ_VIDEO_TIME, 0);

    /* 睡眠等待，由线程 app_cmd_handle_thd 唤醒 */
	sem_wait(&pstDock->semRecordTime);

    unOutputData.aiData[0] = pstDock->uiRecordTime;

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : query_prog_handle
 功能描述  :  App查询进度命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月30日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 query_prog_handle(stDock *pstDock, u64 ullInputData)
{
    unData unOutputData;

    /* 初始化输出数据 */
    unOutputData.ullData = 0;

    /* 拼接进度百分数计算公式 */
    /* 拼接进度百分数 = (current_frame_count * 100) / (fps * video_sec) */
    unOutputData.acData[0] = (pstDock->uiFrameCount * 100) / (25 * pstDock->uiVideoSecond);

    return unOutputData.ullData;
}

/*****************************************************************************
 函 数 名  : change_mode_handle
 功能描述  : App模式切换命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月30日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 change_mode_handle(stDock *pstDock, u64 ullInputData)
{
    /* 切换拍摄模式操作 */
    change_shoot_mode(pstDock);

    /* 返回状态查询回复 */
    return query_status_handle(pstDock, 0);;
}

/*****************************************************************************
 函 数 名  : shoot_operare_handle
 功能描述  : App拍摄操作命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月30日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 shoot_operare_handle(stDock *pstDock, u64 ullInputData)
{
    if (SHOOT_MODE_PHOTO == pstDock->ucShootMode)
    {
        photo_opera_handle(pstDock);
    }
    else if (SHOOT_MODE_VIDEO == pstDock->ucShootMode)
    {
        video_opera_handle(pstDock);
    }
    else
    {
        msg("Recieve MCU Shoot Mode value: %d error!\n", pstDock->ucShootMode);
    }

    /* 返回状态查询回复 */
    return query_status_handle(pstDock, 0);;
}

/*****************************************************************************
 函 数 名  : preview_operare_handle
 功能描述  : App预览操作命令处理实现函数
 输入参数  : stDock *pstDock
             u64 ullInputData
 输出参数  : 无
 返 回 值  : static
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月25日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
u64 preview_operare_handle(stDock *pstDock, u64 ullInputData)
{
    int ret;
    unData unOutputData;

    dbg("(App cmd handle Thread) App preview operare \n");

    if (STATUS_STOP == pstDock->app_preview_flag)
    {   /* App预览停止时 */
        pthread_t EncodeHandleThdID;

        /* 打开App预览 */
        pstDock->app_preview_flag = STATUS_DOING;

        if (DEBUG_ON == g_app_preview_debug_switch)
        {
            pthread_t BallDataThdID;

            dbg("(App cmd handle Thread) Open App preview\n");

            pstDock->stop_data_flag = STATUS_DOING;
            pstDock->app_send_done_flag = true;

            /* 创建球机数据线程 */
            dbg("(App cmd handle Thread) Create ball_data_thd\n\n");
            ret = pthread_create(&BallDataThdID, NULL, ball_data_thd, (void *)pstDock);
            if (ret)
            {
                int errno;
                char *err_msg = strerror(errno);

                msg("Can't Create thread %s, Error no: %d (%s)\n", "ball_data_thd", errno, err_msg);
                return ret;
            }
            /* 设置子线程为脱离状态 */
            pthread_detach(BallDataThdID);

            /* 发送开始预览命令到球机 */
            dbg("(App cmd handle Thread) Send %d command to Ball\n", BALL_CMD_START_PREVIEW);
            send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);

            return 0;
        }

        /* 检测到HDMI预览未打开 */
        if (false == pstDock->HDMI_preview_flag)
        {
            pthread_t PreviewForntThdID;

            /* 创建预览前端线程 */
            dbg("(App cmd handle Thread) Create preview_fornt_thd\n\n");
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

        /* 创建编码处理线程 */
        dbg("(App cmd handle Thread) Create encode_handle_thd\n\n");
        ret = pthread_create(&EncodeHandleThdID, NULL, encode_handle_thd, (void *)pstDock);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "encode_handle_thd", errno, err_msg);
            return;
        }
        /* 设置子线程为脱离状态 */
        pthread_detach(EncodeHandleThdID);

        /* 等待球机处理 */
        dbg("(App cmd handle Thread) delay 1s for waiting hdmi_preview_thd\n\n");
        sleep(1);

        /* 发送球机开始预览命令 */
        send_ball_cmd_pkt(BALL_CMD_START_PREVIEW, 0);
    }
    else if (STATUS_DOING == pstDock->app_preview_flag)
    {   /* App预览正在进行时 */

        /* 停止App预览 */
        pstDock->app_preview_flag = STATUS_STOP;

        /* 检测到HDMI预览未打开 */
        if (STATUS_STOP == pstDock->HDMI_preview_flag)
        {
            pstDock->stop_data_flag = STATUS_STOP;
        }

        /* 发送停止预览命令到球机 */
        dbg("(App cmd handle Thread) Send %d command to Ball\n", BALL_CMD_STOP_PREVIEW);
        send_ball_cmd_pkt(BALL_CMD_STOP_PREVIEW, 0);
    }

    if (DEBUG_ON == g_app_preview_debug_switch)
        return 0;
ack:
    /* 获取系统状态：[0]拍摄模式；[1]录像状态；[2]预览状态；[3]错误码 */
    unOutputData.aiData[0] = pstDock->ucShootMode;

    if (SYS_STATUS_VIDEO == pstDock->ucSysStatus)
    {
        unOutputData.aiData[1] = STATUS_DOING;
    }
    else if (SYS_STATUS_READY == pstDock->ucSysStatus)
    {
        unOutputData.aiData[1] = STATUS_STOP;
    }
    else
    {
        /* [add_code]这里添加错误处理 */
    }

    unOutputData.aiData[2] = pstDock->app_preview_flag;
    unOutputData.aiData[3] = pstDock->ucErrorCode;

    return unOutputData.ullData;
}

u64 splice_video_handle(stDock *pstDock, u64 ullInputData)
{

}

u64 set_para_handle(stDock *pstDock, u64 ullInputData)
{

}

/*****************************************************************************
 函 数 名  : app_cmd_handle_thd
 功能描述  : App命令处理线程实现函数
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void *
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void *app_cmd_handle_thd(void *arg)
{
    int i;
    stConnectInfo *pConnInfo = (stConnectInfo *)arg;
    stDock *pstDock = pConnInfo->pstDock;
    ssize_t ulRecvBytes, ulSendBytes;
    struct app_packet stRecvAppPkt, stSendAppPkt;

    dbg("(App cmd handle Thread) Enter %s\n", __func__);

    /* 接收App报文 */
    memset((void *)&stRecvAppPkt, 0, sizeof(struct app_packet));
    ulRecvBytes = recv(pConnInfo->iConnfd, &stRecvAppPkt, sizeof(struct app_packet), 0);
    if (0 == ulRecvBytes)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(App cmd handle Thread) Client close connect, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }
    else if (ulRecvBytes < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(App cmd handle Thread) Recieve data failed, Error no: %d (%s)\n", errno, err_msg);
        goto end;
    }

    dbg("(App cmd handle Thread) Recv App %s command success, recieved %d bytes\n", app_type_to_str(stRecvAppPkt.type), (u32)ulRecvBytes);
    dbg("Recv packet, Magic: %s Type: %d, Data: %lld\n", stRecvAppPkt.magic, stRecvAppPkt.type, stRecvAppPkt.data.ullData);

    /* 检查报文的识别key */
    if (strncmp(stRecvAppPkt.magic, "Acmd", APP_MAX_MAGIC_LEN))
    {
        msg("(App cmd handle Thread) Recieve error magic packet, magic: %s\n", stRecvAppPkt.magic);
        goto end;
    }

    /* 根据接收的报文的类型处理，同时填充好回应报文内容 */
    for (i = 0; i < MAX_APP_CMD_NUM; i++) {
        /* 根据命令类型匹配 */
        if (m_app_cmd_list[i].cmd_type != stRecvAppPkt.type)
            continue;

        /* 填充命令报文内容 */
        strcpy(stSendAppPkt.magic, "Aack");
        stSendAppPkt.type = m_app_cmd_list[i].ack_type;

        /* 调用对应命令处理函数，执行命令，填充回复数据 */
        stSendAppPkt.data.ullData = m_app_cmd_list[i].cmd_func(pstDock, stRecvAppPkt.data.ullData);
    }

    /* 发送App回复报文 */
    ulSendBytes = send(pConnInfo->iConnfd, &stSendAppPkt, sizeof(stSendAppPkt), 0);
	if(ulSendBytes <= 0)
	{
        int errno;
        char *err_msg = strerror(errno);

		msg("Send App ack packet failed, Error no: %d (%s)\n", errno, err_msg);
	}
	else
	{
		dbg("(App cmd handle Thread) Send App ack success, Send %d bytes\n\n", ulSendBytes);
		dbg("\tpacket magic: %s\n\ttype: %s\n", stSendAppPkt.magic, app_type_to_str(stSendAppPkt.type));
	}

end:
    close(pConnInfo->iConnfd);
    free(pConnInfo);
    return NULL;
}

/*****************************************************************************
 函 数 名  : accept_app_thd
 功能描述  : 接受App连接线程实现函数
 输入参数  : void *arg
 输出参数  : 无
 返 回 值  : void *
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月15日
    作    者   : 彭斌全
    修改内容   : 新生成函数

*****************************************************************************/
static void *accept_app_thd(void *arg)
{
    int ret, iSock_ac;
    stDock *pstDock = (stDock *)arg;
    struct sockaddr_in stSockAcAddr;

    dbg("(Accept App Thread) Enter %s\n", __func__);

    /* 创建App命令使用的Socket: sock_ac */
    iSock_ac = socket(AF_INET, SOCK_STREAM, 0);
    if (iSock_ac < 0)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create Socket %s, Error no: %d (%s)\n", "sock_ac", errno, err_msg);
        pthread_exit("Thread accept_app_thd quit\ns");
        return NULL;
    }
    dbg("(Accept App Thread) Create Socket %s OK, Socket: %d\n", "sock_ac", iSock_ac);

    /* 设置服务端的IP地址和端口号 */
    memset(&stSockAcAddr, 0, sizeof(stSockAcAddr));
    stSockAcAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SOCK_AC_IP_ADDR, &stSockAcAddr.sin_addr);
    stSockAcAddr.sin_port = htons(SOCK_AC_PORT);

    /* 绑定IP地址和端口 */
    ret = bind(iSock_ac, (struct sockaddr *)&stSockAcAddr, sizeof(stSockAcAddr));
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Bind to %s:%d, Error no: %d (%s)\n", SOCK_AC_IP_ADDR, SOCK_AC_PORT, errno, err_msg);
        goto end;
    }
    dbg("(Accept App Thread) Bind to %s:%d success\n", SOCK_AC_IP_ADDR, SOCK_AC_PORT);

    /* 监听端口，最多同时接受128个连接 */
    ret = listen(iSock_ac, 128);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(Accept App Thread) Listen %s failed, Error no: %d (%s)\n", "sock_ac", errno, err_msg);
        goto end;
    }

    /* 当所有程序退出时，此循环才退出，退出条件后续完善 */
    while (true)
    {
        int iNewConnfd;
        struct sockaddr stNewClientAddr;
        socklen_t iClientAddrLen = sizeof(stNewClientAddr);
        pthread_t AppCmdThdID;
        stConnectInfo *pConnInfo;

        /* 阻塞，直到在监听队列接受到一个连接 */
        dbg("(Accept App Thread) Blocking, Accept App command connect\n\n");
        iNewConnfd = accept(iSock_ac, (struct sockaddr *)&stNewClientAddr, &iClientAddrLen);
        if (iNewConnfd < 0)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("(Accept App Thread) Accept connect failed, Error no: %d (%s)\n", errno, err_msg);
            continue;
        }
        dbg("(Accept App Thread) Accept a connect: %d\n", iNewConnfd);

        /* 保存接收报文连接信息 */
        pConnInfo = calloc(1, sizeof(stConnectInfo));
        pConnInfo->pstDock = pstDock;
        pConnInfo->iConnfd = iNewConnfd;

        /* 创建App命令处理线程接收并且处理报文 */
        dbg("(Accept App Thread) Create thread app_cmd_handle_thd \n");
        ret = pthread_create(&AppCmdThdID, NULL, app_cmd_handle_thd, (void *)pConnInfo);
        if (ret)
        {
            int errno;
            char *err_msg = strerror(errno);

            msg("Can't Create thread %s, Error no: %d (%s)\n", "app_cmd_handle_thd", errno, err_msg);
            continue;
        }
        /* 设置子线程为脱离状态 */
        pthread_detach(AppCmdThdID);
    }

end:
    /* 关闭sock_ac，关闭线程 */
    close(iSock_ac);
    pthread_exit("Thread accept_app_thd quit\n");
    return NULL;
}

/*****************************************************************************
 函 数 名  : app_init
 功能描述  : App初始化实现函数
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
void app_init(stDock *pstDock)
{
    int ret;
    pthread_t AppPreviewThdID;
    pthread_t AcceptAppThdID;

    /* 创建App预览线程 app_preview_thd */
    dbg("(Recv MCU Thread) Create thread app_preview_thd\n\n");
    ret = pthread_create(&AppPreviewThdID, NULL, app_preview_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "app_preview_thd", errno, err_msg);
        return;
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(AppPreviewThdID);

    /* 创建接受App连接线程 accept_app_thd */
    dbg("(Recv MCU Thread) Create thread accept_mcu_thd\n\n");
    ret = pthread_create(&AcceptAppThdID, NULL, accept_app_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("Can't Create thread %s, Error no: %d (%s)\n", "accept_mcu_thd", errno, err_msg);
        return;
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(AcceptAppThdID);
}

