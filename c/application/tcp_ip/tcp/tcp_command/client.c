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
#include "client.h"

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
    unsigned int  seqno;
    unsigned int  iframe;
    unsigned int frame_id;
    unsigned int len[4];
}net_enc;

int data_pthread(char *argv[])
{
    int sockfd, sendbytes;
    char buf[BUFFER_SIZE];
    struct hostent *host;
    struct sockaddr_in serv_addr;
    FILE *fp[4];
    int ret;
    int i,j = 0;
    int file_flag = 0;
    int recv_len;

    file_flag = strtol(argv[3],NULL,10);
    printf("file_flag:%#x\n",file_flag);

    /*地址解析函数*/
    if ((host = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    memset(buf, 0, sizeof(buf));
    for(i = 0;i < 4 ;i++)
    {
        sprintf(buf, "video/%s_%d.264", argv[2],i+1);
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

{
    /*调用connect函数主动发起对服务器端的连接*/
    if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr))== -1)
    {
        perror("connect");
        exit(1);
    }
	else
	printf("connect data line success\n");
    /*发送消息给服务器端*/
    dock.buf[0] = '1';
    if ((sendbytes = send(sockfd, &dock, sizeof(dock), 0)) == -1)
    {
        perror("send");
        exit(1);
    }
    else
    printf("send:%d\n",sendbytes);
	sem_post(&sem_stream);
    while(1)
    {
        sendbytes = 0;
        while(sendbytes < sizeof(net_enc))
        {
            recv_len = recv(sockfd,&buf[sendbytes],sizeof(net_enc)-sendbytes,0);
            if(recv_len <= 0)
            {
                perror("recv frame ");
 
                close(sockfd);
                return -1;
            }
            sendbytes += recv_len;
        }
        memcpy(&net_enc,buf,sizeof(net_enc));
        int enc_len = net_enc.len[0] + net_enc.len[1] + net_enc.len[2] + net_enc.len[3];
     //   printf("head:%d,need recv:%d\n",sendbytes,enc_len,net_enc.len[0],net_enc.len[1]);
        sendbytes = 0;
        while(sendbytes < enc_len)
        {
            recv_len = recv(sockfd,&buf[sendbytes],(enc_len - sendbytes),0);
            if(recv_len <= 0)
            {
                perror("recv frame ");
                return -1;
            }
            sendbytes += recv_len;
        }
#if 0        
        printf("recv size;%d i_frame:%d frame_id:%d video1:%d,video2:%d,video3:%d,video4:%d\n",sendbytes,net_enc.iframe,net_enc.seqno,net_enc.len[0],net_enc.len[1],net_enc.len[2],net_enc.len[3]);
            printf("video0 info ");
            for(i = 0; i < 10 ;i++)
            {
                printf("%#2x ",buf[i]);
            }
            printf("\n");
            printf("video1 info ");
            for(i = 0; i < 10 ;i++)
            {
                printf("%#2x ",buf[net_enc.len[0]+i]);
            }
            printf("\n");
            printf("video2 info ");
            for(i = 0; i < 10 ;i++)
            {
                printf("%#2x ",buf[net_enc.len[0]+net_enc.len[1]+i]);
            }
            printf("\n");
            printf("video3 info ");
            for(i = 0; i < 10 ;i++)
            {
                printf("%#2x ",buf[net_enc.len[0]+net_enc.len[1]+net_enc.len[2]+i]);
            }
            printf("\n");
#endif            
        if(file_flag == 1)
        {
            if(net_enc.len[0] != 0)
            ret = fwrite(buf,net_enc.len[0],1,fp[0]);
            if( 1 != ret )
            {
                printf("line 1 ret:%d\n",ret);
                perror("fwrite");
                return -1;
            }
            if(net_enc.len[1] != 0)
            {
                ret = fwrite(&buf[net_enc.len[0]],net_enc.len[1],1,fp[1]);
                if( 1 != ret )
                {
                    printf("line 2 ret:%d\n",ret);
                    perror("fwrite");
                    return -1;
                }
            }
            if(net_enc.len[2] != 0)
            {
                ret = fwrite(&buf[net_enc.len[0]+net_enc.len[1]],net_enc.len[2],1,fp[2]);
                if( 1 != ret )
                {
                    printf("line 3 ret:%d\n",ret);
                    perror("fwrite");
                    return -1;
                }
            }
            if(net_enc.len[3] != 0)
            {
                ret = fwrite(&buf[enc_len-net_enc.len[3]],net_enc.len[3],1,fp[3]);
                if( 1 != ret )
                {
                    printf("line 4 ret:%d\n",ret);
                    perror("fwrite");
                    return -1;
                }

            }
        }
    }
}

    for(i=0;i<4;i++)
    fclose(fp[i]);
    close(sockfd);

return 0;
}
