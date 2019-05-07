/*************************************************************************
> File Name: firmware_comb.c
> Author: 
> Mail: 
> Created Time: Fri 29 Mar 2019 08:08:16 PM CST
************************************************************************/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>

#define MAX_FILENAME_LENTH (1024)

typedef struct{
    char filename[MAX_FILENAME_LENTH];
    int  filesize;
    int  address;
}type_comb_file;

void usage(char *process)
{
    printf("Usage:  %s\n",process);
    printf("Option: 1_address 1_filename   2_address 2_filename   filler  combfilename\n");
}

int main(int argc, char *argv[])
{
    int fd_1, fd_2, fd_3;
    int ret;
    type_comb_file  file_1,file_2;
    char comb_file_name[MAX_FILENAME_LENTH];
    int filler;
    char buf[1024];
    int readlenth = 0, fill_total = 0, filler_size = 0;
    char *filler_buf; 

    if(argc < 7)
    {
        usage(argv[0]);
        return -1;
    }

    strcpy(file_1.filename, argv[2]);
    strcpy(file_2.filename, argv[4]);
    strcpy(comb_file_name, argv[6]);
    sscanf(argv[1], "%x", &file_1.address);
    sscanf(argv[3], "%x", &file_2.address);
    filler = atoi(argv[5]);

    printf("(file %s address:%#x)++(file:%s address:%#x) ==> %s\n",file_1.filename,file_1.address ,
           file_2.filename, file_2.address, comb_file_name);

    fd_1 = open(file_1.filename, O_RDWR | O_CREAT);
    if(fd_1 < 0)
    {
        printf("Open %s failed!\n",file_1.filename);
        return -1;
    }

    fd_2 = open(file_2.filename, O_RDWR | O_CREAT);
    if(fd_2 < 0)
    {
        printf("Open %s failed!\n",file_2.filename);
        return -1;
    }

    fd_3 = open(comb_file_name, O_RDWR | O_CREAT);
    if(fd_3 < 0)
    {
        printf("Open %s failed!\n",comb_file_name);
        return -1;
    }
    memset(buf, filler, sizeof(buf));
    /* 1. fill fist file */    
    while(1) {
        readlenth = read(fd_1, buf, sizeof(buf));
        if(readlenth <= 0)
        {
            break;
        }
        write(fd_3, buf, readlenth);
        fill_total += readlenth;
    }
    printf("fill %s ==> %s  address:%#x total:%d\n",file_1.filename, comb_file_name, file_1.address, fill_total);
    /* 2. fill filler  */
        filler_size =  file_2.address - fill_total;
        if(filler_size < 0 ) {
            printf("Error fist file size:%#x second file address:%#x, can't coverage!\n",fill_total, file_2.address);
            return -1;
        }
        filler_buf = malloc(filler_size);
        if(filler_buf == NULL){
           printf("Malloc %d fillerbuf failed!",filler_size);
           return -1;
        }
        memset(filler_buf, 0, filler_size);
        write(fd_3, filler_buf, filler_size);

        free(filler_buf);


    /* 3. fill second file */    
    while(1) {
        readlenth = read(fd_2, buf, sizeof(buf));
        if(readlenth <= 0)
        {
            break;
        }
        write(fd_3, buf, readlenth);
        fill_total += readlenth;
    }
    printf("fill %s ==> %s  address:%#x total:%d\n",file_2.filename, comb_file_name, file_1.address, fill_total);

    return 0;



}
