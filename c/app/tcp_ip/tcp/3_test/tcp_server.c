/*************************************************************************
    Function:����TCP��������ͻ���ͨ��,����ͨ���ͻ��˷��ʷ�������(��./tc 192.168.1.102)��Ȼ����ͨ��,
	     ͨ�Ž����ɹ��󣬿ͻ��˺ͷ������˸������������߳�(read/write),�Ϳ��Խ���ͨ���ˣ�����һ������
	     exit,�������ͨ��˫�����̶�������
    Purpose: ��Ϥsocket���,����socket�������÷�
    Author:  ZJQ and Others     
    Time:    2012-09-11
*************************************************************************/
#include <stdio.h>
#include <sys/socket.h>//socket()
#include <arpa/inet.h>//htonl()and so on
#include <stdlib.h>//exit()
#include <errno.h>//errno
#include <unistd.h>//close()
#include <string.h>//bzero()
#include <pthread.h>/*pthread_create()/pthread_join()/pthread_mutex_lock()/pthread_mutex_unlock()*/


#define PORT_NUMBERS 6000
#define LISTEN_MAX 5

void *fun_read(void *arg);
void *fun_write(void *arg);

int sockfd, new_fd;
struct sockaddr_in server_addr;
struct sockaddr_in client_addr;
//int nbytes;
char buf_read[1024];
char buf_write[1024];

int main(int argc, char * argv[])
{
	
	int sin_size;
	/*int nbytes;
	char buffer[1024];*/

	pthread_t thread_write;
	pthread_t thread_read;
	

	/*�������˿�ʼ����sockfd������:socket()*/
	//AF_INET:IPV4; SOCK_STREAM:TCP
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*�����������sockaddr�ṹ*/
	bzero(&server_addr, sizeof(struct sockaddr_in));//��ʼ��,��0
	server_addr.sin_family=AF_INET;   //Internet��ַ��:IPV4
	server_addr.sin_port = htons(PORT_NUMBERS);//���������ϵ�short����ת��Ϊ�����ϵ�
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//(����׽����ܹ�ʹ�ñ����κ�һ����ַ������ͨ��)����������long����ת��Ϊ�����ϵ�long����

	/*����sockfd��������IP��ַ:bind()*/
	if(bind(sockfd, (struct sockaddr*)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}

	/*�����������ӵ����ͻ�����:listen()*/
	if(listen(sockfd,LISTEN_MAX)==-1)
	{
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
		exit(1);
	}

	/*�ȴ��ͻ������Ӻ󲢽��ж�����*/
	//while(1)
//	{
		/*������������ֱ���ͻ�����������:accept()*/
		sin_size = sizeof(struct sockaddr_in);
		if((new_fd=accept(sockfd,(struct sockaddr*)(&client_addr),(socklen_t *)&sin_size))==-1)
		{
		fprintf(stderr,"Accept error:%s\n\a",strerror(errno));
		exit(1);
		}


#if 0
		if(pthread_create(&thread_read,NULL,fun_read,NULL) != 0)
		{
			printf("create pthread of read error!\n");
			return -1;
		}	

		
		if(pthread_create(&thread_write,NULL,fun_write,NULL) != 0)
		{
			printf("create pthread of write error!\n");
			return -1;
		}	
		
		/*�ȴ��߳̽���*/
		if(pthread_join(thread_read,NULL) != 0)
		{
		printf("can't join with thread of read!\n");
		exit(1);
		}
		
		/*�ȴ��߳̽���*/
		if(pthread_join(thread_write,NULL)!=0)
		{
			printf("can't join with thread of write!\n");
			exit(1);
		}

		/*���ͨѶ�Ѿ�����*/
		close(new_fd);
		/*��һ��ѭ��*/
	}

	/*����ͨѶ*/
	close(sockfd);
	
	return 0;
}

/*���߳������еĺ���*/
void *fun_read(void *arg)
{
    #endif
	int nbytes;
	fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr));
	while(1)
	{
//		nbytes=recv(new_fd,buf_read,sizeof(buf_read),0);/*ע������û��ֵʱ�������������Բ���Ҫ��ʱ������ʱ*/
		//printf("test...\n");
		//memset(buffer,'\0',sizeof(buffer));
        
  #if 1
		if ((nbytes=recv(new_fd,buf_read,sizeof(buf_read),0))==0)/*ע������û��ֵʱ�������������Բ���Ҫ��ʱ������ʱ*/
		{
                			/*usleep(500);��ʵ����==0 /==-1���Ժϲ�������������Բ�ǿ*/
		}else if (nbytes==-1)
		{
		fprintf(stderr,"Read error:%s\n\a",strerror(errno));
		exit(1);
		}else
		{	
		fprintf(stderr,"From:%s and ",inet_ntoa(client_addr.sin_addr));
		buf_read[nbytes]='\0';
/*		if(strncmp(buf_read,"exit",4)==0)//ֻ���ж�ǰ��4���ַ��ǲ���exit,�о��˳�,���ﲻ����(exitxxxҲ��ʹ�����˳�)
		{
			printf("the other side has exited this communication!\n");

			sleep(1);//�ÿͻ�����close(sockfd)
			//�˳�֮ǰ�ر�socket��ʶ��	
			close(new_fd);
			close(sockfd);

			exit(1);
			//break;
		}

            */
		printf("Server received %d character \n%s\n",nbytes,buf_read);
		}
            #endif
	//	buf_read[nbytes]='\0';
//		printf("Server received %d character \n%s\n",nbytes,buf_read);
	}	
#if 0	
	return NULL;
}

/*д�߳������еĺ���*/
void *fun_write(void *arg)
{
	int nbytes = 0;

	while(1)
	{
	memset(buf_write,'\0',sizeof(buf_write));
	fgets(buf_write,sizeof(buf_write),stdin);/*д�����Ҳ������,��Ҫ���û��ֵ���س�Ҳ�����ַ�*/
	//gets(buffer);
	//fflush(stdin);
	
	/*�����ַ�*/
	if((nbytes=send(new_fd,buf_write,strlen(buf_write),0))==-1)
	{
		fprintf(stderr,"Write error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*�ж��Ƿ��˳��˴�ͨ��*/
	if(strncmp(buf_write,"exit",4)==0)/*ֻ���ж�ǰ��4���ַ��ǲ���exit,�о��˳�,���ﲻ����(exitxxxҲ��ʹ�����˳�)*/
	{
		printf("You will exit this communication with %s\n",inet_ntoa(client_addr.sin_addr));
	
		sleep(1);/*�ÿͻ�����close(sockfd)�˳�*/		
		/*�˳�֮ǰ�ر�socket��ʶ��*/	
			close(new_fd);
			close(sockfd);
		
		exit(1);/*�������ֱ���˳�����*/
	}
	
	}
	
	return (void*)0;
    #endif
    return 0;
}
/*************************************************************************
���ȴ�./tcp_server,�ٴ� ./tc 192.168.1.102
Operation results in Linux:
----------------------------------------
running normally!
----------------------------------------
Summary:
01H:ע��û�����Ͳ��ܽ���ͨ��,�˿ں�Ҫһ��(why?)
02H:����Bind error:Address already in use������ʾʱ,���Ըö˿ں��ڽ��б���ͨ��(û�дӸ����Ͻ������)
û�в�ȡ�����������ܵĳ��׽������,���õ����˳�ʱ���ÿͻ��˵���close(socket).
http://www.diybl.com/course/6_system/linux/Linuxjs/2008630/129368.html
03H:�ڶ��߳��м���system()������ʵ��Զ�̵��������
*************************************************************************/









