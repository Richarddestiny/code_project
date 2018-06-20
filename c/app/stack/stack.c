/*************************************************************************
    Function:用数组实现一个栈
    Purpose: 掌握栈的实现方法
	         栈顶指针：初始化设置top=-1
			 进栈操作：栈不满时，栈顶指针先+1，再送值到栈顶元素
			 出栈操作：栈非空时，先取栈顶元素值，再将栈顶指针-1
			 栈空条件：top= -1;
			 栈满条件: top = STACK_SIZE-1
			 栈长：top+1

    Author:  ZJQ and Others     
    Time:    2012-03-23 13:21:05
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>/*exit()*/
#include "stack.h"


PUBLIC void makeEmpty (Stack *s)
{
	//top =NULL;/*栈顶指针指向NULL*/
	//s = top;
	s->next=NULL;
}


PUBLIC int isEmpty(const Stack *s)
{
	return (s->next) == NULL;
}
/*链表实现栈中去掉了isFull()
PRIVATE int isFull(void)
{
	return top == STACK_SIZE-1;
}
*/

/*进栈时，栈不满，栈顶指针+1，再送值到栈顶元素*/
PUBLIC void push(Stack *s, int i)
{
	Stack *newNode;
	newNode =(Stack*) malloc(sizeof(Stack));

	if(newNode == NULL)
	{
		printf("Error in push: The stack is full!\n");
		exit(EXIT_FAILURE);
	}

	newNode->data = i;
	newNode->next = (s->next);
	(s->next) = newNode;

}

/*出栈时，栈非空，先取栈顶元素值，再将栈顶指针-1*/
PUBLIC int pop(Stack *s)
{
	Stack *oldTop;
	int i;
	if(isEmpty(s))
	{
		printf("Error in pop: The stack if empty!\n");
		/*exit(0);*/
		exit(EXIT_FAILURE);
	}
	else
	{/*画一个图就可以分析清楚了*/
		oldTop = (s->next);
		i = (s->next) ->data;
		(s->next) = (s->next)->next;
		free(oldTop);
		return i;
	}
}

PUBLIC int getTop( Stack *s)
{
	if (isEmpty(s))
	{
		printf("Error in getTop:The stack if empty!\n");
		exit(EXIT_FAILURE);
	}
	else
	{/*
	 写成return s->data是错误的;
	 return (s->next->next)->data将返回倒数第二个值
	 Maybe应该将s看作是尾结点
	 */
		return (s->next)->data;
	}
}
PUBLIC void clearStack(Stack *s)
{
	Stack *tempNode;
		tempNode = (s->next);
	if(isEmpty(s))
	{
		printf("Error in pop: The stack if empty!\n");
		/*exit(0);*/
		exit(EXIT_FAILURE);
	}
	
		while(tempNode!=NULL)
		{
			(s->next)=(s->next)->next;
			free(tempNode);
			tempNode = (s->next);
		}

		return ;
}
/*************************************************************************
Operation results in VS2010:
----------------------------------------
链表栈实现的基本功能正常
----------------------------------------
Summary:注意链表的操作，画图就可以分析清楚了
*************************************************************************/