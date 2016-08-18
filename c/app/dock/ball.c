/******************************************************************************

                  版权所有 (C), 2016-2026, 深圳进化动力数码科技有限公司

 ******************************************************************************
  文 件 名   : ball.c
  版 本 号   : 初稿
  作    者   : 彭斌全
  生成日期   : 2016年4月14日
  最近修改   :
  功能描述   : 球机相关函数实现
  函数列表   :
  修改历史   :
  1.日    期   : 2016年4月14日
    作    者   : 彭斌全
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/socket.h>
#include "include/main.h"
#include "include/dbg.h"
#include "include/ball.h"
#include "include/ffmpeg.h"

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
////存储decode的buffer
struct ball_frame_buffer gFrameBuffer;

////保存4幅拍照图片
char* gPictureBuffer;

////拍照时间戳
int g_pictrue_time = 0;

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/
static int m_sockBallCtrl;
static int m_sockBallStream;
static sem_t m_semBallCtrl;
static struct ball_packet m_stBallCmdPkt;

bool m_writing_file_flag = false;		/*是否在保存录像文件*/
bool m_video_flag = false;				/*是否正在录像*/
bool m_preview_flag = false;			/*是否正在预览*/

//录像文件路径
char szVideoPath[4][100];
char szVideoFileName[4][64] = {"master_dvp.264", "master_mipi.264", "slave_dvp.264", "slave_mipi.264"};

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/
 #define SAVE_FILE 				////是否存预览文件,调试用
 #define APP_STREAM 				////直接给app测试
 #define TAKE_PHOTO_FILE			////拍照操作,是否存文件,调试用

/*****************************************************************************
 函 数 名  : mcu_type_to_str
 功能描述  : 球机报文类型转换字符串说明实现函数
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
char *ball_type_to_str(u8 ucType)
{
    static char *acStr[] = {"keep alive", "date operate", "change mode",
    "start preview", "stop preview", "start video", "stop video", "read video time",
    "take photo", "set exp comp", "set wt bal mode", "read date", "read video time", "take photo"};

    return acStr[ucType];
}

/*
函数名:connect_server
功	能:模块函数,dock连接球机服务器
参  数:pSocket:通信套接字,psz_ip:球机IP地址, n_port:端口号
作  者:lulei
日  期:2016,04,19
*/
static int connect_server(int* p_socket, int n_port)
{
	*p_socket;
	int Socket;
	struct sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(n_port);
	Addr.sin_addr.s_addr = inet_addr(SOCK_BC_IP_ADDR);

    /* 创建socket */
	Socket = socket(AF_INET, SOCK_STREAM, 0);

	while(1)
	{
		if (connect(Socket, (struct sockaddr *) &Addr, sizeof(struct sockaddr) ) < 0)
		{
			printf("error:connect server failed!\n");
			sleep(1);
		}
		else
		{
			//printf("info:connect server successful!\n");
			*p_socket = Socket;
			return 1;
		}
	}

	return 0;
}

