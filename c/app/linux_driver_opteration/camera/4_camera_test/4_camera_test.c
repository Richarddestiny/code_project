#include "v4l.h"

v4l_device vd;

void main()
{
 v4l_open(DEFAULT_DEVICE,&vd);
 v4l_mmap_init(&vd);
 v4l_grab_init(&vd,320,240);
 v4l_grab_sync(&vd);
 
 /*while(1)
 {
  vd.frame_current^=1;
  v4l_grab_frame(&vd,vd.frame_current);
  v4l_grab_sync(&vd); 
   
 }*/
 v4l_close(&vd);
}
