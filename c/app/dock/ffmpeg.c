/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ�

 ******************************************************************************
  �� �� ��   : ffmpeg.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��20��
  ����޸�   :
  ��������   : mmpegת����ز����ӿ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��20��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
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
 �� �� ��  : FFmpegH264ToFlv
 ��������  : ����h264���ݣ��ϳɳ���ý���ʽ���ļ�:ý��frame���԰����Զ���ߴ硣
 �������  : char *h264FrameDataBuf   H264֡buff
 			 unsigned int nDatalen
             char *mediapath  ת�����ý���ļ�.MP4
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��20��
    ��    ��   : ��С��
    �޸�����   : �����ɺ���

*****************************************************************************/
int FFmpegH264ToMP4(u8 *h264FrameDataBuf, u32 nDatalen, char *mediapath)
{
	int iskeyframe = 0;  //�Ƿ��ǹؼ�֡
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
	char *srs_mux_file_type = "mp4";  //ת������ļ��ĺ�׺ ������mp4 and flv
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

		//��h264֡����д����ý��������
		ff_fwriter_mux_write_video(ff_fw_obj, h264FrameDataBuf, nDatalen, 0, iskeyframe);

	} while (0);

	ff_fwriter_mux_finish(ff_fw_obj);
	ff_fwriter_obj_destory(ff_fw_obj);

	return 0;
}



/*****************************************************************************
 �� �� ��  : video_opera_handle
 ��������  : ԭʼH264�ļ�ת��Ϊ.FLV�ļ������߳�
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

	/*�ȴ�1s,�ȴ�ԭʼ�ز��ļ�.264д�����*/
	sleep(1);

	/*��������ļ��Ƿ����*/
	if (1 == chk_exist(pPath->input))
	{
		msg("input file NOT exist");
		return;
	}

	strcpy(inputfile, pPath->input);
	strcpy(outputfile, pPath->output);

	//ע���shell�ű��Ĵ��·�������������پ���
	sprintf(Cmdbuf, "./h264Toflv.sh %s %s", inputfile, outputfile);
	system(Cmdbuf);  //�����ӿ�

	return;
}



/*****************************************************************************
 �� �� ��  : convert_yuv_to_photo
 ��������  : yuv֡����ת��Ϊ��Ƭ�ļ�(jpg ��������ͼ)�ļ�
*****************************************************************************/
void* convert_yuv_to_photo(unsigned char *yuv, const int width_in, const int height_in, const int width_out, const int height_out, const char *fimagename)
{
	convert_jpg_from_yuv_frame_data(yuv, width_in, height_in, width_out, height_out, fimagename);
}

