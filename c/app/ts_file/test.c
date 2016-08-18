#include <sys/types.h>          /* See NOTES */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
 #include <poll.h>
int main(void)
{
    int gpio_fd,ret;
	int i;
	int key_stauts;
	char buff[20];
	struct pollfd fds[1];
	gpio_fd = open("/sys/class/gpio/gpio45/value",O_RDONLY);
	if( gpio_fd == -1 )
	   printf("gpio open");
	fds[0].fd = gpio_fd;
	fds[0].events  = POLLPRI;
	ret = read(gpio_fd,buff,10);
	if( ret == -1 )
		printf("read");
	while(1){
		 ret = poll(fds,1,-1);
		 if( ret == -1 )
		     printf("poll");
		   if( fds[0].revents & POLLPRI){
		       ret = lseek(gpio_fd,0,SEEK_SET);
		       if( ret == -1 )
		           printf("lseek");
		       ret = read(gpio_fd,buff,10);
		       if( ret == -1 )
		           printf("read");
				printf("test %s\n",buff);
		       
		}
}
}
