/*************************************************************************
	> File Name: 7_file.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: Wed 21 Oct 2015 03:33:19 AM PDT
 ************************************************************************/

#include<stdio.h>
 #include <sys/types.h>
        #include <sys/stat.h>
               #include <fcntl.h>


int main(void)
{
    int fd;
    char buf[20];
    int ret,test;
    fd = open("num", O_RDWR | O_CREAT,00777);
    if(fd < 0)
    {
        return -1;
    }

   ret  =  read(fd,buf,20);

    if(ret > 0)
    {
        test = strtol(buf,NULL,10);
        test ++;
        printf("buf:%stest:%d\n",buf,test);
        sprintf(buf,"%d",test);
    }
    else
    {
        printf("ret:%d\n",ret);
        buf[0]='0';
    }
    lseek(fd,0,SEEK_SET);
    write(fd,buf,20);
    close(fd);
    return 0;
}

