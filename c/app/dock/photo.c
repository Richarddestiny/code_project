/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技

 ******************************************************************************
  文 件 名   : photo.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月20日
  最近修改   :
  功能描述   :
  函数列表   :
              photo_opera_handle
  修改历史   :
  1.日    期   : 2016年4月20日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "ball.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/
extern char* gPictureBuffer; /*球机采集的拍照数据*/
extern int g_pictrue_time;   /*拍照时间戳*/

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

/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : photo.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月19日
  最近修改   :
  功能描述   : 拍照操作相关函数实现
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月19日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "stitch.h"
#include "photo.h"
#include "ffmpeg.h"

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
#define  YUV_PICTURE_SIZE   (1280*1080*3/2) /*原始照片文件的size*/


/*----------------------------------------------*
 * 结构体定义                                       *
 *----------------------------------------------*/
/* 系统时间结构*/
typedef struct tag_ST_TIME
{
    u8     ucCentury;          /* 世纪，从19到99 */
    u8     ucYear;             /* 年份，从0到99 */
    u8     ucMonth;            /* 月，从0到11 */
    u8     ucDay;              /* 日，从1到31，大小月、闰月另算 */
    u8     ucHour;             /* 小时，从0到23 */
    u8     ucMinute;           /* 分，从0到59 */
    u8     ucSecond;           /* 秒，从0到59 */
    u8     ucCentiSecond;      /* 百分秒，从0到99 */
}ST_TIME;



/*****************************************************************************
 函数名称    : ConvertSecToTime();
 功能        : 将秒时间转化为结构化时间.
               秒时间是指从1900年1月1日0点0分0秒开始的秒计数.
 输入参数    : ulSecond,    秒计数.
 输出参数    : pstTime,     指向输出的结构化时间的指针, 如果成功, 返回结构化的时间.
 返回值      : SUCESS,  成功;
               其他,    失败.
 函数调用说明:
 典型使用示例:
*****************************************************************************/
u32 ConvertSecToTime(u32 ulSecond, ST_TIME *pstTime)
{
    struct tm*  ptmspec;
    time_t      sec;
    int         year;
    u32        ulSec;

    sec = (time_t)ulSecond;

    ptmspec = localtime(&sec);
    if( NULL == ptmspec )
    {
        return -1;
    }

    year = ptmspec->tm_year;

    pstTime->ucCentury      = (u8)((year/100) + 19 );
    pstTime->ucYear         = (u8)(year%100);
    pstTime->ucMonth        = (u8)(ptmspec->tm_mon);
    pstTime->ucDay          = (u8)(ptmspec->tm_mday);
    pstTime->ucHour         = (u8)(ptmspec->tm_hour);
    pstTime->ucMinute       = (u8)(ptmspec->tm_min);
    pstTime->ucSecond       = (u8)(ptmspec->tm_sec);
    pstTime->ucCentiSecond  =  0;

    ulSec   = (u32)(ptmspec->tm_hour) * 60 * 60;
    ulSec  += (u32)(ptmspec->tm_min) * 60;
    ulSec  += (u32)(ptmspec->tm_sec);

    return 0;
}


