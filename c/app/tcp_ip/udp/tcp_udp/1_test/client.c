/*client.c*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#define PORT	5556
#define BUFFER_SIZE 0x20000

int main(int argc, char *argv[])
{
	int sockfd, sendbytes;
	char buf[BUFFER_SIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	
	if(argc < 3)
	{
		fprintf(stderr,"USAGE: ./client Hostname(or ip address) Text\n");
		exit(1);
	}
	
	/*地址解析函数*/
	if ((host = gethostbyname(argv[1])) == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
	
	memset(buf, 0, sizeof(buf));
	sprintf(buf, "%s", argv[2]);
	
	/*创建socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(1);
	}
	
	/*设置sockaddr_in 结构体中相关参数*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serv_addr.sin_zero), 8);
	
    printf("%d\n",__LINE__);
	/*调用connect函数主动发起对服务器端的连接*/
	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr))== -1)
	{
		perror("connect");
		exit(1);
	}
    printf("%d\n",__LINE__);
	int i;
    for(i = 0;i < BUFFER_SIZE-0x100; i++)
    {
        buf[i] = i%25+0x41;
    }
	/*发送消息给服务器端*/
    printf("%d\n",__LINE__);
    while(1)
    {
	if ((sendbytes = send(sockfd, buf, strlen(buf), 0)) == -1)
	{
		perror("send");
		exit(1);
	}
    else
        printf("size:%d \n",sendbytes);
        sleep(1);
    }
	close(sockfd);
	exit(0);
}
