#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
int main(void)
{
	int fd= 0;
    while(1)
         {
             
             printf("sleep 5 scconed\n");
            sleep(6);
             fd++;
             if(fd ==11)
             break;
        }
	fd =open("/dev/test",O_RDWR);
	return 0;

}
