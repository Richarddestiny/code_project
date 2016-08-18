#include <sys/types.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>

#define VPDMA_LIST_ATTR (0x4810D008)

int main(void)
{
	 char* map_base;
    int fd;
    char read_buf[10];

	    if((fd=open("/dev/mem",O_RDWR|O_SYNC))==-1)
	    {
		  perror("open /dev/mem"); 
		return(-1);
	    }
	map_base=mmap(0,0x1000,PROT_READ|PROT_WRITE,MAP_SHARED,fd,VPDMA_LIST_ATTR);
   printf("map_base:%lx\n",map_base);
   if((int)map_base ==-1)
   {
      printf("Error: failed to map framebuffer device %lx to memory. fd=%d\r\n",(unsigned long)map_base,fd);
      perror("mmap:");
      exit(0);
   }
    memcpy(read_buf,(char *)map_base,10);
    printf("read sram :%s \n",read_buf);


 	 close(fd);
  	  munmap(map_base,0x1000);//解除映射关系
	return 0;
}
