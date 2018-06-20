/*************************************************************************
  > File Name: main.c
  > Author: Richard
  > Mail: freedom_wings@foxmail.com 
  > Created Time: 2014年05月14日 星期三 09时33分57秒
 ************************************************************************/

#include "camera.h"

int get_ack(camera_msg *cmd)
{
	return cmd->head + cmd->ctr_code + strlen(cmd->data);

}


int calibrate_date(camera_msg *cmd)
{
	char date[50];
	sprintf(date,"date -s \"%s\";hwclock -w",cmd->data);
#if defined(DEBUG)
	fprintf(stdout,"date -s \"%s\";hwclock -w\n",cmd->data);
#endif
	system(date);

}

int send_capture(void)
{
	char buff[BUFFSIZE];
	int count;
	FILE *fd = NULL;
	char  filename[FILE_NAME_MAX_SIZE] = {"picture.jpg"};
	camera_msg picture_msg={0x68,HOST_ASK_CAPTURE,0,0,0x16};

	strcpy(picture_msg.data,filename);
	fprintf(stderr,"send picturn:%s size:%d\n",filename,strlen(filename));
	memcpy(buff,&picture_msg,sizeof(camera_msg));
	sleep(1);
	count=send(connfd,buff,BUFFSIZE,0);
	if(count<0)
	{
		perror("Send file information");
		exit(1);
	}

	fd = fopen(filename,"rb");
	if(fd == NULL)
	{
		fprintf(stderr,"open %s failed\n",filename);
		return -1;
	}
	else
	{
		bzero(buff,BUFFSIZE);
		int file_block_length=0;
		while((file_block_length=fread(buff,sizeof(char),BUFFSIZE,fd))>0    )
		{
			printf("file_block_length:%d\n",file_block_length);
			if(send(connfd,buff,file_block_length,0)<0)
			{
				perror("Send");
				exit(1);
			}
			bzero(buff,BUFFSIZE);
		}
		fclose(fd);
		printf("Transfer file finished !\n");
	}


}

int set_wifi_config(FILE *config_file,char wifi_account_password[][SAFE_SAPCE])
{
	char config_linebuf[256];
	char line_name[40];
	char *config_sign = "=";
	char *leave_line;

	fseek(config_file,0,SEEK_END);
	long congig_lenth = ftell(config_file);
	int configbuf_lenth = SAFE_SAPCE;
	configbuf_lenth = configbuf_lenth + 5;
	char sum_buf[congig_lenth+configbuf_lenth+SAFE_SAPCE+5]; //跟文件一样大小的buff
	memset(sum_buf,0,sizeof(sum_buf));
	fseek(config_file,0,SEEK_SET); 
	while(fgets(config_linebuf,256,config_file) != NULL)
	{   
		if(strlen(config_linebuf) < 3) //判断是否是空行
		{
			strcat(sum_buf,config_linebuf);
			continue;
		}
		leave_line = NULL;
		leave_line = strstr(config_linebuf,config_sign);
		if(leave_line == NULL)                            //去除无"="的情况
		{
			strcat(sum_buf,config_linebuf);
			continue;
		}
		int leave_num = leave_line - config_linebuf;
		memset(line_name,0,sizeof(line_name));
		strncpy(line_name,config_linebuf,leave_num);
		if(strcmp(line_name,"AUTH_KEY") ==0)
		{
			strcat(sum_buf,line_name);
			strcat(sum_buf,"=");
			strcat(sum_buf,wifi_account_password[3]);
			strcat(sum_buf,"\n");

		}
		else if(strcmp(line_name,"SSID") ==0)
		{
			strcat(sum_buf,line_name);
			strcat(sum_buf,"=");
			strcat(sum_buf,wifi_account_password[2]);
			strcat(sum_buf,"\n");

		}
		else
		{
			strcat(sum_buf,config_linebuf);
		}
		if(fgetc(config_file)==EOF)
		{
			break;  
		}
		fseek(config_file,-1,SEEK_CUR);
		memset(config_linebuf,0,sizeof(config_linebuf));
	}


	///mnt/nand1-1/etc/network_config
	printf("---sum_buf---->%s<----------\n",sum_buf);
	rename("network_config","network_config~");
	// fclose(config_file);
	FILE *fp;
	//  fp = fopen(conf_path,"w+");
	fp = fopen("network_config","w+");
	if(fp == NULL)
	{
		printf("OPEN CONFIG FALID\n");
		return 2;
	}
	fseek(fp,0,SEEK_SET);
	fputs(sum_buf,fp);
	fclose(fp);

}

