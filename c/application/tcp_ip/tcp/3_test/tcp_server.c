/*************************************************************************
    Function:基于TCP服务器与客户端通信,可以通过客户端访问服务器端(如./tc 192.168.1.102)，然后建立通信,
	     通信建立成功后，客户端和服务器端各自运行两个线程(read/write),就可以进行通信了，任意一方输入
	     exit,将会结束通信双方进程都将结束
    Purpose: 熟悉socket编程,掌握socket函数的用法
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
	

	/*服务器端开始建立sockfd描述符:socket()*/
	//AF_INET:IPV4; SOCK_STREAM:TCP
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*服务器端填充sockaddr结构*/
	bzero(&server_addr, sizeof(struct sockaddr_in));//初始化,置0
	server_addr.sin_family=AF_INET;   //Internet地址族:IPV4
	server_addr.sin_port = htons(PORT_NUMBERS);//将本机器上的short数据转化为网络上的
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);//(这个套接字能够使用本地任何一个地址来进行通信)将本机器上long数据转化为网络上的long数据

	/*捆绑sockfd描述符到IP地址:bind()*/
	if(bind(sockfd, (struct sockaddr*)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Bind error:%s\n\a",strerror(errno));
		exit(1);
	}

	/*设置允许连接的最大客户端数:listen()*/
	if(listen(sockfd,LISTEN_MAX)==-1)
	{
		fprintf(stderr,"Listen error:%s\n\a",strerror(errno));
		exit(1);
	}

	/*等待客户端连接后并进行读操作*/
	//while(1)
//	{
		/*服务器阻塞，直到客户程序建立连接:accept()*/
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
		
		/*等待线程结束*/
		if(pthread_join(thread_read,NULL) != 0)
		{
		printf("can't join with thread of read!\n");
		exit(1);
		}
		
		/*等待线程结束*/
		if(pthread_join(thread_write,NULL)!=0)
		{
			printf("can't join with thread of write!\n");
			exit(1);
		}

		/*这个通讯已经结束*/
		close(new_fd);
		/*下一个循环*/
	}

	/*结束通讯*/
	close(sockfd);
	
	return 0;
}

/*读线程所运行的函数*/
void *fun_read(void *arg)
{
    #endif
	int nbytes;
	fprintf(stderr,"Server get connection from %s\n",inet_ntoa(client_addr.sin_addr));
	while(1)
	{
//		nbytes=recv(new_fd,buf_read,sizeof(buf_read),0);/*注意这里没有值时将会阻塞，所以不需要延时函数延时*/
		//printf("test...\n");
		//memset(buffer,'\0',sizeof(buffer));
        
  #if 1
		if ((nbytes=recv(new_fd,buf_read,sizeof(buf_read),0))==0)/*注意这里没有值时将会阻塞，所以不需要延时函数延时*/
		{
                			/*usleep(500);其实这里==0 /==-1可以合并，但这样层次性不强*/
		}else if (nbytes==-1)
		{
		fprintf(stderr,"Read error:%s\n\a",strerror(errno));
		exit(1);
		}else
		{	
		fprintf(stderr,"From:%s and ",inet_ntoa(client_addr.sin_addr));
		buf_read[nbytes]='\0';
/*		if(strncmp(buf_read,"exit",4)==0)//只能判断前面4个字符是不是exit,有就退出,这里不完善(exitxxx也会使程序退出)
		{
			printf("the other side has exited this communication!\n");

			sleep(1);//让客户端先close(sockfd)
			//退出之前关闭socket标识符	
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

/*写线程所运行的函数*/
void *fun_write(void *arg)
{
	int nbytes = 0;

	while(1)
	{
	memset(buf_write,'\0',sizeof(buf_write));
	fgets(buf_write,sizeof(buf_write),stdin);/*写在这儿也会阻塞,需要解决没有值按回车也发送字符*/
	//gets(buffer);
	//fflush(stdin);
	
	/*发送字符*/
	if((nbytes=send(new_fd,buf_write,strlen(buf_write),0))==-1)
	{
		fprintf(stderr,"Write error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*判断是否退出此次通信*/
	if(strncmp(buf_write,"exit",4)==0)/*只能判断前面4个字符是不是exit,有就退出,这里不完善(exitxxx也会使程序退出)*/
	{
		printf("You will exit this communication with %s\n",inet_ntoa(client_addr.sin_addr));
	
		sleep(1);/*让客户端先close(sockfd)退出*/		
		/*退出之前关闭socket标识符*/	
			close(new_fd);
			close(sockfd);
		
		exit(1);/*将在这儿直接退出程序*/
	}
	
	}
	
	return (void*)0;
    #endif
    return 0;
}
/*************************************************************************
首先打开./tcp_server,再打开 ./tc 192.168.1.102
Operation results in Linux:
----------------------------------------
running normally!
----------------------------------------
Summary:
01H:注意没有网就不能进行通信,端口号要一致(why?)
02H:出现Bind error:Address already in use错误提示时,可以该端口号在进行编译通信(没有从根本上解决问题)
没有采取链接中所介绍的彻底解决方法,采用的是退出时先让客户端的先close(socket).
http://www.diybl.com/course/6_system/linux/Linuxjs/2008630/129368.html
03H:在读线程中加上system()，可以实现远程的命令操作
*************************************************************************/









