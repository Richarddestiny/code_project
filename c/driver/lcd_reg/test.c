#include<stdio.h>

int main(void)
{
	char buff[][15] = {"aa","bb"};

	printf("%d\t%d",sizeof(buff),sizeof(*buff));
}
