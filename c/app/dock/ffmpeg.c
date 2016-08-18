/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技

 ******************************************************************************
  文 件 名   : ffmpeg.c
  版 本 号   : 初稿
  作    者   : 蒋小辉
  生成日期   : 2016年4月20日
  最近修改   :
  功能描述   : mmpeg转换相关操作接口
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月20日
    作    者   : 蒋小辉
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ffmpeg.h"
#include "dbg.h"
#include "../ffmpeg/srs_ffmpeg_stream_picture.h"
#include "../ffmpeg/srs_ffmpeg_stream_fwriter.h"


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



//check file exist or filesize is zero
//0: file exist && filesize > zero
//1: file NOT exist || filesize == 0
static int chk_exist(char *pfile)
{
	int exist = 0;
	struct stat statbuf;

	//exist: 1 file exist
	//       0 file not exist
	exist = !(access(pfile, F_OK));
	stat(pfile, &statbuf);

	if (exist && statbuf.st_size > 0)
	{
	    return 0;
	}
	else
	{
	    if (exist && statbuf.st_size == 0)
	    {
	    	msg("%s filesize is zero\n", pfile);
		}
	    return 1;
	}
}



/*****************************************************************************
 函 数 名  : FFmpegH264ToFlv
 功能描述  : 传入h264数据，合成成流媒体格式的文件:媒体frame可以按照自定义尺寸。
 输入参数  : char *h264FrameDataBuf   H264帧buff
 			 unsigned int nDatalen
             char *mediapath  转换后的媒体文件.MP4
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月20日
    作    者   : 蒋小辉
    修改内容   : 新生成函数

*****************************************************************************/
int FFmpegH264ToMP4(u8 *h264FrameDataBuf, u32 nDatalen, char *mediapath)
{
	int iskeyframe = 0;  //是否是关键帧
	unsigned int srs_width  = 1280;
	unsigned int srs_height = 1080;
	unsigned int srcdatalen = srs_width * srs_height * 3 / 2;

	unsigned int   srs_mux_width  = srs_width;
	unsigned int   srs_mux_height = srs_height;
	unsigned int   srs_mux_fps    = 25;
	unsigned int   srs_mux_keyframeinterval = 5;
	unsigned int   srs_mux_bitrate = 6000000;
	unsigned int   srs_mux_samplerate = 44100;
	unsigned int   srs_mux_samplebit = 16;
	unsigned int   srs_mux_soundchanel = 2;
	char *srs_mux_file_type = "mp4";  //转换后的文件的后缀 可以是mp4 and flv
	char *srs_mux_file_name = mediapath;

	ffmpeg_fwriter_obj   *ff_fw_obj = ff_fwriter_obj_create();
	ff_fwriter_mux_width(ff_fw_obj, srs_mux_width);
	ff_fwriter_mux_height(ff_fw_obj, srs_mux_height);
	ff_fwriter_mux_fps(ff_fw_obj, srs_mux_fps);
	ff_fwriter_mux_keyframeinterval(ff_fw_obj, srs_mux_keyframeinterval);
	ff_fwriter_mux_bitrate(ff_fw_obj, srs_mux_bitrate);
	ff_fwriter_mux_samplerate(ff_fw_obj, srs_mux_samplerate);
	ff_fwriter_mux_samplebit(ff_fw_obj, srs_mux_samplebit);
	ff_fwriter_mux_soundchanel(ff_fw_obj, srs_mux_soundchanel);
	ff_fwriter_mux_file_type(ff_fw_obj, srs_mux_file_type);
	ff_fwriter_mux_file_name(ff_fw_obj, srs_mux_file_name);
	ff_fw_obj = ff_fwriter_mux_create(ff_fw_obj);

	unsigned char *pdata = (unsigned char *)malloc(srcdatalen);
	do
	{
		if (NULL == pdata)
		{
			break;
		}

		if (NULL == ff_fw_obj)
		{
			break;
		}

		//把h264帧数据写入流媒体容器中
		ff_fwriter_mux_write_video(ff_fw_obj, h264FrameDataBuf, nDatalen, 0, iskeyframe);

	} while (0);

	ff_fwriter_mux_finish(ff_fw_obj);
	ff_fwriter_obj_destory(ff_fw_obj);

	return 0;
}



/*****************************************************************************
 函 数 名  : video_opera_handle
 功能描述  : 原始H264文件转换为.FLV文件操作线程
*****************************************************************************/
void* h264toflv_handle_thd(stMPath *pPath)
{
	char Cmdbuf[MAX_SHEEL_CMD_LEN] = {0};
	char inputfile[MAX_FILE_PATH_NAME_LEN] = {0};
	char outputfile[MAX_FILE_PATH_NAME_LEN] = {0};

	if (NULL == pPath)
	{
		msg("(%s, %d)param error !\n", __FUNCTION__, __LINE__);
		return;
	}

	/*等待1s,等待原始素材文件.264写入完成*/
	sleep(1);

	/*检测输入文件是否存在*/
	if (1 == chk_exist(pPath->input))
	{
		msg("input file NOT exist");
		return;
	}

	strcpy(inputfile, pPath->input);
	strcpy(outputfile, pPath->output);

	//注意此shell脚本的存放路径，后续讨论再决定
	sprintf(Cmdbuf, "./h264Toflv.sh %s %s", inputfile, outputfile);
	system(Cmdbuf);  //阻塞接口

	return;
}



/*****************************************************************************
 函 数 名  : convert_yuv_to_photo
 功能描述  : yuv帧数据转换为照片文件(jpg 或者缩略图)文件
*****************************************************************************/
void* convert_yuv_to_photo(unsigned char *yuv, const int width_in, const int height_in, const int width_out, const int height_out, const char *fimagename)
{
	convert_jpg_from_yuv_frame_data(yuv, width_in, height_in, width_out, height_out, fimagename);
}

