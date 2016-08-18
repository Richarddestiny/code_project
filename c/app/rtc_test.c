#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <termios.h>

void  serial_init(int fd);
int check_fd_gprs(int fd_gprs);
int ask_request(int fd,const char at[][15],const char at_reply[][30],int flag);
int send_ctl(int* fd);
void close_fd(int *fd);
int  open_modem(void);

static int fd[2];

int main(int argc,char *argv[])
{

	int flag = 0;
	int ret;

	/*check modem*/

	ret =open_modem();
	if(ret == -1)
		return -1;

	fd[1]=open("/dev/c5300",O_RDWR);
	if(fd<0){
		printf("can't find GPRS modem,check it and restart\n");
		return -1;
	}
	sleep(1);
	serial_init(fd[0]);/*bps 115200 */

	if(send_ctl(fd) == -1)
	{
		printf("modem init error!\n");
		return -1;
	}
	ret = system("/usr/sbin/pppd_real call unicom");
	if(ret <= 0)
		printf("pppd call wrong! the program exit\n");
	return 0;	
}
int  open_modem(void)
{
	fd[0] = open("/dev/ttyUSB0",O_RDWR|O_NOCTTY|O_NDELAY);
	if(fd[0]<0){
		perror("/dev/ttyUSB0\n");
		exit -1;
	}
	return 0;
}

int check_fd_gprs(int fd_gprs)
{
	if(fd_gprs < 0)
	{
		perror("/dev/ttyUSB0");
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
	options.c_cflag &= ~CSTOPB; 
	options.c_iflag |= IGNPAR;	
	options.c_oflag = 0;
	options.c_lflag = 0; 
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	tcsetattr(fd,TCSANOW,&options);
}

int ask_request(int fd,const char at[][15],const char at_reply[][30],int flag)
{
	int nread,nwrite,j,k;
	char reply[100];
	char reply_real[30];

	printf("at=%s\n",at[flag]);
	nwrite = write(fd,at[flag],strlen(at[flag]));
	printf("1_nwrite = %d, %s\n", nwrite, at[flag]);

	memset(reply,0,sizeof(reply));
	sleep(1);
	nread = read(fd,reply,sizeof(reply));

	for(j=0,k=0;j<nread;j++)
	{
		if(reply[j] == '\r' || reply[j] =='\n')
			continue;
		reply_real[k] = reply[j];
		k++;
	}
	printf("at_read  %s\n", at[flag], reply_real);

	if(strncmp(reply_real,at_reply[flag],strlen(at_reply[flag])) != 0)
	{
		return -(flag+1);
	}
	else 
		return 0;

}

int send_ctl(int* fd)
{

	const char at[][15]= {"AT\r","AT+CPIN?\r","AT^RESET\r"};
	const char at_reply[][30] = {"ATOK","AT+CPIN?+CPIN: READYOK","AT^RESETOK"};
	int reg,ret;
	char y;


	if(ask_request(fd[0],at,at_reply,0)!=0)	/*at*/
	{
		int flag = 1;
		int ret = 0;	
		printf("cmd for \"at\" modem reply  is worng!\ntry to restarting\n");
		/*hard reset*/

		ret=write(fd[1],&flag,sizeof(int));
		if(ret!=0){
			printf("hard reset error\n");
			return -1;
		}
		sleep(5);

		ret =open_modem();
		if(ret == -1)
			return -1;

		if(ask_request(fd[0],at,at_reply,0)!=0)
		{
			printf("restart failed! check the hardware\n");
			return -1;
		}
	}

	if(ask_request(fd[0],at,at_reply,1)!=0)
	{	

		ask_request(fd[0],at,at_reply,2); /*soft reset*/
		close(fd[0]);
		sleep(5);

		ret =open_modem();
		if(ret == -1)
			return -1;

		if(ask_request(fd[0],at,at_reply,1)!=0)
		{
			while(1)
			{
				printf("ERROR: SIM not inserted,check it!pls input(y/n):");
				reg = scanf("%c",&y);
				printf("%d %c\n",reg,y);
				if(reg !=1 || getchar()!='\n' || (y!='y' && y!='n'&&y!='Y'&&y!='N'))
				{	
					printf("input error!\n");
					continue;
				}
				if('y' == y||'Y' == y)
				{	



					ask_request(fd[0],at,at_reply,2);  /*soft reset*/
					close(fd[0]);
					sleep(5);

					ret =open_modem();
					if(ret == -1)
						return -1;

					if(ask_request(fd[0],at,at_reply,1)!=0)
					{
						printf("modem check error!\n");
						return -1;	
					}
					else
						return 0;
				}
				else
					if('n' == y||'N' == y)
					{
						printf("pppd exit!\n");
						return 0;
					}
			}


		}
	}

	printf("call send over\n");	
}



