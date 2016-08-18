/*************************************************************************
	> File Name: getopt.c
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: 2016年06月22日 星期三 00时23分27秒
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
static char* mainopts = "ABCDEF";
int main(int argc,char *argv[])
{
    int opt;
   while((opt = getopt(argc,argv,mainopts)) != -1)
    {

        printf("opt:%d,optind:%d,opterr:%d,optopt:%d\n",opt,optind,opterr,opterr);
    }
    return 0;
}
