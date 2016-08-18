#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>

#define	buff_len	128

//函数声明
int open_port(int fd,int comport);
int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop);
//char buff[]={0xff,0xfe,0xfd,0xfc,0xfb,0xfa,0xf9,0xf8,0xf7,0xf6,0xf5,0xf4,0xf3,0xf2,0xf1,0xf0};
unsigned char buff[14]={0x68,0x99,0x99,0x99,0x99,0x99,0x99,0x68,0x01,0x02,0x01,0x10,0x7A,0x16};

int main(int argc,char **argv)
{
	int x,fd;
	int nread,i;
	char dat;
	unsigned char buff_read[buff_len];
	char buff_write[]="LPC is here.\n";
	fd=open_port(fd,2);
	if(fd<0)
	{
		printf("Can't open /dev/ttyO1!!!\n");
                return -1;
	}
	x=set_opt(fd,9600,8,'N',1);
	if(x<0)
	{
		printf("Set COM2 error!!!\n");
		if(fd>0)
			close(fd);
		return -1;
	}
		printf("Set COM2 ok!!!\n");
//	while(1)
	{
#if 0
		memset(buff,dat++,sizeof(buff));
	//	printf("%s",buff_write);
		nread=read(fd,buff_read,buff_len);
		if(nread)
#endif
		{
			//printf("nread=%d,%s\n",nread,buff_read);
			write(fd,buff,nread);
		}
		sleep(1);
		read(fd,buff_read,sizeof(buff_read));
		while(1)
		{
			printf("%x\t",buff_read[i]);
			if(0x16 == buff_read[i]|| i>20)
			{
				break;
				printf("\n");
			}
			i++;
		}
#if 0
		write(fd,buff_write,sizeof(buff_write));
		write(fd,buff,16);
#endif
	}
	close(fd);
	return 0;
}

int set_opt(int fd,int nSpeed,int nBits,char nEvent,int nStop)
{
	struct termios newtio,oldtio;
	if(tcgetattr(fd,&oldtio)!=0)
	{
		perror("SetupSerial 1");
		return -1;
	}
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag|=CLOCAL|CREAD;
	newtio.c_cflag&=~CSIZE;
	switch(nBits)
	{
		case 7:
			newtio.c_cflag|=CS7;
		break;
		case 8:
			newtio.c_cflag|=CS8;
		break;
	}
	switch(nEvent)
	{
		case 'O':
		case 'o':
			newtio.c_cflag|=PARENB;
			newtio.c_cflag|=PARODD;
			newtio.c_iflag|=(INPCK|ISTRIP);
		break;
		case 'E':
		case 'e':
			newtio.c_iflag|=(INPCK|ISTRIP);
			newtio.c_cflag|=PARENB;
			newtio.c_cflag&=~PARODD;
		break;
		case 'N':
		case 'n':
			newtio.c_cflag&=~PARENB;
		break;
	}
	switch(nSpeed)
	{
		case 2400:
			cfsetispeed(&newtio,B2400);
			cfsetospeed(&newtio,B2400);
		break;
		case 4800:
			cfsetispeed(&newtio,B4800);
			cfsetospeed(&newtio,B4800);
		break;
		case 9600:
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
		break;
		case 19200:
			cfsetispeed(&newtio,B19200);
			cfsetospeed(&newtio,B19200);
		break;
		case 115200:
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);
		break;
		case 460800:
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);
		break;
		default:
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
		break;
	}
	if(nStop==1)
		newtio.c_cflag&=~CSTOPB;
	else if(nStop==2 )
		newtio.c_cflag|=CSTOPB;
#if 1
	//Serial as RAW mode
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);	//input
	newtio.c_oflag &= ~OPOST;				/*Output*/
	//Serial as RAW mode
	newtio.c_cc[VTIME]=0;
	newtio.c_cc[VMIN]=0;
	tcflush(fd,TCIFLUSH);
#endif
	if((tcsetattr(fd,TCSANOW,&newtio))!=0)
	{
		perror("COM set error!\n");
		return -1;
	}
	//printf("Set done!\n");
	return 0;
}

int open_port(int fd,int comport)
{
	char *dev[]={"/dev/ttyO0","/dev/ttyO1","/dev/ttyO2"};
	long vdisable;
	if(comport==1)
	{
		fd=open("/dev/ttyO0",O_RDWR|O_NOCTTY|O_NDELAY);
		if(-1==fd)
		{
			perror("Can't Open Serial Port\n");
			return(-1);
		}
	}
	else if(comport==2)
	{
		//fd=open("/dev/ttyO1",O_RDWR);
		fd=open("/dev/ttyO1",O_RDWR|O_NOCTTY);
		if(-1==fd)
		{
			perror("Can't Open Serial Port\n");
			return(-1);
		}else {
			printf("open ttyO1\n");
		}
	
	}
	else if(comport==3)
	{
		fd=open("/dev/ttyO2",O_RDWR|O_NOCTTY|O_NDELAY);
		if(-1==fd)
		{
			perror("Can't Open Serial Port\n");
			return(-1);
		}
	}
	else if(comport==4)
	{
		fd=open("/dev/ttyUSB0",O_RDWR|O_NOCTTY|O_NDELAY);
		if(-1==fd)
		{
			perror("Can't Open Serial Port\n");
			return(-1);
		}
	}
	/*
	if(fcntl(fd,F_SETFL,0)<0)
		printf("fcntl failed!\n");
		*/
	//else
	//	printf("fcntl=%d\n",fcntl(fd,F_SETFL,0));
	//
	/*
	if(isatty(STDIN_FILENO)==0)
		printf("Standard input is not a terminal device\n");

		*/
	//else
	//	printf("Isatty success!\n");
	//printf("fd-open=%d\n",fd);
	return fd;
}