/*****************************************************************************
 函 数 名  : send_ball_cmd_pkt
 功能描述  : 发送球机命令报文实现函数
 输入参数  : u32 uiCmd
             u32 *pData
 输出参数  : u32 *pData
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月19日
    作    者   : 卢磊
    修改内容   : 新生成函数

  2.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 改成线程方式处理
*****************************************************************************/
int send_ball_cmd_pkt( u32 ulCmd, u32 ulData)
{
    /* 初始化MCU命令报文 */
    memset(&m_stBallCmdPkt, 0, sizeof(m_stBallCmdPkt));

    /* 填充命令报文内容 */
	strcpy(m_stBallCmdPkt.magic, "Bcmd");
    m_stBallCmdPkt.type = ulCmd;
    m_stBallCmdPkt.data = ulData;

    /* 唤醒等待的球机控制线程 ball_ctrl_thd，发送球机命令 */
    if (g_init_debug_switch)
        dbg("\t\t\tWake up Ball Ctrl Thread\n");
    /* 唤醒等待的发送MCU线程，发送MCU命令 */

	////球机操作命令的特殊处理
	/*如果是停止预览操作,设置标志位*/
	if(ulCmd == BALL_CMD_STOP_PREVIEW)
	{
		if(false == m_preview_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview\n");
			return 0;
		}
		m_preview_flag = false;
	}

	/*如果是开始录像操作则设置的标志位*/
	else if(ulCmd == BALL_CMD_START_VIDEO)
	{
		/*前提是预览*/
		if( false == m_preview_flag )
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview, can not video\n");
			return 0;
		}

		m_video_flag = true;
		m_writing_file_flag = false;
	}

	/*如果是停止录像操作则清理标志位*/
	else if(ulCmd == BALL_CMD_STOP_VIDEO)
	{
		if(false == m_video_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start video, can not stop\n");
			return 0;
		}

		m_video_flag = false;
	}

	/*如果是读取录像时间操作,则要先保证是在录像状态*/
	else if(ulCmd == BALL_CMD_READ_VIDEO_TIME)
	{
		/*如果不是录像状态,则不发送这个命令*/
		if(m_video_flag == false)
		{
			msg("(send_ball_cmd_pkt) error, you do not start vide\n");
			return 0;
		}
	}

	/*拍照命令是在预览的前提下*/
	else if(ulCmd == BALL_CMD_TAKE_PHOTO)
	{
		if(false == m_preview_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview, can not take photo\n");
			return 0;
		}
	}

	/*设置曝光是在预览的前提下*/
	else if(ulCmd == BALL_CMD_SET_EXP_COMP)
	{
		if(false == m_preview_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview, can not set exp comp\n");
			return 0;
		}
	}

	else
	{}

	//唤醒控制线程发送球机控制命令
    sem_post(&m_semBallCtrl);
}

