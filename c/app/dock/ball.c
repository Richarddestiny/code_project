/******************************************************************************

                  ��Ȩ���� (C), 2016-2026, ���ڽ�����������Ƽ����޹�˾

 ******************************************************************************
  �� �� ��   : ball.c
  �� �� ��   : ����
  ��    ��   : ���ȫ
  ��������   : 2016��4��14��
  ����޸�   :
  ��������   : �����غ���ʵ��
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��4��14��
    ��    ��   : ���ȫ
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
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
////�洢decode��buffer
struct ball_frame_buffer gFrameBuffer;

////����4������ͼƬ
char* gPictureBuffer;

////����ʱ���
int g_pictrue_time = 0;

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/
static int m_sockBallCtrl;
static int m_sockBallStream;
static sem_t m_semBallCtrl;
static struct ball_packet m_stBallCmdPkt;

bool m_writing_file_flag = false;		/*�Ƿ��ڱ���¼���ļ�*/
bool m_video_flag = false;				/*�Ƿ�����¼��*/
bool m_preview_flag = false;			/*�Ƿ�����Ԥ��*/

//¼���ļ�·��
char szVideoPath[4][100];
char szVideoFileName[4][64] = {"master_dvp.264", "master_mipi.264", "slave_dvp.264", "slave_mipi.264"};

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/
 #define SAVE_FILE 				////�Ƿ��Ԥ���ļ�,������
 #define APP_STREAM 				////ֱ�Ӹ�app����
 #define TAKE_PHOTO_FILE			////���ղ���,�Ƿ���ļ�,������

/*****************************************************************************
 �� �� ��  : mcu_type_to_str
 ��������  : �����������ת���ַ���˵��ʵ�ֺ���
 �������  : u8 ucType
 �������  : ��
 �� �� ֵ  : char
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��21��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
char *ball_type_to_str(u8 ucType)
{
    static char *acStr[] = {"keep alive", "date operate", "change mode",
    "start preview", "stop preview", "start video", "stop video", "read video time",
    "take photo", "set exp comp", "set wt bal mode", "read date", "read video time", "take photo"};

    return acStr[ucType];
}

/*
������:connect_server
��	��:ģ�麯��,dock�������������
��  ��:pSocket:ͨ���׽���,psz_ip:���IP��ַ, n_port:�˿ں�
��  ��:lulei
��  ��:2016,04,19
*/
static int connect_server(int* p_socket, int n_port)
{
	*p_socket;
	int Socket;
	struct sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(n_port);
	Addr.sin_addr.s_addr = inet_addr(SOCK_BC_IP_ADDR);

    /* ����socket */
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
 �� �� ��  : send_ball_cmd_pkt
 ��������  : ������������ʵ�ֺ���
 �������  : u32 uiCmd
             u32 *pData
 �������  : u32 *pData
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��19��
    ��    ��   : ¬��
    �޸�����   : �����ɺ���

  2.��    ��   : 2016��4��20��
    ��    ��   : ���ȫ
    �޸�����   : �ĳ��̷߳�ʽ����
*****************************************************************************/
int send_ball_cmd_pkt( u32 ulCmd, u32 ulData)
{
    /* ��ʼ��MCU����� */
    memset(&m_stBallCmdPkt, 0, sizeof(m_stBallCmdPkt));

    /* ������������ */
	strcpy(m_stBallCmdPkt.magic, "Bcmd");
    m_stBallCmdPkt.type = ulCmd;
    m_stBallCmdPkt.data = ulData;

    /* ���ѵȴ�����������߳� ball_ctrl_thd������������� */
    if (g_init_debug_switch)
        dbg("\t\t\tWake up Ball Ctrl Thread\n");
    /* ���ѵȴ��ķ���MCU�̣߳�����MCU���� */

	////���������������⴦��
	/*�����ֹͣԤ������,���ñ�־λ*/
	if(ulCmd == BALL_CMD_STOP_PREVIEW)
	{
		if(false == m_preview_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview\n");
			return 0;
		}
		m_preview_flag = false;
	}

	/*����ǿ�ʼ¼����������õı�־λ*/
	else if(ulCmd == BALL_CMD_START_VIDEO)
	{
		/*ǰ����Ԥ��*/
		if( false == m_preview_flag )
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview, can not video\n");
			return 0;
		}

		m_video_flag = true;
		m_writing_file_flag = false;
	}

	/*�����ֹͣ¼������������־λ*/
	else if(ulCmd == BALL_CMD_STOP_VIDEO)
	{
		if(false == m_video_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start video, can not stop\n");
			return 0;
		}

		m_video_flag = false;
	}

	/*����Ƕ�ȡ¼��ʱ�����,��Ҫ�ȱ�֤����¼��״̬*/
	else if(ulCmd == BALL_CMD_READ_VIDEO_TIME)
	{
		/*�������¼��״̬,�򲻷����������*/
		if(m_video_flag == false)
		{
			msg("(send_ball_cmd_pkt) error, you do not start vide\n");
			return 0;
		}
	}

	/*������������Ԥ����ǰ����*/
	else if(ulCmd == BALL_CMD_TAKE_PHOTO)
	{
		if(false == m_preview_flag)
		{
			msg("(send_ball_cmd_pkt) error, you do not start preview, can not take photo\n");
			return 0;
		}
	}

	/*�����ع�����Ԥ����ǰ����*/
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

	//���ѿ����̷߳��������������
    sem_post(&m_semBallCtrl);
}

