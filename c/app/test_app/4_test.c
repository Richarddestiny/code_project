/*************************************************************************
> File Name: 4_test.c
> Author: richard 
> Mail: freedom_wings@foxmail.com
> Created Time: Wed 16 Sep 2015 11:52:50 PM PDT
************************************************************************/

#include<stdio.h>

  #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

int main(void)
{
    char filename_mipi[20],filename_dvp[20],filename_test[20];
    int a =1, b = 2, c =3;
    int cap_fd[3];
    sprintf(filename_mipi,"master_mipi_capture%d.yuv",a);
    sprintf(filename_dvp,"master_dvp_capture%d.yuv",b);

    sprintf(filename_test,"master_test_capture%d.yuv",c);

    cap_fd[2] = open(filename_test, O_WRONLY | O_CREAT,00777);
    cap_fd[1] = open(filename_mipi, O_WRONLY | O_CREAT,00777);
    cap_fd[0] = open(filename_dvp, O_WRONLY | O_CREAT,00777);


    if(cap_fd[0] < 0 && cap_fd[1] < 0  && cap_fd[2] <  0)
    {
        printf("open picture failed\n");

    }
    close(cap_fd[1]);
    close(cap_fd[0]);
    close(cap_fd[2]);
}
