#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>



int main(int argc,char *argv[])
{
	int fd;
	unsigned long  i = 1;
	unsigned long off=0;
	int ret;
	if(argc!=2){
		printf("input error.\n  led_test /dev/led \n");
		return -1;
	}

	
	fd=open("/dev/c5300",O_RDWR);
	if(fd<0){
		printf("open fail\n");
		return -1;
	}
		write(fd,&i,sizeof(i));
	
	return 0;

}
