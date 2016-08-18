#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <malloc.h>

#include <string.h>
#include <sys/time.h>


#define USB_VIDEO_DEV "/dev/video0"
#define FILE_NAME "3.jpg"
#define STILL_IMAGE -1
#define VIDEO_START 0
#define VIDEO_STOP 1
#define VIDEO_PALETTE_RAW_JPEG 20
#define VIDEO_PALETTE_JPEG 21
static int debug = 1;

int get_jpegsize(unsigned char *buf, int size)
{
  int i;
  for(i = 1024; i < size; i++)
  {  
  if ( (buf[i] == 0xFF)&&(buf[i+1] == 0xD9)) return i+2;//jpeg文件格式中是以0xFF 0xD9结尾的,

  //以此判断文件大小
  }
  return -1;
}

int main(int argc, char *argv[])
{   
int usb_camera_fd = -1,framesize=0,jpegsize=0;
char *usb_video_dev = USB_VIDEO_DEV; //"/dev/ video0"
char *filename = FILE_NAME;// "/tmp/1.jpg"

FILE *fp;
struct video_capability video_caps;
struct video_channel video_chan;
struct video_picture video_pic;   
struct video_mbuf video_mbuffer;
struct video_mmap vid_mmap;
unsigned char *mapaddr=NULL,*framebuffer=NULL,*destbuffer=NULL;
usb_camera_fd = open(usb_video_dev,O_RDWR);//打开设备,可读写,也即打开"/dev/ video0"
// usb_camera_fd是设备号，open成功则返回设备号
if (usb_camera_fd == -1)
{
fprintf(stderr,"Can't open device %s",usb_video_dev);
return 1;
}   
//-----------------------------video_capability-----------------------------
if (ioctl(usb_camera_fd,VIDIOCGCAP,&video_caps) == -1)//get videodevice capability 获取设
//    备基本信息。
{
perror("Couldn't get videodevice capability");
return -1;
}

if (debug)
{
printf("video name : %s\n",video_caps.name);// struct video_capability video_caps;
printf("video_caps.channels :%d\n",video_caps.channels);
printf("video_caps.type : 0x%x\n",video_caps.type);
printf("video maxwidth : %d\n",video_caps.maxwidth);
printf("video maxheight : %d\n",video_caps.maxheight);
printf("video minwidth : %d\n",video_caps.minwidth);
printf("video minheight : %d\n",video_caps.minheight);   
}

//-----------------------------video_channel-----------------------------
// struct video_channel video_chan;
#if 0
if (ioctl(usb_camera_fd,VIDIOCGCHAN,&video_chan) == -1)//获取信号源的属性
{
perror("ioctl (VIDIOCGCAP)");
return -1;
}
if (debug)
{
printf("video channel: %d\n",video_chan.channel);
printf("video channel name: %s\n",video_chan.name);
printf("video channel type: %d\n",video_chan.type);
}
#endif
//-----------------------------video_picture-----------------------------
if (ioctl(usb_camera_fd,VIDIOCGPICT,&video_pic) == -1)//获取设备采集的图象的各种属性
{
perror("ioctl (VIDIOCGPICT)");
return -1;
}


if (debug)
{
printf("video_pic.brightness : %d\n",video_pic.brightness);
printf("video_pic.colour : %d\n",video_pic.colour);
printf("video_pic.contrast : %d\n",video_pic.contrast);
printf("video_pic.depth : %d\n",video_pic.depth);
printf("video_pic.hue : %d\n",video_pic.hue);
printf("video_pic.whiteness : %d\n",video_pic.whiteness);
printf("video_pic.palette : %d\n",video_pic.palette);
}
//-----------------------------video_mbuf-----------------------------
memset(&video_mbuffer,0,sizeof(video_mbuffer));


//初始化video_mbuf，以得到所映射的buffer的信息
if (ioctl(usb_camera_fd,VIDIOCGMBUF,&video_mbuffer) == -1)//video_mbuf
{
perror("ioctl (VIDIOCGMBUF)");
return -1;
}
if (debug)
{
printf("video_mbuffer.frames : %d\n",video_mbuffer.frames); // frames最多支持的帧数
printf("video_mbuffer.offsets[0] : %d\nvideo_mbuffer.offsets[1] :%d\n",video_mbuffer.offsets [0],video_mbuffer.offsets[1]); //每帧相对基址的偏移
printf("video_mbuffer.size : %d\n",video_mbuffer.size); //每帧大小
   
}
////等一等，对照v4l.docx看到这儿了
//将mmap与video_mbuf绑定
mapaddr=(unsigned char *)mmap(0,video_mbuffer.size,PROT_READ,MAP_SHARED,usb_camera_fd, 0);

if (mapaddr < 0)
{
perror("v4l mmap");
return -1;
}
//-----------------------------video_mmap-----------------------------
vid_mmap.width = 320;
vid_mmap.height = 240;
vid_mmap.frame = 0;//单帧采集
vid_mmap.format = VIDEO_PALETTE_JPEG;

//-----------------------------start capture-----------------------------Mmap方式下真正开始视频截取
int ret = VIDEO_START; 
//若调用成功，开始一帧的截取，是非阻塞的 是否截取完毕留给VIDIOCSYNC来判断
 ret =ioctl(usb_camera_fd,VIDIOCCAPTURE,&ret);

printf("VIDIOCCAPTURE ret =%d ag2=%d start=%d\n",ret,VIDIOCCAPTURE,VIDEO_START);
//-----------------------------wait for ready---------------------///调用VIDIOCSYNC等待一帧截取结束

//若成功，表明一帧截取已完成。可以开始做下一次 VIDIOCMCAPTURE
//frame是当前截取的帧的序号

   
framesize=320*240>>2;//实际采集到的jpeg图像的大小最多也就十几KB

//获取刚刚采集到的图像数据帧的地址

framebuffer = mapaddr + video_mbuffer.offsets[vid_mmap.frame];

//获取图像的大小
printf("framebuffer=%d framesize=%d\n",framebuffer,framesize);
jpegsize = get_jpegsize(framebuffer, framesize);
printf("jpegsize=%d\n",jpegsize);
#if 0
if (jpegsize < 0)
{
printf("Can't get size of jpeg picture\n");
return 1;
}
#endif
//分配空间,准备将destbuffer 缓冲区中的图像数据写入文件
destbuffer = (unsigned char *)malloc(video_mbuffer.size);
if ( destbuffer == NULL)
{
printf("malloc memory for destbuffer error\n");
return -1;
}
memcpy(destbuffer,framebuffer,video_mbuffer.size);

printf("this is fopen 3_tss.jpg\n");

fp = fopen("3_tsst.jpg","w");//打开文件
if (!fp)
{
printf("Can't open file %s",filename);
//return -1;
}

fflush(fp);

fwrite(destbuffer,video_mbuffer.size,1,fp);//写入文件
fclose(fp);//关闭文件

free(destbuffer);//释放空间
munmap(mapaddr,video_mbuffer.size);//取消绑定
close(usb_camera_fd);
return 1;
}
