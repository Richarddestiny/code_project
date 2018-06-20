/*************************************************************************
    Function:������ʵ��һ��ջ
    Purpose: ����ջ��ʵ�ַ���
	         ջ��ָ�룺��ʼ������top=-1
			 ��ջ������ջ����ʱ��ջ��ָ����+1������ֵ��ջ��Ԫ��
			 ��ջ������ջ�ǿ�ʱ����ȡջ��Ԫ��ֵ���ٽ�ջ��ָ��-1
			 ջ��������top= -1;
			 ջ������: top = STACK_SIZE-1
			 ջ����top+1

    Author:  ZJQ and Others     
    Time:    2012-03-23 13:21:05
*************************************************************************/
#include <stdio.h>
#include <stdlib.h>/*exit()*/
#include "stack.h"


PUBLIC void makeEmpty (Stack *s)
{
	//top =NULL;/*ջ��ָ��ָ��NULL*/
	//s = top;
	s->next=NULL;
}


PUBLIC int isEmpty(const Stack *s)
{
	return (s->next) == NULL;
}
/*����ʵ��ջ��ȥ����isFull()
PRIVATE int isFull(void)
{
	return top == STACK_SIZE-1;
}
*/

/*��ջʱ��ջ������ջ��ָ��+1������ֵ��ջ��Ԫ��*/
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

/*��ջʱ��ջ�ǿգ���ȡջ��Ԫ��ֵ���ٽ�ջ��ָ��-1*/
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
	{/*��һ��ͼ�Ϳ��Է��������*/
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
	 д��return s->data�Ǵ����;
	 return (s->next->next)->data�����ص����ڶ���ֵ
	 MaybeӦ�ý�s������β���
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
����ջʵ�ֵĻ�����������
----------------------------------------
Summary:ע������Ĳ�������ͼ�Ϳ��Է��������
*************************************************************************/