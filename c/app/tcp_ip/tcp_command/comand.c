/*************************************************************************
> File Name: main.c
> Author: richard 
> Mail: freedom_wings@foxmail.com
> Created Time: Sun 11 Oct 2015 06:57:32 PM PDT
************************************************************************/

#include<stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "client.h"

#define BUFSIZE 1000
#define MAGIC_KEY "EvoC"
#define IDLE_STATE_TIME "00:00:00"
#define IDLE_STATE_PER "00%"
#define SOCKET_IP "192.168.0.163"
#define SOCKET_PORT 6666
#define DOCK_BUF 1024
struct dock_client {
            char magic_key[8];
            int cmd_type;
            int data;

};
struct ball_packet
{
    char magic[8];//识别key：命令为：“Bcmd”；回应为：“Back”
    unsigned int type;    //命令或者回应类型
    unsigned int  data;    //命令或者回应数据
};
sem_t sem_stream;
int main(int argc,char*argv[])
{
    struct hostent *host;
    int socket_fd;
    struct sockaddr_in remote_addr;
    struct dock_client dock_buf;
    int count = 0,len;
    const int image_size = 1280*1080*3/2;
    struct ball_packet back_packet;
    char imag_buf[image_size];
    int i;
    int temp_size;
    FILE *fd;
    char name_buf[100];
	pthread_t dataline_tid;
	int mode,operate = 0; 
	char pthoto_dir[] = "pthoto";
	char video_dir[] = "video";
    int ret;
	memset (&remote_addr, 0, sizeof (remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(SOCKET_PORT);
    if(argc < 4)
    {
        fprintf(stderr,"USAGE: ./client Hostname(or ip address) Text file_flag\n");
        exit(1);
    }

    if((host = gethostbyname(argv[1])) == NULL)
     {
         perror("gethostbyname");
         exit(-1);
     } 
    remote_addr.sin_addr = *((struct in_addr *)host->h_addr);

	 if(access(pthoto_dir,NULL) != 0)
    {
        ret = mkdir(pthoto_dir,S_IRWXU | S_IRWXG | S_IRWXO);
        if(ret < 0)
        {
            printf("mkdir failed:%s\n",strerror(errno));
            return -1;
        }
    }
    else
        printf("waring %s dir existed\n",pthoto_dir);
	
	 if(access(video_dir,NULL) != 0)
    {
        ret = mkdir(video_dir,S_IRWXU | S_IRWXG | S_IRWXO);
        if(ret < 0)
        {
            printf("mkdir failed:%s\n",strerror(errno));
            return -1;
        }
    }
    else
        printf("waring %s dir existed\n",video_dir);
	
	
    if ((socket_fd = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return 1;
    }
    while(1)
    {
        if (connect (socket_fd, (struct sockaddr *) &remote_addr, sizeof (struct sockaddr)) < 0)
        {
            perror ("connect");
            return 1;
        }
        else
        break;
    }
	 sem_init(&sem_stream,0,0);
	printf("select manual or auto mode ( 0、1 ):");
	scanf("%d",&mode);
	
	while(1)
	{	
		pthread_create(&dataline_tid,NULL,(void*)data_pthread,(void*)argv);
		sem_wait(&sem_stream);
		while (1)
		{
			count++;
			memset(&dock_buf,0,sizeof(dock_buf));
			memcpy(&dock_buf.magic_key,MAGIC_KEY,5);
			if(mode == 0)
			{	
				printf("pls input cmd type:");
				scanf("%d",&dock_buf.cmd_type);
				if(dock_buf.cmd_type == 9 )
				{
					printf("pls input exp valule:");
					scanf("%d",&dock_buf.data);
				}
			}
			else
			{
				if(count != 1)
				sleep(20);
				switch(operate)
				{
					case 0:
					printf("set medium start\n");
					dock_buf.cmd_type = 3;
					operate = 1;
					break;
					case 1:
					printf("get pthoto\n");
					dock_buf.cmd_type = 8;
					operate = 2;
					break;
					case 2:
					printf("set camera to %d\n",count%13);
					dock_buf.cmd_type = 9;
					dock_buf.data = count %13;
					operate = 3;
					break;
					case 3:
					printf("set medium stop\n");
					dock_buf.cmd_type = 4;
					operate = 0;
					break;
					default : 
					printf("unknow cmd\n");
					break;
				}
			}
	
			// scanf("%d %hd %hd",&dock_buf.cmd_type,&dock_buf.buf[0],&dock_buf.buf[1]);
			printf("send:%d\n",dock_buf.cmd_type);
			send(socket_fd,&dock_buf,sizeof(dock_buf),0);
			if(dock_buf.cmd_type == 1 || dock_buf.cmd_type == 9)
			len = recv (socket_fd, &dock_buf, sizeof(dock_buf), 0);
			else if(dock_buf.cmd_type == 8)
			{
				len = recv(socket_fd,&back_packet,sizeof(back_packet),0);
				printf("recv pthoto head %d\n",len);
				for(i = 0;i < 4;i++)
				{
					len = 0;
					while(image_size != len)
					{
						len += recv(socket_fd,&imag_buf[len],image_size - len,0);
					}
						printf("recv image:%d, len:%d\n",i,len);
						sprintf(name_buf,"pthoto/image_%d_%d.yuv",i,count);
						fd = fopen(name_buf,"wb");
						if(fd == NULL)
						{
							perror("file open failed\n");
							exit(-1);

						}
						fwrite(imag_buf,image_size,1,fd);
						fclose(fd);

				}
			}
			else if(dock_buf.cmd_type == 4)
			{
				pthread_cancel(dataline_tid);
				pthread_join(dataline_tid, NULL);
				printf("data line pthread leave\n");
				break;
			}
			
		 }

   }
}