/*****************************************************************************
 �� �� ��  : ball_data_thd
 ��������  : ��������߳�ʵ�ֺ���
 �������  : Dock��ṹ������,pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��14��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���
	2.��	��	 : 2016��4��19��
	  ��	��	 : ¬��
	  �޸�����	 : �������ܵ�ʵ��

*****************************************************************************/
void *ball_data_thd(void *arg)
{
    stDock *pstDock = (stDock *)arg;

    dbg("(Ball data Thread) Enter ball_data_thd\n\n");

    ////����buffer,��Ž��յ���4·����,����DECODE
	char* pszDataBuffer[4] = {NULL, NULL, NULL, NULL};

	struct frame_info net_enc;

	//����ǰ��֡����
	struct frame_info net_enc_first;
	struct frame_info net_enc_second;
	//��־λ,�Ƿ��ǵ�һ��ڶ�֡����
	bool bIsFirstFrame = false;
	bool bIsSecondFrame = false;
	//ǰ��֡֡����
	char szFirstFrameData[1024] = "";
	char szSecondFrameData[1024] = "";

	int BUFFER_SIZE = 2700000;
	char buf[2700000];
	int file_flag = 1;
	int ret = 0;

	int nIsFirstIFrame = 0;	////��־λ,�Ƿ��ǵ�һ���ؼ�֡

	int sendbytes;

	////����ͼ
	int nWidthIn   = 1280;			//yuv�Ŀ�
	int nHeightIn  = 1080;			//yuv�ĸ�
	int nWidthOut  = 1280;			//ת����Ŀ�
	int nHeightOut = 1080;			//ת����ĸ�
	char PictureName[40] = "";
	int nIsCreatePicture = 0; 		//�Ƿ񴴽�����ͼ

	int i;
	struct dock dock_statu;
	strcpy(dock_statu.magic_key, "EvoS");

	//����Ԥ�����ļ�
	FILE *pFileView[4];
	//����¼���ļ�
	FILE *pFileVideo[4];
	//��־λ,�Ƿ������¼��
	bool bIsHaveVideoFile = false;

	dbg("(data thread)::::Enter ball_data_thd\n");

	//��ʼ���ļ�ָ��
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

	////Ϊbuffer����ռ�
	for(i = 0; i < 4; i++)
	{
		pszDataBuffer[i] = (char*)malloc(sizeof(char) * 1280*1080*3/2);
		if(pszDataBuffer[i] == NULL)
		{
			msg("(data thread) malloc buffer fail %d", i+1);
			goto COME_HERE;
		}
		memset(pszDataBuffer[i], 0, 1280*1080*3/2);

		////��ʼ��gFrameBuffer
		gFrameBuffer.pszFrameBuffer[i] = pszDataBuffer[i];
		gFrameBuffer.nLen[i] = 0;
	}

	////����������������ݶ˿�
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
				//һ��������յ�ֹͣԤ������,Ȼ�������Ͽ�����
				msg("(data thread) recv frame head error\n");
				//����Ԥ����־λ
				m_preview_flag = false;

				goto COME_HERE;
			}

			sendbytes += nRecvOnce;
        }

		//�������ݴ��������,˵��Ԥ���ɹ�,���ñ�־λ
		if(false == m_preview_flag)
		{
			m_preview_flag = true;
		}

		//����ͷ��Ϣ
		memcpy(&net_enc, buf, sizeof(net_enc) );
        int enc_len = net_enc.len[0] + net_enc.len[1] + net_enc.len[2] + net_enc.len[3];
        msg("Frame ID: %d, i_frame: %d\n", net_enc.seq_no, net_enc.i_frame);

		//����ǰ��֡ͷ��Ϣ
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

		//�Ƿ���¼��,¼����洢¼���ļ�
		if(true == m_video_flag)
		{
			//�Ƿ��Ѿ���ʼд�ļ�
			if(false == m_writing_file_flag)
			{
				//�Ƿ��ǹؼ�֡
				if(0 == net_enc.i_frame)
				{
					//��ȡ��ǰϵͳʱ��,����·��
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
					//����¼���ļ�
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
						//�������ǰ��֡����д���ļ�
						int nDataLen1 = 0;
						int nDataLen2 = 0;
						for(i = 0; i< 4; i++)
						{
							nDataLen1 = fwrite(szFirstFrameData + nDataLen1, net_enc_first.len[i], 1,pFileVideo[i]);
							nDataLen1 += net_enc_first.len[i];
							nDataLen2 = fwrite(szSecondFrameData + nDataLen2, net_enc_second.len[i], 1,pFileVideo[i]);
							nDataLen2 += net_enc_second.len[i];
						}

						//���ñ�־λ
						nIsCreatePicture = 1;			//��ʼ��������ͼ
						m_writing_file_flag = true;	//��ʼд¼���ļ�
					}
				}
			}
		}

        msg("head:%d,need recv:%d\n",sendbytes,enc_len);

		//���ݲ���
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

		//����ǰ��֡����
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

		//���ȸ�ֵ
		for(i = 0; i < 4; i++)
		{
			gFrameBuffer.nLen[i] = net_enc.len[i];
		}

		if(file_flag == 1)
        {
            if(net_enc.len[0] != 0)
            {
	            //ret = fwrite(buf,net_enc.len[0],1,pFileView[0]);
	            ////�����ݿ����������õ�buffer
#ifdef APP_STREAM
                if(false == pstDock->app_send_done_flag)
#endif
	            memcpy(pszDataBuffer[0], buf, net_enc.len[0]);
				msg("(data thread) memcpy successful data1\n");

				////���ѽ����߳�,��gFrameBuffer.pszFrameBuffer[]�е�����ȡ��
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

    				/* ���� ����Ԥ�������߳� app_preview_thd  */
                    dbg("(Ball data Thread) Wake up app_preview_thd\n\n");
                    sem_post(&pstDock->sem_app_preview);
				}
#endif
				//д¼���ļ�
				if(true == m_writing_file_flag)
				{
					////��������ͼ(ת��ͼƬ���û���?)
					msg("(data thread) write video file1\n");
					if(nIsCreatePicture == 1)
					{
						strcpy(PictureName, "master_dvp.tmb");
						//convert_yuv_to_photo(buf, nWidthIn, nHeightIn, nWidthOut, nHeightOut, PictureName);
						nIsCreatePicture = 0;
					}
					////����¼���ļ�
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
				////д�ļ�
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

				////���ѽ����߳�,��gFrameBuffer.pszFrameBuffer[nDataNum]�е�����ȡ��
				//sem_post(&(pstDock->sem_recv_decode[1]) );

				//д¼���ļ�
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

				////���ѽ����߳�,��gFrameBuffer.pszFrameBuffer[nDataNum]�е�����ȡ��
				//sem_post(&(pstDock->sem_recv_decode[2]) );

				//д¼���ļ�
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

				////���ѽ����߳�,��gFrameBuffer.pszFrameBuffer[nDataNum]�е�����ȡ��
				//sem_post(&(pstDock->sem_recv_decode[3]) );

				//д¼���ļ�
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

			//�ж�g_video_flag��־,����ر���¼��
			if(m_video_flag == false)
			{
				//ֹͣд¼���ļ�
				if(m_writing_file_flag == true)
				{
					m_writing_file_flag = false;
					////�ر��ļ�
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
						//������������߳�,��264�ļ�ת��Ϊflv�ļ�
						bIsHaveVideoFile = false;
						send_ball_cmd_pkt(NOTIFY_CTRL_VIDEO_COMPLETE,0);
					}
				}
			}
         }
    }

	////COME_HERE,�����쳣�˳�����������
	COME_HERE:

	////�ͷſռ�
	for(i = 0; i < 4; i++)
	{
		if(NULL != pszDataBuffer[i])
		{
			free(pszDataBuffer[i]);
			pszDataBuffer[i] = NULL;
		}
	}

	////�ر��ļ�
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

	////���ñ�־λ
	m_writing_file_flag = false;
	m_video_flag = false;

	//�ر���������
	close(m_sockBallStream);

	dbg("(data thread) ::::data_thd:ball_data_thd exit\n");
    return 0;
}

