/*************************************************************/

/******************main.c************************/

/*name :main.c

  date:2009-5-20

  author:kevin

  copyright:all is reserved

************************************/

 

#include "v4l.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#define norm VIDEO_MODE_NTSC
#define DEFAULT_FILE_NAME "picture"
int main()
{

    char *buffer=NULL;
    v4ldevice VD;
    v4ldevice *vd=&VD;

    int frame=0;
    int f_d;

    f_d=open(DEFAULT_FILE_NAME,O_RDWR|O_CREAT,0666);//获取文件的描述符
    
    if(0==v4l_open("/dev/video0",vd)) //打开设备
        printf("open success!/n");
    else 
        printf("open failure/n");
//    set (vd);
    if(0==v4l_set_norm(vd,norm))
        printf("set_norm success/n");
    else
        printf("set_norm failure/n");
    if(0==v4l_grab_init(vd,NTSC_WIDTH,NTSC_HEIGHT))//初始化设备，定义获取图像的大小
        printf("init success!/n");
    else
        printf("init failure/n");
    if(0==v4l_mmap_init(vd))//内存映射
        printf("memory map success!/n");
    else
        printf("memory map failure/n");
    if(0==v4l_grab_start(vd,frame))//开始获取图像
        printf("get picture success!/n");
    else
        printf("get picture failure/n");     

   v4l_grab_sync(vd,frame);//等待传完一帧
        
        buffer=(char *)v4l_get_address(vd);//得到这一帧的地址
        printf("img address %p/n",buffer);

    write(f_d,buffer,NTSC_WIDTH*3*NTSC_HEIGHT);//报存到文件中   

    v4l_close(vd);
    return 0;
    
}
