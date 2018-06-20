#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>



int check_fd_gprs(int fd_gprs)
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
	if (check_fd_gprs(fd) < 0){
		return;
	}
	struct termios options;
	tcgetattr(fd, &options);
	options.c_cflag |= ( CLOCAL | CREAD );/*input mode flag:ignore modem 
										  control lines; Enable receiver */
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag |= CS8;
	options.c_cflag &= ~CSTOPB; //停止位
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
	char buff[128];
	char reply[128];

	memset(buff,0,sizeof(buff));
	strcpy(buff,"AT\r");
	nwrite = write(fd,buff,strlen(buff));
	printf("1_nwrite = %d, %s\n", nwrite, buff);

	memset(reply,0,sizeof(reply));
	sleep(1);
	nread = read(fd,reply,sizeof(reply));
	printf("2_nread = %d, %s\n", nread, reply);


	memset(buff,0,sizeof(buff));
	strcpy(buff,"AT^RESET");
	strcat(buff,"\r");
	nwrite = write(fd,buff,strlen(buff));
	printf("3_nwrite = %d, %s\n", nwrite, buff);

	memset(reply,0,sizeof(reply));
	sleep(1);
	nread = read(fd,reply,sizeof(reply));
	printf("4_nread = %d, %s\n", nread, reply);



	printf("call send over\n");	
}

int main(int argc,char **argv)
{

	int fd;
	if((fd =open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY))<0)
	{
		perror("/dev/ttyUSB0");
		return -1;
	}
		
	serial_init(fd);
	send(fd);


return 0;
}

