#ifndef STACK_H
#define STACK_H
/*
在格式上，一些程序员使用宏来指明那些函数和变量是“公有的”，（即可以被程序的其他部分访问）
那些是私有的（即仅限该文件内访问）
*/
#define PUBLIC /*empty*/
#define PRIVATE static
typedef struct node{
	int data;
	struct node *next;
} Stack;

PRIVATE struct node *top = NULL;/*栈顶指针,也可以理解为头结点*/

PUBLIC void makeEmpty(Stack *s);
PUBLIC int isEmpty(const Stack *s);/*传指针比传值更节约时间*/
PUBLIC void push(Stack *s, int i);
PUBLIC int pop(Stack *s);
PUBLIC int getTop( Stack *s);
PUBLIC void clearStack(Stack *s);
/*int isFull(void);*/
/*isFull(void)原型并没有放在头文件stack.h中，isFull(void)在使用数组存储栈时是有意义的，但在使用链表来存储栈时就没有意义了*/
#endif