/*************************************************************************
	> File Name: 9_vpusync.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: Wed 04 Nov 2015 12:45:43 AM PST
 ************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/time.h>
#include<pthread.h>

static int run = 1;
static int read_pthread(int *p_fd)
{
    int test = 1;
    long sec,usec;
    long time_sub = 0,time_temp;
    char buf[1];
    struct timeval time_start,time_stop;
    while(run)
    {
        gettimeofday(&time_start,NULL);
//        printf("time start:%ld\n",time_start.tv_sec*1000000 + time_start.tv_usec);
        read(*p_fd,buf,sizeof(buf));
        gettimeofday(&time_stop,NULL);
  //      printf("time stop:%ld\n",time_stop.tv_sec*1000000 + time_stop.tv_usec);
        sec = time_stop.tv_sec - time_start.tv_sec;
        usec = time_stop.tv_usec - time_start.tv_usec;
        time_temp = 50000 - ((sec*1000000) + usec);
        time_sub = ((time_sub > time_temp) ? time_sub : time_temp);
        printf("read:time sub:%ld temp:%d\n",time_sub,time_temp);

    }
    return 0;
}

static int write_pthread(int *p_fd)
{
    int test = 1;
    long sec,usec;
    long time_sub = 0,time_temp;
    char buf[1];
    struct timeval time_start,time_stop;
    while(run)
    {
        gettimeofday(&time_start,NULL);
//        printf("time start:%ld\n",time_start.tv_sec*1000000 + time_start.tv_usec);
        write(*p_fd,buf,sizeof(buf));
        gettimeofday(&time_stop,NULL);
  //      printf("time stop:%ld\n",time_stop.tv_sec*1000000 + time_stop.tv_usec);
        sec = time_stop.tv_sec - time_start.tv_sec;
        usec = time_stop.tv_usec - time_start.tv_usec;
        time_temp = 50000 - ((sec*1000000) + usec);
        time_sub = ((time_sub > time_temp) ? time_sub : time_temp);
        printf("write:time sub:%ld temp:%d\n",time_sub,time_temp);
    }
    return 0;
}
int main(void)
{
    int fd;
    int test;
    pthread_t read_tid,write_tid;
    fd  = open("/dev/vpu_sync",O_RDWR);

    pthread_create(&read_tid,NULL,(void*)&read_pthread,(void*)&fd);
    pthread_create(&write_tid,NULL,(void*)&write_pthread,(void*)&fd);
    test = 20;
    sleep(1);
    ioctl(fd, 0,&test);
    ioctl(fd, 1,&test);
    getchar();
    run = 0;
    pthread_join(read_tid,NULL);
    pthread_join(write_tid,NULL);

    return 0;
}
