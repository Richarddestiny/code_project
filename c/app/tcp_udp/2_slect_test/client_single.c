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

#define PORT	7777
#define BUFFER_SIZE 0x280000
#define DOCK_BUF 1024
struct dock {
        char magic_key[5];
        int cmd_type;
        int data_size;
        char buf[DOCK_BUF];

}dock = {"EvoS"};

struct netstr {
    char magic_key[5];
    int seqno;
    unsigned int iframe;
    unsigned int frame_id;
    unsigned int len[4];
}net_enc = {"EvoS"};

int main(int argc, char *argv[])
{
	int sockfd, sendbytes;
	char buf[BUFFER_SIZE];
	struct hostent *host;
	struct sockaddr_in serv_addr;
	FILE *fp[4];
    int ret;
    int i;
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
    for(i = 0;i < 4 ;i++)
    {
	sprintf(buf, "%s_%d.264", argv[2],i+1);
    fp[i] = fopen(buf,"wb");
    if(fp[i] == NULL)
    {
        perror("fopen");
        return 0;
    }
    }
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
	
	/*调用connect函数主动发起对服务器端的连接*/
	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr))== -1)
	{
		perror("connect");
		exit(1);
	}
	
	/*发送消息给服务器端*/
    dock.buf[0] = '1';
	if ((sendbytes = send(sockfd, &dock, sizeof(dock), 0)) == -1)
	{
		perror("send");
		exit(1);
	}
    else
        printf("send:%d\n",sendbytes);
    while(1)
    {
        sendbytes = recv(sockfd,&net_enc,sizeof(net_enc),0);
        if(sendbytes != sizeof(net_enc))
        {
            printf("net_enc err,exit\n");
            exit(-1);
        }
        int enc_len = net_enc.len[0] + net_enc.len[1] + net_enc.len[2] + net_enc.len[3];
        printf("head:%d,need recv:%d\n",sendbytes,enc_len,net_enc.len[0],net_enc.len[1]);
        sendbytes = 0;
        while(sendbytes < enc_len)
        {
            int recv_len = recv(sockfd,&buf[sendbytes],(enc_len - sendbytes),0);
            if(recv_len <= 0)
            {
                printf("recv enc err,exit\n");
                exit(-1);
            }
            sendbytes += recv_len;
        }
        printf("recv size;%d video1:%d,video2:%d,video3:%d,video4:%d\n",sendbytes,net_enc.len[0],net_enc.len[1],net_enc.len[2],net_enc.len[3]);
        ret = fwrite(buf,net_enc.len[0],1,fp[0]);
        if( 1 != ret )
        {
            printf("ret:%d\n",ret);
            perror("fwrite");
            return -1;
        }
        #if 0
        ret = fwrite(&buf[net_enc.len[0]],net_enc.len[1],1,fp[1]);
        if( 1 != ret )
        {
            printf("ret:%d\n",ret);
            perror("fwrite");
            return -1;
        }
        ret = fwrite(&buf[net_enc.len[0]+net_enc.len[1]],net_enc.len[2],1,fp[2]);
        if( 1 != ret )
        {
            printf("ret:%d\n",ret);
            perror("fwrite");
            return -1;
        }
        ret = fwrite(&buf[enc_len-net_enc.len[3]],net_enc.len[3],1,fp[3]);
        if( 1 != ret )
        {
            printf("ret:%d\n",ret);
            perror("fwrite");
            return -1;
        }
        #endif
    }
    for(i=0;i<4;i++)
    fclose(fp[i]);
	close(sockfd);
	exit(0);
}
