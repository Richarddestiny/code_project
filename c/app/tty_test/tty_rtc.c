#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>



int check_fd(int fd_gprs)
{
	if(fd_gprs < 0)
	{
		perror("fd_gprs is error!");
		return fd_gprs;
	}
	return 1;
}


void  serial_init(int fd)
{
	if (check_fd(fd) < 0){
		return;
	}
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag |= ( CLOCAL | CREAD );/*input mode flag:ignore modem 
										  control lines; Enable receiver */
	options.c_cflag &= ~CSIZE;//屏蔽其他标志位
	options.c_cflag &= ~CRTSCTS;//不使用数据流控制
	options.c_cflag |= CS8;  //8位数据位
	options.c_cflag &= ~CSTOPB; //停止位
	options.c_cflag &= ~PARENB; //无奇偶效验
    options.c_iflag &= ~INPCK;
	options.c_iflag |= IGNPAR;	// 忽略校验错误
	options.c_oflag = 0;// 无输出模式
	options.c_lflag = 0; //本地模式禁用
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	tcsetattr(fd,TCSANOW,&options);
}



int send(int fd)//发送函数，用于AT指令的发送
{
	int nread,nwrite;
	unsigned char buff[]={0x68,0x99,0x99,0x99,0x99,0x99,0x99,0x68,0x01,0x02,0x01,0x10,0x7A,0x16};
	unsigned char reply[128];
	int i=0;

	memset(reply,0,sizeof(reply));

	nwrite = write(fd,buff,sizeof(buff));
	printf("1_nwrite = %d\n", nwrite);
#if 1

	nread = read(fd,reply,sizeof(reply));
	while(1)
	{

		printf("%x\t",reply[i]);
		if(0x16 == reply[i]|| i>25)
		{
			printf("\n");
			break;
		}
		i++;
	}
#endif
#if 0
	memset(buff,0,sizeof(buff));
	strcpy(buff,"AT^RESET");
	strcat(buff,"\r");
	nwrite = write(fd,buff,strlen(buff));
	printf("3_nwrite = %d, %s\n", nwrite, buff);

	memset(reply,0,sizeof(reply));
	sleep(1);
	nread = read(fd,reply,sizeof(reply));
	printf("4_nread = %d, %s\n", nread, reply);
#endif


	printf("call send over\n");	
	return 0;
}

int main(int argc,char **argv)
{

	int fd;
	if((fd =open("/dev/ttyO1", O_RDWR))<0)
	//if((fd =open("/dev/ttyO1", O_RDWR | O_NOCTTY |O_NDELAY))<0)
	{
		perror("/dev/ttyO1");
		return -1;
	}
	serial_init(fd);
	send(fd);
	close(fd);

return 0;
}