int get_wifi_account_password(camera_msg *cmd,char wifi_account_password[][SAFE_SAPCE])
{
	char *buff = cmd->data;
	char **buff_2 = wifi_account_password;
	int i,j,k;

	buff[DATA_SIZE-1] = '\0';
	for(i = 0,j = 0;i < 4;i++)
	{
		while(buff[j] == ' ') //跳过空格
			j++;

		for(k = 0; k < 50 && j < strlen(buff);j++,k++)
		{
			if(buff[j] == ' ')
			{
				buff_2[i][k] = '\0';
				break;
			}
			else
			{
				if(k ==SAFE_SAPCE-1 && buff[j] != ' ')
				{
					fprintf(stderr,"wifi account or password err");
					return -1;
				}
			}
			buff_2[i][k] = buff[j];

		}
	}
}

int check_acconut_password(FILE *f,char account[][SAFE_SAPCE])
{
	char config_linebuf[256];
	char line_name[40];
	char exchange_buf[256];
	char *config_sign = "=";
	char *leave_line;
	char config_buff[SAFE_SAPCE];

	int flag = 0;
	fseek(f,0,SEEK_SET); 
	while(fgets(config_linebuf,256,f) != NULL)
	{   
		if(strlen(config_linebuf) < 3) //判断是否是空行
		{
			continue;
		}
		if (config_linebuf[strlen(config_linebuf)-1] == 10) //去除最后一位是\n的情况
		{

			memset(exchange_buf,0,sizeof(exchange_buf));
			strncpy(exchange_buf,config_linebuf,strlen(config_linebuf)-1);
			memset(config_linebuf,0,sizeof(config_linebuf));
			strcpy(config_linebuf,exchange_buf);
		}
		memset(line_name,0,sizeof(line_name));
		leave_line = strstr(config_linebuf,config_sign);
		if(leave_line == NULL)                            //去除无"="的情况
		{
			continue;
		}
		int leave_num = leave_line - config_linebuf;
		strncpy(line_name,config_linebuf,leave_num);
		if(strcmp(line_name,"AUTH_KEY") ==0)
		{
			strncpy(config_buff,config_linebuf+(leave_num+1),strlen(config_linebuf)-leave_num);
			printf("auth_key:%s  %s\n",config_buff,account[1]);
			if(strcmp(config_buff,account[1]) != 0)

				return -2;
			else
				flag ++;

		}
		else if((strcmp(line_name,"SSID") ==0))
		{
			strncpy(config_buff,config_linebuf+(leave_num+1),strlen(config_linebuf)-leave_num);
			printf("SSID:%s  %s\n",config_buff,account[0]);
			if(strcmp(config_buff,account[0]) != 0)	
				return -2;
			else
				flag ++;
		}
		if(fgetc(f)==EOF)
		{
			break;  
		}
		fseek(f,-1,SEEK_CUR);
		memset(config_linebuf,0,sizeof(config_linebuf));
	}

	return flag == 2?0:-1;
}

int set_wifi(camera_msg *cmd)
{
	FILE *config_file;
	char wifi_account_password[4][SAFE_SAPCE]= {"admin","12345","zhanzheng","98765432"};
	int ret;
	get_wifi_account_password(cmd,wifi_account_password);
	config_file = fopen("network_config","r+");
	if(config_file == NULL)
	{
		fprintf(stderr,"fopen network_config failed:%s\n",strerror(errno));
		return -1;
	}

	ret = check_acconut_password(config_file,wifi_account_password);
	if( ret < 0)
	{
		fprintf(stderr,"account or passwork mach failed:%d\n",ret);
		fclose(config_file);
		return -2;
	}
	set_wifi_config(config_file , wifi_account_password );
	fclose(config_file);
}



int send_version(void)
{
	char version[50] = "version test";
	int count;
	char send_buf[BUFFSIZE];

	//init cmd
	camera_msg version_cmd = {0x68, HOST_ASK_VERSION_OK ,0,0,0x16};
	strcpy(version_cmd.data,version);
	count = get_ack(&version_cmd);
	version_cmd.ack = count;
	memcpy(send_buf,&version_cmd,sizeof(version_cmd));
	count = send(connfd,send_buf,BUFFSIZE,0);
	if(count < 0)
	{
		fprintf(stderr,"send version error:%s\n",strerror(errno));
		return -1;
	}

}

void printf_cmd(camera_msg *cmd)
{
	cmd->data[DATA_SIZE-1] = '\0';
	printf("cmd->head=%#x\n",cmd->head);
	printf("cmd->ctr_code=%#x\n",cmd->ctr_code);
	printf("cmd->data=%s\n",cmd->data);
	printf("cmd->ack=%#x\n",cmd->ack);
	printf("cmd->end=%#x\n",cmd->end);
}

