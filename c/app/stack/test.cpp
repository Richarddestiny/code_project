/*************************************************************************
    Function:栈测试
    Purpose: 用数组实现的栈进行进制转换
    Author:  ZJQ and Other     
    Time:    2012-03-23 16:39:00
*************************************************************************/

#include<stdio.h>
#include "stack.h"

void hexConversion(int num, int hex, Stack *s);

int main()
{
	int num;//十进制数
	int hex;//进制
	Stack s1;
	makeEmpty(&s1);/*初始化栈*/
	printf("此程序的功能是将一个十进制数转换为常用的x进制数\n");
	printf("请输入一个十进制数: ");
	/*测试栈功能
	scanf("%d",&num);
	while(num!=1)
	{
		push(num);
		printf("请输入一个十进制数: ");
		scanf("%d",&num);
	}
	while (!isEmpty())
	{
		printf("%d",pop());
	}
	*/
	scanf("%d",&num);
	printf("请输入要转换的进制x: ");
	scanf("%d",&hex);
	hexConversion(num,hex,&s1);
	printf("The top num is %d",getTop(&s1));
	printf("\n");
	clearStack(&s1);
	while(!isEmpty(&s1))
	{
		printf("%d",pop(&s1));
	}
	printf("\n");

	return 0;
}


void hexConversion(int num, int hex,Stack *s)
{
	while(num != 0)
	{
		push(s,num%hex);
		num = num/hex;
	}
}

/*************************************************************************
Operation results in VS2010:
----------------------------------------

----------------------------------------
Summary:
0:出现错误： "fatal error LNK1120: 1 个无法解析的外部命令"
  解决方法：原来是将test.cpp写错为test.h,后来重建工程才解决
1：较基础版而言，在stack.h 添加了宏PRIVATE和PUBLIC，增强了程序的可读性
还增加了getTop()
2：较0:1而言，定义了栈类型，但遗憾的是，这种Stack不是抽象数据类型，因为stack.h
暴露了Stack类型的具体实现方式，因此无法阻止将Stack变量作为结直接使用:
Stack s1;
s1.top = 0;
3:较0：1：2而言增加了clearStack(),对栈的结构更加了解了。
*************************************************************************/