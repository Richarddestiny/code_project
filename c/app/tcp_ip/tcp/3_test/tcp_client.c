#include <stdio.h>
#include <sys/socket.h>/*socket() and so on*/
#include <string.h>/*string.h*/
#include <arpa/inet.h>/*htonl()*/
#include <netdb.h>/*gethostbyname()*/
#include <errno.h>/*errno*/
#include <stdlib.h>/*exit()*/
#include <unistd.h>/*close()*/
#include <sys/types.h> 
#include <pthread.h>/*pthread_create()/pthread_join()/pthread_mutex_lock()/pthread_mutex_unlock()*/

#define PORT_NUMBERS 6000

void *fun_write(void *arg);
void *fun_read(void *arg);

int sockfd;
struct sockaddr_in server_addr;
char buf_write[1024];
char buf_read[1024];
int main(int argc, char *argv[])
{
	/*int sockfd;*/
	/*char buffer[1024];*/
	
	struct hostent *host;

	pthread_t thread_write;
	pthread_t thread_read;

	/*使用hostname查询host名字*/
	if(argc!=2)
	{
		fprintf(stderr,"Usage:%s hostname\n\a",argv[0]);
		exit(1);
	}

	if((host=gethostbyname(argv[1]))==NULL)
	{
		fprintf(stderr,"Gethostname error!\n");
		exit(1);
	}

	/*客户程序开始建立sockfd描述符号:socket()*/
	//AF_INET:IPV4; SOCK_STREAM:TCP
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		fprintf(stderr,"Socket error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*客户端填充sockaddr结构*/
	bzero(&server_addr, sizeof(server_addr));//初始化,置0
	server_addr.sin_family=AF_INET;   //Internet地址族:IPV4
	server_addr.sin_port = htons(PORT_NUMBERS);//将本机器上的short数据转化为网络上的short数据
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);//IP地址
	
	/*客户端发起连接请求*/
	if(connect(sockfd,(struct sockaddr*)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		fprintf(stderr,"Connect error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*连接成功了*/
	printf("Please input char:\n");
#if 0	
	if(pthread_create(&thread_write,NULL,fun_write,NULL) != 0)
	{
		printf("create pthread of write error!\n");
		return -1;
	}	

	if(pthread_create(&thread_read,NULL,fun_read,NULL) != 0)
		{
			printf("create pthread of read error!\n");
			return -1;
		}	
		
	/*等待线程结束*/
	if(pthread_join(thread_write,NULL)!=0)
	{
		printf("can't join with thread of write!\n");
		exit(1);
	}

	/*等待线程结束*/
		if(pthread_join(thread_read,NULL) != 0)
		{
		printf("can't join with thread of read!\n");
		exit(1);
		}
	
	/*结束通讯*/
	close(sockfd);
	
	return 0;
}

/*写线程所运行的函数*/
void *fun_write(void *arg)
{
    #endif
	int nbytes = 0;

	while(1)
	{
	memset(buf_write,'\0',sizeof(buf_write));
	fgets(buf_write,sizeof(buf_write),stdin);/*写在这儿也会阻塞,需要解决没有值按回车也发送字符*/
	//gets(buffer);
	//fflush(stdin);
	
	/*发送字符*/
	if((nbytes=send(sockfd,buf_write,strlen(buf_write),0))==-1)
	{
		fprintf(stderr,"Write error:%s\n\a",strerror(errno));
		exit(1);
	}
	
	/*判断是否退出此次通信*/
	if(strncmp(buf_write,"exit",4)==0)/*只能判断前面4个字符是不是exit,有就退出,这里不完善(exitxxx也会使程序退出)*/
	{
		printf("You will exit this communication! with %s\n",inet_ntoa(server_addr.sin_addr));
		close(sockfd);/*退出之前关闭socket标识符*/
		exit(1);/*将在这儿直接退出程序*/
		//break;
	}
	
	}
#if 0	
	return (void*)0;
}

void *fun_read(void *arg)
{
	int nbytes;
	fprintf(stderr,"client get connection from %s\n",inet_ntoa(server_addr.sin_addr));
	while(1)
	{
		//memset(buffer,'\0',sizeof(buffer));
		if ((nbytes=recv(sockfd,buf_read,sizeof(buf_read),0))==0)/*注意这里没有值时将会阻塞，所以不需要延时函数延时*/
		{
			/*usleep(500);其实这里==0 /==-1可以合并，但这样层次性不强*/
		}else if (nbytes==-1)
		{
		fprintf(stderr,"Read error:%s\n\a",strerror(errno));
		exit(1);
		}else
		{	
		fprintf(stderr,"From:%s and ",inet_ntoa(server_addr.sin_addr));
		buf_read[nbytes]='\0';
		if(strncmp(buf_read,"exit",4)==0)/*只能判断前面4个字符是不是exit,有就退出,这里不完善(exitxxx也会使程序退出)*/
		{
			printf("the other side has exited this communication!\n");
			close(sockfd);/*退出之前关闭socket标识符*/			
			exit(1);
			//break;
		}
		printf("Client received %d character \n%s\n",nbytes,buf_read);
		//system(buf_read);/*可以执行命令*/
		//printf("test\n");
		}
	}	
	
//	return NULL;
    	#endif
	return 0;
}


