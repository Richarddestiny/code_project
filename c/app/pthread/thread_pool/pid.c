#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>

int main(void)
{
	pid_t result;	
	int i;

	for(i=3;i>0;i--){
		result = vfork();
	

	if(result == -1){
		printf("fork error\n");
	
	}
	if(result == 0){

		printf("the result is %d child pid:%d\n",result,getpid());
/*		if(execl("/bin/ps","ps","-ef",NULL) < 0){
			
			printf("execl error\n");
		}
*/
	}

	}
	
	if(result > 0){
		printf("the resut is %d father ppid:%d\n",result,getppid());
	}

	
	return result;
}
