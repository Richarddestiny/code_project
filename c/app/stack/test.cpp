/*************************************************************************
    Function:ջ����
    Purpose: ������ʵ�ֵ�ջ���н���ת��
    Author:  ZJQ and Other     
    Time:    2012-03-23 16:39:00
*************************************************************************/

#include<stdio.h>
#include "stack.h"

void hexConversion(int num, int hex, Stack *s);

int main()
{
	int num;//ʮ������
	int hex;//����
	Stack s1;
	makeEmpty(&s1);/*��ʼ��ջ*/
	printf("�˳���Ĺ����ǽ�һ��ʮ������ת��Ϊ���õ�x������\n");
	printf("������һ��ʮ������: ");
	/*����ջ����
	scanf("%d",&num);
	while(num!=1)
	{
		push(num);
		printf("������һ��ʮ������: ");
		scanf("%d",&num);
	}
	while (!isEmpty())
	{
		printf("%d",pop());
	}
	*/
	scanf("%d",&num);
	printf("������Ҫת���Ľ���x: ");
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
0:���ִ��� "fatal error LNK1120: 1 ���޷��������ⲿ����"
  ���������ԭ���ǽ�test.cppд��Ϊtest.h,�����ؽ����̲Ž��
1���ϻ�������ԣ���stack.h ����˺�PRIVATE��PUBLIC����ǿ�˳���Ŀɶ���
��������getTop()
2����0:1���ԣ�������ջ���ͣ����ź����ǣ�����Stack���ǳ����������ͣ���Ϊstack.h
��¶��Stack���͵ľ���ʵ�ַ�ʽ������޷���ֹ��Stack������Ϊ��ֱ��ʹ��:
Stack s1;
s1.top = 0;
3:��0��1��2����������clearStack(),��ջ�Ľṹ�����˽��ˡ�
*************************************************************************/