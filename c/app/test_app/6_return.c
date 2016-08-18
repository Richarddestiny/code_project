/*************************************************************************
	> File Name: 6_return.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: Tue 20 Oct 2015 03:04:13 AM PDT
 ************************************************************************/

#include<stdio.h>


int test(void)
{
    int test = 3;
    return test;
}

int main(void)
{

    int test_1 = test();
    printf("test:%d\n",test_1);
    return 0;
}