/*****************************************************************************
 函 数 名  : photo_opera_handle
 功能描述  : 拍照操作处理接口实现函数
 输入参数  : stDock * pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月19日
    作    者   : 蒋小辉
    修改内容   : 新生成函数

*****************************************************************************/
int photo_opera_handle(stDock * pstDock)
{
	int nRet;

	u32 width_in   = 0;//yuv的宽
	u32 height_in  = 0;//yuv的高
	u32 width_out  = 0;//转换后的宽
	u32 height_out = 0;//转换后的高

	char *imgpath  = "/recent/picture/";  //FTP存放路径
	char fimagename[PATH_NAME_LEN] = {0};
	char cFileName[PATH_NAME_LEN] = {0};

	ST_TIME stTime;
	u8 *pStitchOutBuf = NULL; /* 拼接后输出的数据存储buffer */
	ImageParam IP;  /*图像参数，包括输入图像的数目、宽度、高度、尺寸以及输出图像的宽度、高度、尺寸。*/

	if (NULL == pstDock)
	{
		msg("para error \n");
	    return -1;
	}

	/*拼接初始化 STITCH_REFINED:精细拼接*/
	memset(&IP, 0, sizeof(ImageParam));
	if (!StitchInit("/usr/data/param_medium.evo", &IP, 5, 5, 0, 0, STITCH_REFINED))
	{
		msg("StitchInit error!!\n");
		return -1;

	}
	msg("nCam:%d, nCamWidth:%d, nCamHeight:%d, nFinalWidth:%d, nFinalHeight:%d\n",
		IP.nCam, IP.nCamWidth, IP.nCamHeight, IP.nFinalWidth, IP.nFinalHeight);

	if (IP.nCamWidth != 1280 || IP.nCamHeight != 1080)
	{
		msg("photo len is not matching !\n");
		return -1;
	}

	pStitchOutBuf = (u8 *)malloc(IP.nFinalSize * 3 / 2);
	if (NULL == pStitchOutBuf)
	{
		msg("malloc fail !\n");
		return -1;
	}

	/*唤醒球机控制线程 ,发送拍照命令*/
    send_ball_cmd_pkt(BALL_CMD_TAKE_PHOTO, 0);

    /* 等待接收到拍照数据,由球机控制线程 ball_ctrl_thd 唤醒 */
    sem_wait(&pstDock->sem_recv_photo_data);

	/*拼接处理*/
	nRet = StitchProcess(gPictureBuffer,
						 gPictureBuffer + YUV_PICTURE_SIZE,
						 gPictureBuffer + 2*YUV_PICTURE_SIZE,
						 gPictureBuffer + 3*YUV_PICTURE_SIZE,
						 pStitchOutBuf);
	if (0 != nRet)
	{
		msg("Sititch Process fail \n");
		return -1;
	}

	/*转换时间戳*/
	memset(&stTime, 0, sizeof(ST_TIME));
	nRet = ConvertSecToTime(g_pictrue_time, &stTime);
	if (0 != nRet)
	{
		msg("ConvertSecToTime Fail !\n");
		return -1;
	}
	msg("Photo Time: %d-%d-%d-%d-%d-%d\n",
		stTime.ucYear, stTime.ucMonth, stTime.ucDay,
		stTime.ucHour, stTime.ucMinute, stTime.ucSecond);

	//FFmpeg转换处理1 (生成照片文件 .jpg (1280*1080))
	width_in   = 1280;//yuv的宽
	height_in  = 1080;//yuv的高
	width_out  = 1440;//转换后的宽
	height_out = 720;//转换后的高
	memset(fimagename, 0, sizeof(fimagename));
	memset(cFileName, 0, sizeof(cFileName));
	sprintf(cFileName, "%d-%d-%d-%d-%d-%d", (stTime.ucCentury*100 + stTime.ucYear), stTime.ucMonth, stTime.ucDay,
		stTime.ucHour, stTime.ucMinute, stTime.ucSecond);
	strcat(fimagename, imgpath);
	strcat(fimagename, cFileName);
	strcat(fimagename, ".jpg");
	convert_yuv_to_photo(pStitchOutBuf, width_in, height_in, width_out, height_out, fimagename);

	//FFmpeg转换处理2 (生成缩略图 .tmb (192*108))
	width_in   = 1280;//yuv的宽
	height_in  = 1080;//yuv的高
	width_out  = 192;//转换后的宽
	height_out = 108;//转换后的高
	memset(fimagename, 0, sizeof(fimagename));
	memset(cFileName, 0, sizeof(cFileName));
	sprintf(cFileName, "%d-%d-%d-%d-%d-%d", (stTime.ucCentury*100 + stTime.ucYear), stTime.ucMonth, stTime.ucDay,
		stTime.ucHour, stTime.ucMinute, stTime.ucSecond);
	strcat(fimagename, imgpath);
	strcat(fimagename, cFileName);
	strcat(fimagename, ".tmb");
	convert_yuv_to_photo(pStitchOutBuf, width_in, height_in, width_out, height_out, fimagename);

	if (NULL != pStitchOutBuf)
	{
		free(pStitchOutBuf);
	}

	StitchUninit();

    return 0;
}

