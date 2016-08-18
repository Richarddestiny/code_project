/*************************************************************************
	> File Name: irq_value.c
	> Author: richard
	> Mail: freedom_wings@foxmail.com 
	> Created Time: 2013年12月30日 星期一 18时45分14秒
 ************************************************************************/


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

//#define DEBUG

int main(void)
{
    int gpio_fd[2],ret;
	int i;
	int key_status[2];
    char buff[2];
    char file_name[][30]={"/sys/class/gpio/gpio45/value","/sys/class/gpio/gpio46/value"};
    struct pollfd fds[2];

#if defined(DEBUG)
		int flag = 0;
#endif

	system("echo both > /sys/class/gpio/gpio45/edge;echo both > /sys/class/gpio/gpio46/edge");
  
  for(i = 0; i < 2;i++)
    {
        gpio_fd[i] = open(file_name[i],O_RDONLY);
		if( gpio_fd[i] == -1 )
			{
				 perror("gpio open");
				 exit(1);
			}

		fds[i].fd = gpio_fd[i];
		fds[i].events  = POLLPRI;
#if 0
		ret = read(gpio_fd[i],buff,2);

		if( ret == -1 )
			{
				 perror("read");
				 exit(1);
			}

#if defined(DEBUG)
	
		printf("fd =%d\t buff=%s\t\t",gpio_fd[i],buff);

#endif
#endif
	}

#if defined(DEBUG)
		printf("\n");
#endif

	while(1){
		
		 ret = poll(fds,2,-1);
		 if( ret == -1 )
			{
				 perror("poll");
				 exit(1);
			}
		   if( fds[0].revents & POLLPRI){
				i = 0;			
				}
			else{
				i =	1;
				}
		       ret = lseek(gpio_fd[i],0,SEEK_SET);
		      if( ret == -1 )
			{
				 perror("lseek");
				 exit(1);
			}
		    
#if defined(DEBUG)
		flag++;
		printf(" %d value %c  key %s %s\n",flag,buff[0],(i==0?"45":"46"),(buff[0]=='1'?"up":"down"));

#endif
				ret = read(gpio_fd[i],buff,2);
		       if( ret == -1 )
		           perror("read");

  			   key_status[i] = (buff[0] == '1'?0:1); //status up 0  down 1
				if(key_status[0] ==1 && key_status[1] ==1)
					{
						printf("chose to ts_cllibrate\n");
					}
			
	}
}
