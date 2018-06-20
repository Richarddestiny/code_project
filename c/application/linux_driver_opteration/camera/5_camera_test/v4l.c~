/***********************v4l.c****************************/

/*name :v4l.c

  date:2009-5-20

  author:kevin

  copyright:all is reserved

************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <error.h>
#include <fcntl.h>
#include <sys/types.h>
#include <linux/videodev.h>
#include "v4l.h"
#define DEFAULT_DEVICE "/dev/video0"
int v4l_open(char *dev,v4ldevice *vd)
{
    if (!dev) 
              dev = DEFAULT_DEVICE; 

    if((vd->fd=open(dev,O_RDWR,10705))<0)
    {
        return -1;
    };
    if(v4l_get_capability(vd)<0)
        return -1;
    if(v4l_get_picture(vd)<0)
        return -1;
    return 0;
}

int v4l_get_capability(v4ldevice *vd)
{
    if(ioctl(vd->fd,VIDIOCGCAP,&(vd->capability))<0)
    {    
        perror("v4l_get_capability:");
        return -1;
    }

   return 0;

}

int v4l_get_picture(v4ldevice *vd)
{
    if(ioctl(vd->fd,VIDIOCGPICT,&(vd->picture))<0)
    {
        perror("v4l_get_picture");
        return -1;        
    }

   return 0;

}

int v4l_set_norm(v4ldevice *vd, int norm) 
{
   int i;

   for (i = 0; i < vd->capability.channels; i++) {
      vd->channel[i].norm = norm;

  return 0;

}

int v4l_grab_init(v4ldevice *vd,int width,int height)
{
    vd->mmap.width=width;
    vd->mmap.height=height;
    vd->mmap.format=vd->picture.palette;
    vd->frame=0;
    vd->framestat[0]=0;
    vd->framestat[1]=0;    
    return 0;
}

int v4l_mmap_init(v4ldevice *vd)
{
    if(v4l_get_mbuf(vd)<0)
        return -1;
    if((vd->map=mmap(0,vd->mbuf.size,PROT_READ|PROT_WRITE,MAP_SHARED,vd->fd,0))<0)
    {
        return -1;
    }
    return 0;
}

int v4l_get_mbuf(v4ldevice *vd)//查询实际可用的缓存数
{
    if(ioctl(vd->fd,VIDIOCGMBUF,&(vd->mbuf))<0)
    {
        perror("v4l_get_mbuf:");    
        return -1;
    }
    printf("size=%d/n",vd->mbuf.size);
return 0;
}

int v4l_grab_start(v4ldevice *vd,int frame)
{
    vd->mmap.frame=frame;
    if(ioctl(vd->fd,VIDIOCMCAPTURE,&(vd->mmap))<0)//////////////////////
    {
        exit(-1);
        return -1;
    }
    vd->framestat[frame]=1;
    return 0;
}
int v4l_grab_sync(v4ldevice *vd,int frame)
{
    if(ioctl(vd->fd,VIDIOCSYNC,&frame)<0)
    {
        return -1;    
    }
    vd->framestat[frame]=0;
    return 0;
}

unsigned char * v4l_get_address(v4ldevice *vd)
{
    return (vd->map+vd->mbuf.offsets[vd->frame]);
} 
int v4l_close(v4ldevice *vd)
{
    close(vd->fd);
    return 0;
}
