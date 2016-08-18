#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/input.h>


#define TS_ENABLE 1
#define TS_DISABLE 0
struct key_state{
	int gpio;//pin
	int code;//pin value
	int value;//pin state 1 or 0
}event;
int main(void)
{
    int           fd = 0;
    int           rc;

/***************************/
    struct  key_state test= {56,11,0};
/*******************************/

	 if ((fd = open("/dev/gpio_keys", O_RDWR)) >= 0) {
      
         write(fd,&test,sizeof(test));
#if 1
		 printf("key fs = %d\n",fd);
            while (1) {
					rc = read(fd, &event, sizeof(event));
					 printf("type 0x%04x;value %d\n",              
                       event.code,event.value);
           }
#endif

}			
		close(fd);
   
	return 0;
}



