#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define RTC_SET_TIME	0x4024700a
#define DEBUG 1 

typedef struct tagFRAME_DATA_PROP
{
	unsigned char L1_L2;
	unsigned char app_len;
	unsigned char app_cs;

	unsigned char rx_buf[128];
	unsigned char rx_idx;
	unsigned char rx_flag;
	unsigned char rx_time;

	unsigned char rx_frame_ok;

} FRAME_DATA_PROP, *PFRAME_DATA_PROP;


void set_sys_time(PFRAME_DATA_PROP p_data_prop)
{
	int flag;

}
int check_data(unsigned char rx_data[], int num,PFRAME_DATA_PROP p_data_prop)
{
	int err_flag = 20;

	while(num >= p_data_prop->rx_idx+1 )
	{

		switch( p_data_prop->rx_idx )
		{
			case 0:
				if( rx_data[p_data_prop->rx_idx] == 0x68 )
				{
					p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
					p_data_prop->rx_flag = 1;
					p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];
				}
				else
				{
					p_data_prop->rx_idx = 0;
					err_flag--;

				}		

				break;
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6: // 地址域
				p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
				p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];

				break;
			case 7:
				if( rx_data[p_data_prop->rx_idx] == 0x68 )
				{
					p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
					p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];
				}
				else
				{
					p_data_prop->rx_idx = 0;
					err_flag--;

				}

				break;
			case 8: // 控制码
				p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
				p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];		

				break;
			case 9: // 数据长度
				p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
				p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];
				p_data_prop->L1_L2 = rx_data[p_data_prop->rx_idx -1];
				p_data_prop->app_len = 0;

				break;
			default:

				if( p_data_prop->rx_idx >= 10 && p_data_prop->rx_frame_ok == 0 )
				{
					if( p_data_prop->app_len < p_data_prop->L1_L2 ) // 数据
					{	
						p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];

						p_data_prop->app_len++;
						p_data_prop->app_cs += rx_data[p_data_prop->rx_idx - 1];	

					}
					else if( p_data_prop->app_len == p_data_prop->L1_L2 ) // CS
					{

						if( p_data_prop->app_cs == rx_data[p_data_prop->rx_idx] )
						{
							p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
							p_data_prop->app_len++;
						}
						else
						{
							p_data_prop->rx_idx = 0;
							err_flag--;

						}
					}
					else
					{
						if( rx_data[p_data_prop->rx_idx] == 0x16 && p_data_prop->app_len >= 1 )
						{
							p_data_prop->rx_buf[p_data_prop->rx_idx++] = rx_data[p_data_prop->rx_idx];
							p_data_prop->rx_frame_ok = 1;
						}
						else
						{
							p_data_prop->rx_idx = 0;
							err_flag--;


						}
					}
				}

				break;
		}
	}

	if(err_flag <0)
		return -1;
	return 0;
}



void set_termios(int fd_uart)
{
	struct termios newtio,oldtio;

	tcgetattr(fd_uart,&oldtio);
	bzero(&newtio,sizeof(newtio) );
	newtio.c_cflag  |=  CLOCAL | CREAD; 
	newtio.c_cflag &= ~CSIZE; 
	newtio.c_cflag |= CS8;
	newtio.c_cflag &= ~PARENB;
	cfsetispeed(&newtio, B9600);
	cfsetospeed(&newtio, B9600);
	newtio.c_cflag &=  ~CSTOPB;

	tcflush(fd_uart,TCIFLUSH);
	tcsetattr(fd_uart,TCSANOW,&newtio);
#if DEBUG
	printf("set done!\n");
#endif
}
int main(int argc,char **argv)
{
	int fd_uart = 0,fd_rtc =0;	
	int nread,nwrite;
	unsigned char read_date[14]= {0x68,0x01,0x00,0x00,0x00,0x00,0x13,0x68,0x01,0x02,0x04,0x10,0xfb,0x16,};
	unsigned char reply[128];
	int i=0;
	FRAME_DATA_PROP data_prop;

	struct rtc_time {
		int tm_sec;
		int tm_min;
		int tm_hour;
		int tm_mday;
		int tm_mon;
		int tm_year;
		int tm_wday;
		int tm_yday;
		int tm_isdst;
	}rtc_time;


	if((fd_uart = open( "/dev/ttyO1", O_RDWR | O_NOCTTY | O_NDELAY) ) < 0)
	{
		perror("open ttyO1");
		return -1;
	}
	if((fd_rtc = open("/dev/rtc" ,O_RDWR)) < 0)
	{
		perror("open ttyO1");
		return -1;
	}
	set_termios(fd_uart);
	while(1)
{
	i = 0;
	nwrite = write(fd_uart,read_date,sizeof(read_date)/sizeof(unsigned char));
	if(nwrite < 0)
	{
		perror("write");
		return -1;
	}
	printf("reading\n");
	sleep(5);
	nread = read(fd_uart,reply,sizeof(reply));
	if(nread <= 0)
	{
		printf("read error\n");
		return -1;
	}
	memset(&data_prop,0,sizeof(data_prop));
	printf("nread =%d\n",nread);
	while(1)
	{

		printf("%x\t",reply[i]);
		if(0x16 == reply[i]|| i>30)
		{
			printf("\n");
			break;
		}
		i++;
	}
}
#if DEBUG
	if(check_data(reply,nread,&data_prop) < 0)
	{
		perror("date_decode");
		return -1;
	}


	memset(&rtc_time,0,sizeof(rtc_time));

	rtc_time.tm_sec= data_prop.rx_buf[18];
	rtc_time.tm_min= data_prop.rx_buf[17];
	rtc_time.tm_hour= data_prop.rx_buf[16];
	rtc_time.tm_wday= data_prop.rx_buf[15];
	rtc_time.tm_mday= data_prop.rx_buf[14];
	rtc_time.tm_mon= data_prop.rx_buf[13]-1;
	rtc_time.tm_year= 100+data_prop.rx_buf[12];
	ioctl(fd_rtc,RTC_SET_TIME,&rtc_time);
#endif
	close(fd_rtc);
	close(fd_uart);
	system("hwclock -s;date");
#if DEBUG
	printf("get real time clock ok!\n");
#endif
	return 0;
}
