/*************************************************************************
> File Name: 5_datetime.c
> Author: richard 
> Mail: freedom_wings@foxmail.com
> Created Time: Mon 12 Oct 2015 12:08:21 AM PDT
************************************************************************/

#include<stdio.h>
#include<time.h>
int main(void){
    char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep); /*取得当地时间*/
    printf ("%d%d%d ", (1900+p->tm_year),( 1+p->tm_mon ), p->tm_mday);
    printf("%s%d:%d:%d\n", wday[p->tm_wday],p->tm_hour, p

           ->tm_min, p->tm_sec);
    return 0;
}
