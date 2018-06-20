#ifndef STACK_H
#define STACK_H
/*
�ڸ�ʽ�ϣ�һЩ����Աʹ�ú���ָ����Щ�����ͱ����ǡ����еġ����������Ա�������������ַ��ʣ�
��Щ��˽�еģ������޸��ļ��ڷ��ʣ�
*/
#define PUBLIC /*empty*/
#define PRIVATE static
typedef struct node{
	int data;
	struct node *next;
} Stack;

PRIVATE struct node *top = NULL;/*ջ��ָ��,Ҳ�������Ϊͷ���*/

PUBLIC void makeEmpty(Stack *s);
PUBLIC int isEmpty(const Stack *s);/*��ָ��ȴ�ֵ����Լʱ��*/
PUBLIC void push(Stack *s, int i);
PUBLIC int pop(Stack *s);
PUBLIC int getTop( Stack *s);
PUBLIC void clearStack(Stack *s);
/*int isFull(void);*/
/*isFull(void)ԭ�Ͳ�û�з���ͷ�ļ�stack.h�У�isFull(void)��ʹ������洢ջʱ��������ģ�����ʹ���������洢ջʱ��û��������*/
#endif