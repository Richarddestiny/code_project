#include <stdio.h>
#include <stdlib.h>


int main(void)
{
    char time[] ="test19/01/09,08:58:42-01test";

    int  y,m,d,h,mi,s,z;
    sscanf(time,"test%02d/%02d/%02d,%02d:%02d:%02d%d",&y,&m,&d,&h,&mi,&s,&z);

    printf("y:%d,m:%d,d:%d,h:%d,m:%d,s:%d z:%d\n",y,m,d,h,mi,s,z);

    return 0;
}


