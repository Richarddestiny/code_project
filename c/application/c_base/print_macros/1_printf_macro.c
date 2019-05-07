/*************************************************************************
	> File Name: 1_printf_macro.c
	> Author: 
	> Mail: 
	> Created Time: Sun 31 Mar 2019 10:48:52 AM CST
 ************************************************************************/

#include<stdio.h>
#include <stdlib.h>


#define PRINTF_TEST(fmt,...) \
        { \
            printf("TEST %s "fmt,__func__, ##__VA_ARGS__);  \
        }



int main (int argc, char *argv[])
{
    PRINTF_TEST("this is test\n");


    return 0;
}
