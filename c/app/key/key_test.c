#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <linux/input.h>

struct input_event event;

int main(void)
{
     char          name[64];           /* RATS: Use ok, but could be better */
    char          buf[256] = { 0, };  /* RATS: Use ok */
    int           fd = 0;
    int           i;
    int           rc;

#if 0
  for (i = 0; i < 32; i++) {
        sprintf(name, "/dev/input/event%d", i);
        if ((fd = open(name, O_RDONLY, 0)) >= 0) {         
            ioctl(fd, EVIOCGNAME(sizeof(buf)), buf);               
	if(0 == strncmp(buf,"gpio-keys",9))
	{	
		i = 0;
		break;
	}	
           close(fd);
        }
	
    }

	if(i ==32)
	{
		printf("can't find keyboard device\n");
		exit(-1);
	}
#endif 
//	 if ((fd = open("/dev/input/keyboard", O_RDWR, 0)) >= 0) {
	 if ((fd = open("/dev/gpio_keys", O_RDWR, 0)) >= 0) {
            printf("%s: open, fd = %d\n", name, fd);

     
            while ((rc = read(fd, &event, sizeof(event))) > 0) {

                printf("type 0x%04x; code 0x%04x;"
                       " value 0x%08x; %s\n",              
                       event.type, event.code, event.value,"press");
           }
	close(fd);
}
   
	return 0;
}



