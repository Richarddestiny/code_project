#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/input.h>

#define TS_UNABLE 0
#define TS_ENABLE 1
int main(int argc,char **argv)
{
 
    char          buf[256] = { 0, };  /* RATS: Use ok */
    int           fd = 0;
    int           i;
    int           rc;
    int value = 0;

    if (argc < 2)
    {
        printf("test ts_ctrl  or  test ts_able ts_unable  ts_enable\n");
        return 0;
    }
	 if ((fd = open("/dev/ts_ctrl", O_RDWR)) >= 0) {
            printf(" open:fd = %d\n",fd);

			if( strcmp("ts_able",argv[1]) == 0) 
				 {
				 while(1)
				 {
					printf("ts unableing\n");
					ioctl(fd,TS_UNABLE,NULL);
					 sleep(10);
					printf("ts enableing\n");
					 ioctl(fd,TS_ENABLE,NULL);
					 sleep(10);
					 }
				 }
				else
				 if( strcmp("ts_ctrl",argv[1]) == 0)
				 {
				    while ((rc = read(fd, &value, sizeof(value))) > 0) {

				        printf("vlaue = %d\n",value);
				        sleep(1);
				   }
				 }
				else
				if( strcmp("ts_unable",argv[1]) == 0) 
				 {
				
					printf("ts unableing\n");
					ioctl(fd,TS_UNABLE,NULL);					
                     while(1);
                     return  0;
				 }
				else
				if( strcmp("ts_enable",argv[1]) == 0) 
				 {
			
					printf("ts enableing\n");
					 ioctl(fd,TS_ENABLE,NULL);			
				 }
						 close(fd);
				}
         else
         printf("fd =%d\n",fd);
  
	return 0;
}



