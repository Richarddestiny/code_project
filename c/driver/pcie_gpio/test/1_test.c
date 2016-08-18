/*************************************************************************
> File Name: 1_test.c
> Author: Richard
> Mail: freedom_wings@foxmail.com 
> Created Time: 2014年04月28日 星期一 15时26分17秒
************************************************************************/

#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


struct test {
    int pin;
    int value;
}test_1;

int main(void)
{
    int tmp,tmp2,fd;
    fd = open("/dev/gpio_pcie",O_RDWR);
    scanf("%d %d",&tmp,&tmp2);
    test_1.pin = tmp;
    test_1.value = tmp2;

    tmp = write(fd,&test_1,sizeof(test_1));
    printf("tmp =%d\n",tmp);
    return 0;
}
