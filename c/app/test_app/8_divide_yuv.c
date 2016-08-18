/*************************************************************************
> File Name: 8_divide_yuv.c
> Author: richard 
> Mail: freedom_wings@foxmail.com
> Created Time: Thu 05 Nov 2015 12:48:12 AM PST
************************************************************************/

#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>

int main(int argc, char *argv[])
{
    int input_fd,output_fd;
    int ret;
    char input_file[1024];
    char output_file[1024];
    int yuv_wide = 0;
    int yuv_hight = 0;
    int divide_count,count = 0;
    char divide_dir[] = "divide";
    size_t divide_file_size;
    if(argc < 4)
    {
        printf("pls input the yuv file,and resolution and count\n");
        printf("eg:master_mipi.yuv 1280 960 100\n");
        return -1;
    }
    printf("divide yuv file:%s\t",argv[1]);
    printf("reslution:%sX%s\t",argv[2],argv[3]);
    printf("divide count:%s\n",argv[4]);

    strcpy(input_file,argv[1]);
    yuv_wide = strtol(argv[2],NULL,10);
    yuv_hight = strtol(argv[3],NULL,10);
    divide_count = strtol(argv[4],NULL,10);
    if(access(divide_dir,NULL) != 0)
    {
        ret = mkdir(divide_dir,S_IRWXU | S_IRWXG | S_IRWXO);
        if(ret < 0)
        {
            printf("mkdir failed:%s\n",strerror(errno));
            return -1;
        }
    }
    else
        printf("%s dir existed\n",divide_dir);
    input_fd = open(input_file,O_RDWR);
    if(input_fd < 0)
    {
        printf("open %s failed:%s\n",argv[1],strerror(errno));
        return -1;
    }

    ret = chdir(divide_dir);
    if(ret < 0)
    {
        printf("chdir failed:%s\n",strerror(errno));
        return -1;
    }
    divide_file_size = yuv_wide * yuv_hight * 3/2;
    {
        char buf[divide_file_size];
        while(count < divide_count)
        {
            sprintf(output_file,"%d_%s",count,input_file);
            output_fd = open(output_file,O_RDWR|O_CREAT,00777);
            if( output_fd < 0 )
            {
                printf("open output file failed:%s\n",strerror(errno));
                return -1;
            }
            ret = read(input_fd,buf,divide_file_size);
            if(ret != divide_file_size)
            {
                printf("read input_file failed:%s\n",strerror(errno));
                return -1;
            }

            ret = write(output_fd,buf,divide_file_size);
            if(ret != divide_file_size)
            {
                printf("write output file failed:%s\n",strerror(errno));
                return -1;
            }
            close(output_fd);
            count ++;
        }

    }
}
