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


int main ()
{
	int keyboard;
	int ret,i;
	char c;
	fd_set readfd;
		struct timeval timeout;
		//keyboard = open("/sys/class/gpio/gpio45/value",O_RDONLY | O_NONBLOCK);
		keyboard = open("/home/root/test",O_RDONLY);
		
		if(keyboard < 0 )
		{
			perror("open");
			exit(1);
		}
		while(1)
		{
		timeout.tv_sec=1;
		timeout.tv_usec=0;

		FD_ZERO(&readfd);
		FD_SET(keyboard,&readfd);
		ret=select(keyboard+1,&readfd,NULL,NULL,NULL);
		if(FD_ISSET(keyboard,&readfd))
		{
		i=read(keyboard,&c,1);
		//if('\n'==c)
		//continue;
		printf("hehethe input is %c\n",c);

		//if ('q'==c)
	//	break;
}
}
}
