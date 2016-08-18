#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define TS_ENABLE 1
#define TS_DISABLE 0
int main(void)
{
    int           fd = 0;

	 if ((fd = open("/dev/gpio_keys", O_RDWR)) >= 0) {
		 printf("fd =%d\n",fd);

		 sleep(2);
			ioctl(fd,TS_DISABLE,NULL);			
			sleep(5);
			ioctl(fd,TS_ENABLE,NULL);
	}
	else
		printf("error %d\n",fd);
//		close(fd);
   
	return 0;
}



