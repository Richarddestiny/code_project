/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ�

 ******************************************************************************
  �� �� ��   : photo.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��20��
  ����޸�   :
  ��������   :
  �����б�   :
              photo_opera_handle
  �޸���ʷ   :
  1.��    ��   : 2016��4��20��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "ball.h"

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/
extern char* gPictureBuffer; /*����ɼ�����������*/
extern int g_pictrue_time;   /*����ʱ���*/

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

/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : photo.c
  �� �� ��   : ����
  ��    ��   : ��С��
  ��������   : 2016��4��19��
  ����޸�   :
  ��������   : ���ղ�����غ���ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��19��
    ��    ��   : ��С��
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "stitch.h"
#include "photo.h"
#include "ffmpeg.h"

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
#define  YUV_PICTURE_SIZE   (1280*1080*3/2) /*ԭʼ��Ƭ�ļ���size*/


/*----------------------------------------------*
 * �ṹ�嶨��                                       *
 *----------------------------------------------*/
/* ϵͳʱ��ṹ*/
typedef struct tag_ST_TIME
{
    u8     ucCentury;          /* ���ͣ���19��99 */
    u8     ucYear;             /* ��ݣ���0��99 */
    u8     ucMonth;            /* �£���0��11 */
    u8     ucDay;              /* �գ���1��31����С�¡��������� */
    u8     ucHour;             /* Сʱ����0��23 */
    u8     ucMinute;           /* �֣���0��59 */
    u8     ucSecond;           /* �룬��0��59 */
    u8     ucCentiSecond;      /* �ٷ��룬��0��99 */
}ST_TIME;



/*****************************************************************************
 ��������    : ConvertSecToTime();
 ����        : ����ʱ��ת��Ϊ�ṹ��ʱ��.
               ��ʱ����ָ��1900��1��1��0��0��0�뿪ʼ�������.
 �������    : ulSecond,    �����.
 �������    : pstTime,     ָ������Ľṹ��ʱ���ָ��, ����ɹ�, ���ؽṹ����ʱ��.
 ����ֵ      : SUCESS,  �ɹ�;
               ����,    ʧ��.
 ��������˵��:
 ����ʹ��ʾ��:
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
 �� �� ��  : photo_opera_handle
 ��������  : ���ղ�������ӿ�ʵ�ֺ���
 �������  : stDock * pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ��С��
    �޸�����   : �����ɺ���

*****************************************************************************/
int photo_opera_handle(stDock * pstDock)
{
	int nRet;

	u32 width_in   = 0;//yuv�Ŀ�
	u32 height_in  = 0;//yuv�ĸ�
	u32 width_out  = 0;//ת����Ŀ�
	u32 height_out = 0;//ת����ĸ�

	char *imgpath  = "/recent/picture/";  //FTP���·��
	char fimagename[PATH_NAME_LEN] = {0};
	char cFileName[PATH_NAME_LEN] = {0};

	ST_TIME stTime;
	u8 *pStitchOutBuf = NULL; /* ƴ�Ӻ���������ݴ洢buffer */
	ImageParam IP;  /*ͼ���������������ͼ�����Ŀ����ȡ��߶ȡ��ߴ��Լ����ͼ��Ŀ�ȡ��߶ȡ��ߴ硣*/

	if (NULL == pstDock)
	{
		msg("para error \n");
	    return -1;
	}

	/*ƴ�ӳ�ʼ�� STITCH_REFINED:��ϸƴ��*/
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

	/*������������߳� ,������������*/
    send_ball_cmd_pkt(BALL_CMD_TAKE_PHOTO, 0);

    /* �ȴ����յ���������,����������߳� ball_ctrl_thd ���� */
    sem_wait(&pstDock->sem_recv_photo_data);

	/*ƴ�Ӵ���*/
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

	/*ת��ʱ���*/
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

	//FFmpegת������1 (������Ƭ�ļ� .jpg (1280*1080))
	width_in   = 1280;//yuv�Ŀ�
	height_in  = 1080;//yuv�ĸ�
	width_out  = 1440;//ת����Ŀ�
	height_out = 720;//ת����ĸ�
	memset(fimagename, 0, sizeof(fimagename));
	memset(cFileName, 0, sizeof(cFileName));
	sprintf(cFileName, "%d-%d-%d-%d-%d-%d", (stTime.ucCentury*100 + stTime.ucYear), stTime.ucMonth, stTime.ucDay,
		stTime.ucHour, stTime.ucMinute, stTime.ucSecond);
	strcat(fimagename, imgpath);
	strcat(fimagename, cFileName);
	strcat(fimagename, ".jpg");
	convert_yuv_to_photo(pStitchOutBuf, width_in, height_in, width_out, height_out, fimagename);

	//FFmpegת������2 (��������ͼ .tmb (192*108))
	width_in   = 1280;//yuv�Ŀ�
	height_in  = 1080;//yuv�ĸ�
	width_out  = 192;//ת����Ŀ�
	height_out = 108;//ת����ĸ�
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

