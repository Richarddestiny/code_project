#include <linux/i2c-dev.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


/*
 *./i2c_test w  写数据
 *./i2c_test r  读数据
*/
int main(int argc,char **argv)
{
	int fd;
  	int adapter_nr = 4; /* probably dynamically determined */
    char filename[20];
	int addr = 0x27;    /*I2C设备地址*/
	unsigned char addrdata = 0x02; /* Device register to access */
	char wbuf[10];
	char rbuf[10];
  
    sprintf(filename,"/dev/i2c-%d",adapter_nr);
    if ((fd = open(filename,O_RDWR)) < 0) {
       	printf("cannot open /dev/i2c-0!\n");
       	exit(1);
    }

	/*1.发送设备地址*/
	 if (ioctl(fd,I2C_SLAVE,addr) < 0) {
	 	printf("transfer is failed!\n");
	   	exit(1);
	 }
    int tmp;
    unsigned short test;
    while(1)
    {
        printf("pls input address and addrdata:");
        scanf("%d %x",&tmp,&addrdata);
//        getchar();
        printf("tmp=%d,add=%x\n",tmp,addrdata);
        switch(tmp)
        {
            case 1:

        printf("write value:");
        scanf("%d",&test);
		/*2.发寄存器地址/片内地址*/
		wbuf[0] = addrdata;
	  	wbuf[1] = test>>8;
//	  	wbuf[1] = test;
	  	wbuf[2] = test;
            printf("write value wbuf[0]=%#x,wbuf[1]=%#x\n",wbuf[1],wbuf[2]);
	  	if (write(fd,wbuf,3) != 3) {
	    	printf("write is failed!\n");
	  	}	  	
            sleep(2);
		if (write(fd,&addrdata,1) != 1) {
	    	printf("write is failed!\n");
	  	}

		/*3.2.开始读*/
		if (read(fd,rbuf,2) != 2) 
		{
	    	printf("read is failed!\n");
	  	} 
		else 
		{
	    	printf("rbuf[0]=0x%x,rbuf[1]=%#x\n",rbuf[0],rbuf[1]);
	    	printf("rbuf[0]=%#x  %d\n",((rbuf[0]<<8) + rbuf[1]),((rbuf[0]<<8)+rbuf[1]));
    }
            break;
            case 2:
		/*3.读数据*/
		/*3.1.告诉设备,我要从哪里读*/
		if (write(fd,&addrdata,1) != 1) {
	    	printf("write is failed!\n");
	  	}

		/*3.2.开始读*/
		if (read(fd,rbuf,2) != 2) 
		{
	    	printf("read is failed!\n");
	  	} 
		else 
		{
	    	printf("rbuf[0]=0x%x\n",((rbuf[0]<<8) + rbuf[1]));
    }
        break;
        }}

 	return 0;
}
