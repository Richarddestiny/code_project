/*************************************************************************
	> File Name: helloword.cpp
	> Author: richard 
	> Mail: freedom_wings@foxmail.com
	> Created Time: 2016年05月31日 星期二 04时42分00秒
 ************************************************************************/

#include<iostream>
using namespace std;

int main(int argc,char *argv[])
{
    char *str = new char[20];
    while(1)
    {
    cout << "pls input the word:" ;
    cin >>str; 
    cout << str << endl;
}
    return 0;
}
