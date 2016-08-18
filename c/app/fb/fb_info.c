/*************************************************************************
	> File Name: fb_info.c
	> Author: richard
	> Mail: freedom_wings@foxmail.com 
	> Created Time: 2013年12月19日 星期四 18时26分22秒
 ************************************************************************/



#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>

int main () {
 int fp=0;
 struct fb_var_screeninfo vinfo;
 struct fb_fix_screeninfo finfo;
 fp = open ("/dev/fb0",O_RDWR);

 if (fp < 0){
  printf("Error : Can not open framebuffer device/n");
  exit(1);
 }

 if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo)){
  printf("Error reading fixed information/n");
  exit(2);
 }
 
 if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo)){
  printf("Error reading variable information/n");
  exit(3);
 }

 printf("The mem is :%d\n",finfo.smem_len);
 printf("The line_length is :%d\n",finfo.line_length);
 printf("The xres is :%d\n",vinfo.xres);
 printf("The yres is :%d\n",vinfo.yres);
 printf("bits_per_pixel is :%d\n",vinfo.bits_per_pixel);
 close (fp);
}
