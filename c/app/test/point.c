/*************************************************************************
	> File Name: point.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: 2016年08月22日 星期一 23时55分31秒
 ************************************************************************/

#include<stdio.h>

void test(char **p)
{
    char buff[]="11test";
    memcpy(*p,buff,sizeof(buff));
}
int main(void)
{
    char *p = (char *)malloc(2);

    test(&p);
    printf("%s\n",p);
    return 0;
}