/*****************************************************************************
 函 数 名  : ball_data_thd
 功能描述  : 球机数据线程实现函数
 输入参数  : Dock板结构体类型,pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月14日
    作    者   : 彭斌全
    修改内容   : 新生成函数
	2.日	期	 : 2016年4月19日
	  作	者	 : 卢磊
	  修改内容	 : 函数功能的实现

*****************************************************************************/
void *ball_data_thd(void *arg)
{
    stDock *pstDock = (stDock *)arg;

    dbg("(Ball data Thread) Enter ball_data_thd\n\n");

    ////数据buffer,存放接收到的4路数据,用于DECODE
	char* pszDataBuffer[4] = {NULL, NULL, NULL, NULL};

	struct frame_info net_enc;

	//保存前两帧数据
	struct frame_info net_enc_first;
	struct frame_info net_enc_second;
	//标志位,是否是第一或第二帧数据
	bool bIsFirstFrame = false;
	bool bIsSecondFrame = false;
	//前两帧帧数据
	char szFirstFrameData[1024] = "";
	char szSecondFrameData[1024] = "";

	int BUFFER_SIZE = 2700000;
	char buf[2700000];
	int file_flag = 1;
	int ret = 0;

	int nIsFirstIFrame = 0;	////标志位,是否是第一个关键帧

	int sendbytes;

	////缩略图
	int nWidthIn   = 1280;			//yuv的宽
	int nHeightIn  = 1080;			//yuv的高
	int nWidthOut  = 1280;			//转换后的宽
	int nHeightOut = 1080;			//转换后的高
	char PictureName[40] = "";
	int nIsCreatePicture = 0; 		//是否创建缩略图

	int i;
	struct dock dock_statu;
	strcpy(dock_statu.magic_key, "EvoS");

	//保存预览流文件
	FILE *pFileView[4];
	//保存录像文件
	FILE *pFileVideo[4];
	//标志位,是否进行了录像
	bool bIsHaveVideoFile = false;

	dbg("(data thread)::::Enter ball_data_thd\n");

	//初始化文件指针
	for(i = 0; i < 4; i++)
	{
		pFileView[i] = NULL;
		pFileVideo[i] = NULL;
	}

#ifdef SAVE_FILE
	pFileView[0] = fopen("View1.h264", "wb+");
	pFileView[1] = fopen("View2.h264", "wb+");
	pFileView[2] = fopen("View3.h264", "wb+");
	pFileView[3] = fopen("View4.h264", "wb+");
#endif

	////为buffer分配空间
	for(i = 0; i < 4; i++)
	{
		pszDataBuffer[i] = (char*)malloc(sizeof(char) * 1280*1080*3/2);
		if(pszDataBuffer[i] == NULL)
		{
			msg("(data thread) malloc buffer fail %d", i+1);
			goto COME_HERE;
		}
		memset(pszDataBuffer[i], 0, 1280*1080*3/2);

		////初始化gFrameBuffer
		gFrameBuffer.pszFrameBuffer[i] = pszDataBuffer[i];
		gFrameBuffer.nLen[i] = 0;
	}

	////连接球机服务器数据端口
	if(0 == connect_server(&m_sockBallStream, SOCK_BD_PORT) )
	{
		////connect error
		msg("::::ball_data_thd connect server error\n");
		close(m_sockBallStream);
		goto COME_HERE;
	}

	msg("data_thd connect server successful!\n");

	sendbytes = 0;
	if(send(m_sockBallStream, &dock_statu, sizeof(dock_statu), 0) <= 0)
	{
		dbg("(data thread) send error\n");
		close(m_sockBallStream);
		goto COME_HERE;
	}

	dbg("(data thread) send successful, wait stream from ball\n");

	bIsFirstFrame = true;
	bIsSecondFrame = false;
	while(1)
    {
    	sendbytes = 0;
		int nRecvOnce = 0;
        while(sendbytes < sizeof(net_enc))
        {
        	nRecvOnce = recv(m_sockBallStream, &buf[sendbytes],sizeof(net_enc) - sendbytes,0);
			if(nRecvOnce <= 0)
			{
				//一般是球机收到停止预览命令,然后主动断开连接
				msg("(data thread) recv frame head error\n");
				//设置预览标志位
				m_preview_flag = false;

				goto COME_HERE;
			}

			sendbytes += nRecvOnce;
        }

		//当有数据从球机到来,说明预览成功,设置标志位
		if(false == m_preview_flag)
		{
			m_preview_flag = true;
		}

		//接收头信息
		memcpy(&net_enc, buf, sizeof(net_enc) );
        int enc_len = net_enc.len[0] + net_enc.len[1] + net_enc.len[2] + net_enc.len[3];
        msg("Frame ID: %d, i_frame: %d\n", net_enc.seq_no, net_enc.i_frame);

		//保存前两帧头信息
		if(bIsFirstFrame == true)
		{
			net_enc_first.i_frame = net_enc.i_frame;
			for(i = 0; i < 4; i++)
			{
				net_enc_first.len[i] = net_enc.len[i];
			}
			net_enc_first.timestamp = net_enc_first.timestamp;
		}

		if(bIsSecondFrame == true)
		{
			net_enc_second.i_frame = net_enc.i_frame;
			for(i = 0; i < 4; i++)
			{
				net_enc_second.len[i] = net_enc.len[i];
			}
			net_enc_second.timestamp = net_enc_first.timestamp;
		}

		//是否在录像,录像则存储录像文件
		if(true == m_video_flag)
		{
			//是否已经开始写文件
			if(false == m_writing_file_flag)
			{
				//是否是关键帧
				if(0 == net_enc.i_frame)
				{
					//获取当前系统时间,创建路径
					time_t Time;
					struct tm *pTime;
					char szTemp[64];
					time(&Time);
					pTime = localtime(&Time);
					msg("current time:%d\n", Time);
					sprintf(szTemp, "%d-%d-%d-%d-%d-%d/", pTime->tm_year + 1900, pTime->tm_mon+1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
					msg("dir: %s\n", szTemp);
					if( mkdir(szTemp, S_IRWXU, S_IRWXG, S_IRWXO) < 0)
					{
						msg("****make dir fail\n");
						goto COME_HERE;
					}
					//创建录像文件
					for(i = 0; i < 4; i++)
					{
						memset(szVideoPath[i], 0, 100);
						sprintf(szVideoPath[i], "%s%s",szTemp, szVideoFileName[i]);
						pFileVideo[i] = fopen(szVideoPath[i], "wb+");
						if(pFileVideo[i] == NULL)
						{
							msg("(data thread) create video file fail\n");
							m_video_flag = false;
							goto COME_HERE;
						}
					}
					bIsHaveVideoFile = true;

					if(m_video_flag == true)
					{
						msg("open video file successful\n");
						//将保存的前两帧数据写入文件
						int nDataLen1 = 0;
						int nDataLen2 = 0;
						for(i = 0; i< 4; i++)
						{
							nDataLen1 = fwrite(szFirstFrameData + nDataLen1, net_enc_first.len[i], 1,pFileVideo[i]);
							nDataLen1 += net_enc_first.len[i];
							nDataLen2 = fwrite(szSecondFrameData + nDataLen2, net_enc_second.len[i], 1,pFileVideo[i]);
							nDataLen2 += net_enc_second.len[i];
						}

						//设置标志位
						nIsCreatePicture = 1;			//开始创建缩略图
						m_writing_file_flag = true;	//开始写录像文件
					}
				}
			}
		}

        msg("head:%d,need recv:%d\n",sendbytes,enc_len);

		//数据部分
        sendbytes = 0;
        while(sendbytes < enc_len)
        {
            int recv_len = recv(m_sockBallStream,&buf[sendbytes],(enc_len - sendbytes),0);
            if(recv_len <= 0)
            {
                perror("recv frame ");
                msg("recv enc err,exit\n");
				msg("recv size;%d video1:%d,video2:%d,video3:%d,video4:%d\n",sendbytes,net_enc.len[0],net_enc.len[1],net_enc.len[2],net_enc.len[3]);
                goto COME_HERE;
            }
            sendbytes += recv_len;
        }
        msg("recv size;%d video1:%d,video2:%d,video3:%d,video4:%d\n",sendbytes,net_enc.len[0],net_enc.len[1],net_enc.len[2],net_enc.len[3]);

		//保存前两帧数据
		if(bIsFirstFrame == true)
		{
			memcpy(szFirstFrameData, buf, enc_len);
			bIsFirstFrame = false;
			bIsSecondFrame = true;

			msg("save first data successful\n");
		}
		else
		{
			if(true == bIsSecondFrame)
			{
				msg("begin save second data\n");
				memcpy(szSecondFrameData, buf, enc_len);
				bIsSecondFrame = false;

				msg("save second data successful\n");
			}
		}

		//长度赋值
		for(i = 0; i < 4; i++)
		{
			gFrameBuffer.nLen[i] = net_enc.len[i];
		}

		if(file_flag == 1)
        {
            if(net_enc.len[0] != 0)
            {
	            //ret = fwrite(buf,net_enc.len[0],1,pFileView[0]);
	            ////将数据拷贝到解码用的buffer
#ifdef APP_STREAM
                if(false == pstDock->app_send_done_flag)
#endif
	            memcpy(pszDataBuffer[0], buf, net_enc.len[0]);
				msg("(data thread) memcpy successful data1\n");

				////唤醒解码线程,把gFrameBuffer.pszFrameBuffer[]中的数据取走
				//sem_post(&(pstDock->sem_recv_decode[0]) );
#ifdef APP_STREAM
				if(true == pstDock->app_send_done_flag)
				{
					msg("(data thread) enter app_stream\n");
				    pstDock->app_send_done_flag = false;
					pstDock->pAppPreviewBuffer = pszDataBuffer[0];
					pstDock->stAppFrameHead.uiID = net_enc.seq_no;
					pstDock->stAppFrameHead.uiLen = net_enc.len[0];
					if (0 == net_enc.i_frame)
					{
					    pstDock->stAppFrameHead.uiInterval = 0;
    				}
    				else
    				{
    				    pstDock->stAppFrameHead.uiInterval = 5;
    				}

    				/* 唤醒 发送预览数据线程 app_preview_thd  */
                    dbg("(Ball data Thread) Wake up app_preview_thd\n\n");
                    sem_post(&pstDock->sem_app_preview);
				}
#endif
				//写录像文件
				if(true == m_writing_file_flag)
				{
					////生成缩略图(转换图片借口没完成?)
					msg("(data thread) write video file1\n");
					if(nIsCreatePicture == 1)
					{
						strcpy(PictureName, "master_dvp.tmb");
						//convert_yuv_to_photo(buf, nWidthIn, nHeightIn, nWidthOut, nHeightOut, PictureName);
						nIsCreatePicture = 0;
					}
					////生成录像文件
					ret = fwrite(buf,net_enc.len[0],1,pFileVideo[0]);
		            if( 1 != ret )
		            {
		                msg("line 1 ret videofile:%d\n",ret);
		                perror("fwrite");
		                m_writing_file_flag = false;
						m_video_flag = false;
						goto COME_HERE;
		            }
				}

#ifdef SAVE_FILE
				////写文件
				ret = fwrite(buf,net_enc.len[0],1,pFileView[0]);
	            if( 1 != ret )
	            {
	                msg("line 1 ret:%d\n",ret);
	                perror("fwrite");
	                return NULL;
	            }
#endif
            }

            if(net_enc.len[1] != 0)
            {
	            //ret = fwrite(&buf[net_enc.len[0]],net_enc.len[1],1,pFileView[1]);
	            memcpy(pszDataBuffer[1], &buf[net_enc.len[0]], net_enc.len[1]);

				////唤醒解码线程,把gFrameBuffer.pszFrameBuffer[nDataNum]中的数据取走
				//sem_post(&(pstDock->sem_recv_decode[1]) );

				//写录像文件
				if(true == m_writing_file_flag)
				{
					ret = fwrite(&buf[net_enc.len[0]],net_enc.len[1],1,pFileVideo[1]);
		            if( 1 != ret )
		            {
		                msg("line 2 ret videofile:%d\n",ret);
		                perror("fwrite");
		                m_writing_file_flag = false;
						m_video_flag = false;
						goto COME_HERE;
		            }
				}

#ifdef SAVE_FILE
				ret = fwrite(&buf[net_enc.len[0]], net_enc.len[1],1,pFileView[1]);
	            if( 1 != ret )
	            {
	                msg("line 2 ret:%d\n",ret);
	                perror("fwrite");
	                return NULL;
	            }
#endif
            }

            if(net_enc.len[2] != 0)
            {
	            //ret = fwrite(&buf[net_enc.len[0]+net_enc.len[1]],net_enc.len[2],1,pFileView[2]);
				memcpy(pszDataBuffer[2], &buf[net_enc.len[0] + net_enc.len[1]], net_enc.len[2]);

				////唤醒解码线程,把gFrameBuffer.pszFrameBuffer[nDataNum]中的数据取走
				//sem_post(&(pstDock->sem_recv_decode[2]) );

				//写录像文件
				if(true == m_writing_file_flag)
				{
					ret = fwrite(&buf[net_enc.len[0]+net_enc.len[1]],net_enc.len[2],1,pFileVideo[2]);
		            if( 1 != ret )
		            {
		                msg("line 3 ret videofile:%d\n",ret);
		                perror("fwrite");
		                m_writing_file_flag = false;
						m_video_flag = false;
						goto COME_HERE;
		            }
				}

#ifdef SAVE_FILE
				ret = fwrite(&buf[net_enc.len[0]+net_enc.len[1]],net_enc.len[2],1,pFileView[2]);
	            if( 1 != ret )
	            {
	                printf("line 3 ret:%d\n",ret);
	                perror("fwrite");
	                return NULL;
	            }
#endif
            }

            if(net_enc.len[3] != 0)
            {
	            //ret = fwrite(&buf[enc_len-net_enc.len[3]],net_enc.len[3],1,pFileView[3]);
	            memcpy(pszDataBuffer[3], &buf[enc_len - net_enc.len[3]], net_enc.len[3]);

				////唤醒解码线程,把gFrameBuffer.pszFrameBuffer[nDataNum]中的数据取走
				//sem_post(&(pstDock->sem_recv_decode[3]) );

				//写录像文件
				if(true == m_writing_file_flag)
				{
					ret = fwrite(&buf[enc_len-net_enc.len[3]],net_enc.len[3],1,pFileVideo[3]);
		            if( 1 != ret )
		            {
		                msg("line 4 ret videofile:%d\n",ret);
		                perror("fwrite");
		                m_writing_file_flag = false;
						m_video_flag = false;
						goto COME_HERE;
		            }
				}

#ifdef SAVE_FILE
				ret = fwrite(&buf[enc_len-net_enc.len[3]],net_enc.len[3],1,pFileView[3]);
	            if( 1 != ret )
	            {
	                printf("line 4 ret:%d\n",ret);
	                perror("fwrite");
	                return NULL;
	            }
#endif
             }

			//判断g_video_flag标志,如果关闭了录像
			if(m_video_flag == false)
			{
				//停止写录像文件
				if(m_writing_file_flag == true)
				{
					m_writing_file_flag = false;
					////关闭文件
					for(i = 0; i < 4; i++)
					{
						if(NULL != pFileVideo[i])
						{
							fclose(pFileVideo[i]);
							pFileVideo[i] = NULL;
						}
					}
					if(bIsHaveVideoFile == true)
					{
						//唤醒球机控制线程,将264文件转化为flv文件
						bIsHaveVideoFile = false;
						send_ball_cmd_pkt(NOTIFY_CTRL_VIDEO_COMPLETE,0);
					}
				}
			}
         }
    }

	////COME_HERE,函数异常退出都来到这里
	COME_HERE:

	////释放空间
	for(i = 0; i < 4; i++)
	{
		if(NULL != pszDataBuffer[i])
		{
			free(pszDataBuffer[i]);
			pszDataBuffer[i] = NULL;
		}
	}

	////关闭文件
	for(i = 0; i < 4; i++)
	{
#ifdef SAVE_FILE
		if(NULL != pFileView[i])
			fclose(pFileView[i]);
#endif

		if(NULL != pFileVideo[i])
		{
			fclose(pFileVideo[i]);
			pFileVideo[i] = NULL;
		}
	}

	////重置标志位
	m_writing_file_flag = false;
	m_video_flag = false;

	//关闭数据连接
	close(m_sockBallStream);

	dbg("(data thread) ::::data_thd:ball_data_thd exit\n");
    return 0;
}

/*****************************************************************************
 函 数 名  : ball_ctrl_thd
 功能描述  : 球机控制线程实现函数
 输入参数  : stDock *pstDock
 输出参数  : 无
 返 回 值  :
 调用函数  :
 被调函数  :

 修改历史      :
  1.日    期   : 2016年4月14日
    作    者   : 彭斌全
    修改内容   : 新生成函数
  2.日    期   : 2016年4月19日
    作    者   : 卢磊
    修改内容   : 函数功能的实现
  3.日    期   : 2016年4月20日
    作    者   : 彭斌全
    修改内容   : 改成睡眠等待方式

*****************************************************************************/
void *ball_ctrl_thd(void *arg)
{
    stDock *pstDock = (stDock *)arg;
	int i = 0;								////给for循环用
	int nSum = 0;							////接收图片的总大小
	fd_set fd_select;						////select集合
	struct timeval select_timeout = {10,0};//select超时5秒

    dbg("(Ball ctrl Thread) Enter ball_ctrl_thd!\n");

    /* 创建socket，连接球机上的sock_bc：6666 */
	if(0 != connect_server(&m_sockBallCtrl, SOCK_BC_PORT) )
	{
		////connect successful
		dbg("(Ball ctrl Thread) Connect server successful!\n");
		while(1)
		{
        	int ret = 0;
            struct ball_packet stBallAckPkt;
			strcpy(stBallAckPkt.magic, MAGIC_CMD);

            if (g_init_debug_switch)
                dbg("(Ball ctrl Thread) Sleep, Waiting Send Ball command\n\n");
			sem_wait(&m_semBallCtrl);

			//截获通知,将264文件转化为flv文件
			if(m_stBallCmdPkt.type == NOTIFY_CTRL_VIDEO_COMPLETE)
			{
				//是否有录像文件需要转化
				int i = 0;
				msg("(Ballctrl Thread) change 264stream to flv file");
				stMPath szPath;
				char szOutName[100] = "";
				for(i = 0; i < 4; i++)
				{
					strcpy(szOutName, szVideoPath[i]);
					szOutName[strlen(szOutName) - 3] = '\0';	//去掉.264
					strcat(szOutName, "flv");					//加上.flv

					strcpy(szPath.output, szOutName);
					strcpy(szPath.input, szVideoPath[i]);
					h264toflv_handle_thd(&szPath);
				}

				continue;
			}

            if (g_init_debug_switch)
                dbg("(Ball ctrl Thread) Send Ball %s command\n", ball_type_to_str(m_stBallCmdPkt.type));
			////错误命令,或退出命令
			////>100的命令用来表示退出或错误
			if(m_stBallCmdPkt.type >= 100)
			{
				////退出
				msg("ball_ctrl_thd exit\n");
				close(m_sockBallCtrl);
				m_sockBallCtrl = 0;
				////关闭数据通道
				////close(m_sockBallStream);
				break;
			}

        	/* 发送操作命令 */
        	ret = send(m_sockBallCtrl, &m_stBallCmdPkt, sizeof(m_stBallCmdPkt), 0);
        	if(ret <= 0)
        	{
        		msg("send %s command error\n", ball_type_to_str(m_stBallCmdPkt.type));
				continue;
        	}
        	else if (g_init_debug_switch)
        	{
        		dbg("(Ball ctrl Thread) send %s command successful, data: %d\n\n", ball_type_to_str(m_stBallCmdPkt.type),
        		m_stBallCmdPkt.data);
        	}

            /* 是否需要等待回应 */
            if( !(m_stBallCmdPkt.type == BALL_CMD_SET_EXP_COMP || m_stBallCmdPkt.type == BALL_CMD_DATE_OPERATE || m_stBallCmdPkt.type == BALL_CMD_READ_VIDEO_TIME || m_stBallCmdPkt.type == BALL_CMD_TAKE_PHOTO) )
            {
				continue;
			}

            /* 接收操作回应 */
			msg("(Ball ctrl Thread) wait Ack packet\n");
            memset(&stBallAckPkt, 0, sizeof(stBallAckPkt));
			ret = recv(m_sockBallCtrl, &stBallAckPkt, sizeof(stBallAckPkt), 0);
			if(ret <= 0)
			{
				msg("BALL_CMD_DATE_OPERATE error\n");
				return 0;
			}
            dbg("(Ball ctrl Thread) Recv Ball %s Ack packet\n", ball_type_to_str(stBallAckPkt.type));

			/*设置曝光,新加的*/
			if(BALL_CMD_SET_EXP_COMP == stBallAckPkt.type)
			{
				msg("(Ball ctrl Thread)  Set Exp Comp Complete\n");
			}

            /* 拍照回应处理 */
            else if (BALL_ACK_TAKE_PHOTO == stBallAckPkt.type)
            {
            	//判断stBallAckPkt.data是否==0, ==0说明不符合拍照条件,不接受拍照数据
            	if(0 == stBallAckPkt.data)
            	{
            		msg("(Ball ctrl Thread) take picture fail, prease preview firsh\n");
					continue;
				}
#ifdef TAKE_PHOTO_FILE
				////保存文件看看
				FILE *picture_file[4];
				int nPictureSize = 1280*1080*1.5;
				picture_file[0] = fopen("picture_file1", "wb+");
				picture_file[1] = fopen("picture_file2", "wb+");
				picture_file[2] = fopen("picture_file3", "wb+");
				picture_file[3] = fopen("picture_file4", "wb+");

				if(NULL == picture_file)
				{
					msg("*****open file error\n");
				}
#endif
				////保存拍照时间戳
				g_pictrue_time = stBallAckPkt.data;
				msg("(Ball ctrl Thread) TakePhotoTime is %d\n", g_pictrue_time);

				////保存四幅图片到buffer
				gPictureBuffer = (char*)malloc(PICTURE_SIZE);
				memset(gPictureBuffer, 0, PICTURE_SIZE);

				////接收4幅图片数据
				ret = 0;
				nSum = 0;
				while(nSum < PICTURE_SIZE)
				{
					FD_ZERO(&fd_select);
					FD_SET(m_sockBallCtrl, &fd_select);
					ret = select(m_sockBallCtrl + 1, &fd_select, 0, 0, &select_timeout);

					if(ret < 0)			////select出错
					{
						dbg("(Ball ctrl Thread) ball_data_thd select error\n");
						break;
					}
					else if(ret == 0)	////超时
					{
						dbg("(Ball ctrl Thread) ball_data_thd recv picture failed\n");
						continue;
					}
					else				////有数据到来
					{
						if( FD_ISSET(m_sockBallCtrl, &fd_select) )
						{
							ret = 0;
							ret = recv(m_sockBallCtrl, gPictureBuffer + nSum, PICTURE_SIZE - nSum, 0);
							////已经接收到的图片总数据
							nSum += ret;
						}
					}

				}
				msg("********recv take photo data size = %d\n", nSum);

				//唤醒拼接线程从gPictureBuffer中取走数据
				sem_post(&pstDock->sem_recv_photo_data);

#ifdef TAKE_PHOTO_FILE
				if(NULL != picture_file[0])
				{
					fwrite(gPictureBuffer, nPictureSize, 1, picture_file[0]);
					fwrite(gPictureBuffer + nPictureSize, nPictureSize, 1, picture_file[1]);
					fwrite(gPictureBuffer + 2*nPictureSize, nPictureSize, 1, picture_file[2]);
					fwrite(gPictureBuffer + 3*nPictureSize, nPictureSize, 1, picture_file[3]);
	#if 0
					//将图片帧buffer生成图片看看
					int nWidthIn = 1280;
					int nHeightIn = 1080;
					int nWidthOut = 1280;
					int nHeigthOut = 1080;
					char szPictureName[] = "PhotoName";
					convert_yuv_to_photo(gPictureBuffer, nWidthIn, nHeightIn, nWidthOut, nHeigthOut, szPictureName);
	#endif
					int j = 0;
					for(j = 0; j < 4; j++)
					{
						fclose(picture_file[j]);
					}
				}
#endif
				//什么时候释放gPictureBuffer空间?
            }

			/*读取日期处理:读取球机上的日期*/
			else if(BALL_ACK_READ_DATE == stBallAckPkt.type)
			{
				if(stBallAckPkt.data > 0)
				{
					//读取球机时间,并设置dock时间
					char cmd_hw_clk[20] = "hwclock --systohc";
					struct tm *pTime;
					pstDock->bBallDateSyncFlag = 1;
					stime((time_t *)&stBallAckPkt.data);
					pTime = localtime((time_t *)&stBallAckPkt.data);
					system(cmd_hw_clk);
					msg("(Ball ctrl Thread) set time to : %s\n", asctime(pTime));
				}
			}
			/*读取录像时间处理:从录像开始到当前时间的秒数*/
			else if(BALL_ACK_READ_VIDEO_TIME == stBallAckPkt.type)
			{
				/* 将这个时间存储在一个全局变量中 */
				pstDock->uiRecordTime = stBallAckPkt.data;

				/* 唤醒线程 app_cmd_handle_thd ，回复录像时间给App */
				sem_post(&pstDock->semRecordTime);

				/* 打印录制时间 */
				dbg("(Ball ctrl Thread) Video was recorded %u s\n", stBallAckPkt.data);
			}
			else
			{
				msg("error ball_ack type: %d\n", stBallAckPkt.type);
				break;
			}
		}
	}
	else
	{
		////connect error
		msg("ball_ctrl_thd connect server error\n");
	}

	dbg("(Ball ctrl Thread) ball_ctrl_thd exit\n");
    return 0;
}

/*****************************************************************************
 函 数 名  : ball_init
 功能描述  : 球机初始化实现函数
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
void ball_init(stDock *pstDock)
{
    int ret;
    pthread_t BallCtrlThdID;

    /* 初始化球机模块内部使用的信号量 */
    sem_init(&m_semBallCtrl, 0, 0);

    /* 创建球机控制线程 */
    dbg("(main Thread) Create thread ball_ctrl_thd\n\n");
    ret = pthread_create(&BallCtrlThdID, NULL, ball_ctrl_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(main Thread) Can't Create thread %s, Error no: %d (%s)\n", "ball_ctrl_thd", errno, err_msg);
        exit(3);
    }
    /* 设置子线程为脱离状态 */
    pthread_detach(BallCtrlThdID);

    /* 等待球机控制线程处理 */
    sleep(1);

    dbg("(main Thread) Send Date Operate command to Ball\n");
    /* 发送日期操作命令(查询日期) */
    send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, 0);
}

