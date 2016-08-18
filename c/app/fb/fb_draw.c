/*************************************************************************
	> File Name: fb_draw.c
	> Author: richard
	> Mail: freedom_wings@foxmail.com 
	> Created Time: 2013年12月19日 星期四 18时32分06秒
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
 long screensize=0;
 char *fbp = 0;
 int x = 0, y = 0;
 long location = 0;
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

  screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
 /*è¿å°±æ¯æfpææçæä»¶ä¸­ä»å¼å§å°screensizeå¤§å°çåå®¹ç»æ å°åºæ¥ï¼å¾å°ä¸ä¸ªæåè¿åç©ºé´çæé*/
 fbp =(char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp,0);
   if ((int) fbp == -1)
     {
        printf ("Error: failed to map framebuffer device to memory./n");
        exit (4);
     }
/*è¿æ¯ä½ æ³ç»çç¹çä½ç½®åæ ,(0ï¼0)ç¹å¨å±å¹å·¦ä¸è§*/
  x = 100;
  y = 100;
  location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;

  *(fbp + location) = 100;  /* èè²çè²æ·± */  /*ç´æ¥èµå¼æ¥æ¹åå±å¹ä¸æç¹çé¢è²*/
  *(fbp + location + 1) = 15; /* ç»¿è²çè²æ·±*/   
  *(fbp + location + 2) = 200; /* çº¢è²çè²æ·±*/   
  *(fbp + location + 3) = 0;  /* æ¯å¦éæ*/  
  munmap (fbp, screensize); /*è§£é¤æ å°*/
  close (fp);    /*å³é­æä»¶*/
  return 0;

}
