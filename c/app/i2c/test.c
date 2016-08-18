/*************************************************************************
	> File Name: test.c
	> Author: Richard
	> Mail: freedom_wings@foxmail.com 
	> Created Time: 2014年04月21日 星期一 17时42分31秒
 ************************************************************************/

#include<stdio.h>

int main(void)
{
    char test=0x48;
    char test1;
    int test2;

    test1=test<<8;
    test2=test<<8;
    printf("test=%x\n",test);
    printf("test1=%x\n",test1);
    printf("test2=%x\n",test2);
    printf("testp=%x\n",test<<8);
    printf("testp=%h\n",test<<8);
    return 0;
}