int check_cmd( camera_msg *cmd )
{
	int ack = 0;
	camera_msg *recv_cmd = cmd;
	int i;
	switch(0)
	{
		case 0: 
			if(0x68 != recv_cmd->head)
			{
				fprintf(stderr," recv cmd error\n");
				printf_cmd(recv_cmd);
				return -1;
			}
			else
			{
				ack += recv_cmd->head;
			}
		case 1:
			{
				switch(0)
				{
					case 0:
						if(HOST_ASK_VERSION == recv_cmd->ctr_code)
							break; 
					case 1:
						if(HOST_ASK_CAPTURE == recv_cmd->ctr_code)
							break;
					case 2:
						if(HOST_ASK_SETWIFI == recv_cmd->ctr_code)
							break;
					case 3:
						if(HOST_ASK_CHECKTIME == recv_cmd->ctr_code)
							break;

					default :
						fprintf(stderr,"recv cmd ctr_code error\n");
						printf_cmd(recv_cmd);
						return -1;
				}
				ack += recv_cmd->ctr_code;
			}   
		case 2:
			{
				recv_cmd->data[BUFFSIZE-1] = '\0';
				for(i = 0;i<strlen(recv_cmd->data);i++)
				{
					ack+=recv_cmd->data[i];
				}
			}
		case 3:
			if(ack != recv_cmd->ack)
			{
				fprintf(stderr,"recv cmd ack error\n");
				printf_cmd(recv_cmd);
				return -1;
			}
		case 4:
			if(0x16 !=recv_cmd->end)
			{
				fprintf(stderr,"recv cmd end error\n");
				printf_cmd(recv_cmd);
				return -1;
			}
			break;
		default: break;
	}

	return recv_cmd -> ctr_code;

}

int main(void)
{
	int i;
	int ret;
	camera_msg camer_cmd;
	struct sockaddr_in svraddr,clientaddr;
	bzero(&svraddr,sizeof(svraddr));

	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	svraddr.sin_port = htons(PORT);
	//svraddr.sin_port = htons(8033);
printf("PORT=%d\n",PORT);
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		fprintf(stderr,"Socket create error:%s\n",strerror(errno));
		//led_sign(SOCKET_ERR);
		exit(-1);
	}


	if(bind(sockfd,(struct sockaddr *)&svraddr,sizeof(svraddr))<0)
	{
		fprintf(stderr,"Socket bind error:%s\n",strerror(errno));
		//led_sign(SOCKET_ERR);
		exit(-1);
	}

	if(listen(sockfd,LISTENQ) < 0)
	{
		fprintf(stderr,"Socket listen error:%s\n",strerror(errno));
		//led_sign(SOCKET_ERR);
		exit(-1);
	}
	int count = 0;
	char recv_buff[BUFFSIZE];
	socklen_t length = sizeof(clientaddr);

	while(1)
	{
		//accept
		connfd = accept(sockfd,(struct sockaddr *)&clientaddr,&length);
		if(connfd < 0)
		{
			fprintf(stderr,"Socket connect error:%s\n",strerror(errno));
			//led_sign(SOCKET_ERR);
			exit(-1);
		}


		bzero(recv_buff,BUFFSIZE);
		while( count = recv(connfd,recv_buff,BUFFSIZE,0))
		{
			fprintf(stderr,"recv:%d byte from:\n",count);
			if(count < 0)
			{
				fprintf(stderr,"Read cmd error:%s\n",strerror(errno));
				//led_sign(CMD_ERR);
				break;
			}
			memcpy(&camer_cmd,recv_buff,sizeof(camera_msg));

#if defined(DEBUG)
			printf_cmd(&camer_cmd);
#endif
			          ret = check_cmd(&camer_cmd);
#if defined(DEBUG)
			printf("code = %d\n",ret);
#endif
			//ret = HOST_ASK_CAPTURE;
		//	ret = HOST_ASK_CAPTURE;
			ret = HOST_ASK_VERSION;
//			ret = HOST_ASK_CAPTURE;
			switch(ret)
			{
				case HOST_ASK_VERSION:
					puts("send version");
					send_version();
					break;
				case HOST_ASK_CHECKTIME:
					puts("calibrate_date");
					calibrate_date(&camer_cmd);
					break;
				case HOST_ASK_CAPTURE:
					puts("send capture");
					send_capture();
					break;
				case HOST_ASK_SETWIFI:
					puts("set_wifi");
					set_wifi(&camer_cmd);
				default:
					break;
			}


			printf("camer_cmd head:%#x,ctr_code:%#x,end:%#x\n",camer_cmd.head,camer_cmd.ctr_code,camer_cmd.end);
			bzero(recv_buff,BUFFSIZE);

		}
		close(connfd);
	}
	close(sockfd);

	return 0;
}

