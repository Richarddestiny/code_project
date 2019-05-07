/*************************************************************************
	> File Name: time.c
	> Author: 
	> Mail: 
	> Created Time: Tue 07 May 2019 09:51:23 AM CST
 ************************************************************************/

#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[])
{
    time_t system_time;
    struct tm *time_local;
    struct tm *time_gmt;
    char time_str[1024];

    time(&system_time); //获取unix时间戳
    time_local = localtime(&system_time); //转换为当前时区时间
    printf("localtime:%d-%d-%d %d:%d:%d\n", time_local->tm_year + 1900, time_local->tm_mon + 1, time_local->tm_mday,
                        time_local->tm_hour, time_local->tm_min, time_local->tm_sec);

    time_gmt = gmtime(&system_time); //转换为当前时区时间
    printf("gmt:%d-%d-%d %d:%d:%d\n", time_gmt->tm_year + 1900, time_gmt->tm_mon + 1, time_gmt->tm_mday,
                        time_gmt->tm_hour, time_gmt->tm_min, time_gmt->tm_sec);
    
    printf("asctime: %s\n", asctime(time_local));
    printf("ctime: %s\n", ctime(&system_time));

    strftime(time_str, sizeof(time_str), "%F %T %p %B %A", time_local);

    printf("strftime:%s\n", time_str);

}