/*****************************************************************************
 �� �� ��  : ball_ctrl_thd
 ��������  : ��������߳�ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��14��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���
  2.��    ��   : 2016��4��19��
    ��    ��   : ¬��
    �޸�����   : �������ܵ�ʵ��
  3.��    ��   : 2016��4��20��
    ��    ��   : ���ȫ
    �޸�����   : �ĳ�˯�ߵȴ���ʽ

*****************************************************************************/
void *ball_ctrl_thd(void *arg)
{
    stDock *pstDock = (stDock *)arg;
	int i = 0;								////��forѭ����
	int nSum = 0;							////����ͼƬ���ܴ�С
	fd_set fd_select;						////select����
	struct timeval select_timeout = {10,0};//select��ʱ5��

    dbg("(Ball ctrl Thread) Enter ball_ctrl_thd!\n");

    /* ����socket����������ϵ�sock_bc��6666 */
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

			//�ػ�֪ͨ,��264�ļ�ת��Ϊflv�ļ�
			if(m_stBallCmdPkt.type == NOTIFY_CTRL_VIDEO_COMPLETE)
			{
				//�Ƿ���¼���ļ���Ҫת��
				int i = 0;
				msg("(Ballctrl Thread) change 264stream to flv file");
				stMPath szPath;
				char szOutName[100] = "";
				for(i = 0; i < 4; i++)
				{
					strcpy(szOutName, szVideoPath[i]);
					szOutName[strlen(szOutName) - 3] = '\0';	//ȥ��.264
					strcat(szOutName, "flv");					//����.flv

					strcpy(szPath.output, szOutName);
					strcpy(szPath.input, szVideoPath[i]);
					h264toflv_handle_thd(&szPath);
				}

				continue;
			}

            if (g_init_debug_switch)
                dbg("(Ball ctrl Thread) Send Ball %s command\n", ball_type_to_str(m_stBallCmdPkt.type));
			////��������,���˳�����
			////>100������������ʾ�˳������
			if(m_stBallCmdPkt.type >= 100)
			{
				////�˳�
				msg("ball_ctrl_thd exit\n");
				close(m_sockBallCtrl);
				m_sockBallCtrl = 0;
				////�ر�����ͨ��
				////close(m_sockBallStream);
				break;
			}

        	/* ���Ͳ������� */
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

            /* �Ƿ���Ҫ�ȴ���Ӧ */
            if( !(m_stBallCmdPkt.type == BALL_CMD_SET_EXP_COMP || m_stBallCmdPkt.type == BALL_CMD_DATE_OPERATE || m_stBallCmdPkt.type == BALL_CMD_READ_VIDEO_TIME || m_stBallCmdPkt.type == BALL_CMD_TAKE_PHOTO) )
            {
				continue;
			}

            /* ���ղ�����Ӧ */
			msg("(Ball ctrl Thread) wait Ack packet\n");
            memset(&stBallAckPkt, 0, sizeof(stBallAckPkt));
			ret = recv(m_sockBallCtrl, &stBallAckPkt, sizeof(stBallAckPkt), 0);
			if(ret <= 0)
			{
				msg("BALL_CMD_DATE_OPERATE error\n");
				return 0;
			}
            dbg("(Ball ctrl Thread) Recv Ball %s Ack packet\n", ball_type_to_str(stBallAckPkt.type));

			/*�����ع�,�¼ӵ�*/
			if(BALL_CMD_SET_EXP_COMP == stBallAckPkt.type)
			{
				msg("(Ball ctrl Thread)  Set Exp Comp Complete\n");
			}

            /* ���ջ�Ӧ���� */
            else if (BALL_ACK_TAKE_PHOTO == stBallAckPkt.type)
            {
            	//�ж�stBallAckPkt.data�Ƿ�==0, ==0˵����������������,��������������
            	if(0 == stBallAckPkt.data)
            	{
            		msg("(Ball ctrl Thread) take picture fail, prease preview firsh\n");
					continue;
				}
#ifdef TAKE_PHOTO_FILE
				////�����ļ�����
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
				////��������ʱ���
				g_pictrue_time = stBallAckPkt.data;
				msg("(Ball ctrl Thread) TakePhotoTime is %d\n", g_pictrue_time);

				////�����ķ�ͼƬ��buffer
				gPictureBuffer = (char*)malloc(PICTURE_SIZE);
				memset(gPictureBuffer, 0, PICTURE_SIZE);

				////����4��ͼƬ����
				ret = 0;
				nSum = 0;
				while(nSum < PICTURE_SIZE)
				{
					FD_ZERO(&fd_select);
					FD_SET(m_sockBallCtrl, &fd_select);
					ret = select(m_sockBallCtrl + 1, &fd_select, 0, 0, &select_timeout);

					if(ret < 0)			////select����
					{
						dbg("(Ball ctrl Thread) ball_data_thd select error\n");
						break;
					}
					else if(ret == 0)	////��ʱ
					{
						dbg("(Ball ctrl Thread) ball_data_thd recv picture failed\n");
						continue;
					}
					else				////�����ݵ���
					{
						if( FD_ISSET(m_sockBallCtrl, &fd_select) )
						{
							ret = 0;
							ret = recv(m_sockBallCtrl, gPictureBuffer + nSum, PICTURE_SIZE - nSum, 0);
							////�Ѿ����յ���ͼƬ������
							nSum += ret;
						}
					}

				}
				msg("********recv take photo data size = %d\n", nSum);

				//����ƴ���̴߳�gPictureBuffer��ȡ������
				sem_post(&pstDock->sem_recv_photo_data);

#ifdef TAKE_PHOTO_FILE
				if(NULL != picture_file[0])
				{
					fwrite(gPictureBuffer, nPictureSize, 1, picture_file[0]);
					fwrite(gPictureBuffer + nPictureSize, nPictureSize, 1, picture_file[1]);
					fwrite(gPictureBuffer + 2*nPictureSize, nPictureSize, 1, picture_file[2]);
					fwrite(gPictureBuffer + 3*nPictureSize, nPictureSize, 1, picture_file[3]);
	#if 0
					//��ͼƬ֡buffer����ͼƬ����
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
				//ʲôʱ���ͷ�gPictureBuffer�ռ�?
            }

			/*��ȡ���ڴ���:��ȡ����ϵ�����*/
			else if(BALL_ACK_READ_DATE == stBallAckPkt.type)
			{
				if(stBallAckPkt.data > 0)
				{
					//��ȡ���ʱ��,������dockʱ��
					char cmd_hw_clk[20] = "hwclock --systohc";
					struct tm *pTime;
					pstDock->bBallDateSyncFlag = 1;
					stime((time_t *)&stBallAckPkt.data);
					pTime = localtime((time_t *)&stBallAckPkt.data);
					system(cmd_hw_clk);
					msg("(Ball ctrl Thread) set time to : %s\n", asctime(pTime));
				}
			}
			/*��ȡ¼��ʱ�䴦��:��¼��ʼ����ǰʱ�������*/
			else if(BALL_ACK_READ_VIDEO_TIME == stBallAckPkt.type)
			{
				/* �����ʱ��洢��һ��ȫ�ֱ����� */
				pstDock->uiRecordTime = stBallAckPkt.data;

				/* �����߳� app_cmd_handle_thd ���ظ�¼��ʱ���App */
				sem_post(&pstDock->semRecordTime);

				/* ��ӡ¼��ʱ�� */
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
 �� �� ��  : ball_init
 ��������  : �����ʼ��ʵ�ֺ���
 �������  : stDock *pstDock
 �������  : ��
 �� �� ֵ  :
 ���ú���  :
 ��������  :

 �޸���ʷ      :
  1.��    ��   : 2016��4��20��
    ��    ��   : ���ȫ
    �޸�����   : �����ɺ���

*****************************************************************************/
void ball_init(stDock *pstDock)
{
    int ret;
    pthread_t BallCtrlThdID;

    /* ��ʼ�����ģ���ڲ�ʹ�õ��ź��� */
    sem_init(&m_semBallCtrl, 0, 0);

    /* ������������߳� */
    dbg("(main Thread) Create thread ball_ctrl_thd\n\n");
    ret = pthread_create(&BallCtrlThdID, NULL, ball_ctrl_thd, (void *)pstDock);
    if (ret)
    {
        int errno;
        char *err_msg = strerror(errno);

        msg("(main Thread) Can't Create thread %s, Error no: %d (%s)\n", "ball_ctrl_thd", errno, err_msg);
        exit(3);
    }
    /* �������߳�Ϊ����״̬ */
    pthread_detach(BallCtrlThdID);

    /* �ȴ���������̴߳��� */
    sleep(1);

    dbg("(main Thread) Send Date Operate command to Ball\n");
    /* �������ڲ�������(��ѯ����) */
    send_ball_cmd_pkt(BALL_CMD_DATE_OPERATE, 0);
}

