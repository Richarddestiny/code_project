/*************************************************************************
	> File Name: 1_gaodiwei.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: Mon 06 Jul 2015 05:11:39 AM PDT
 ************************************************************************/

#include<stdio.h>


int main(void)
{

    unsigned char test[2] = {0x22,0x33};

    unsigned short _test;

    _test =test[1]*256+ test[0];

    printf("_test:%x\n",_test);
          
          return 0 ;
}
