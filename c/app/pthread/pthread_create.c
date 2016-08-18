// pthread.c

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/*****************************
	thread routine
******************************/
void *tfn(void *arg)
{
	if((errno=pthread_detach(pthread_self())) != 0)
		perror("detach error");
	printf("tfn is running. Argument was \"%s\"\n",
		(char *)arg);
	sleep(2);
	strcpy((char *)arg, "Bye!");
	pthread_exit("Thank you for the CPU time");
}

/****************************
	main function
*****************************/
int main(void)
{
	pthread_t tid;
	void *thread_result;

	/****************
	   local array
	****************/
	char message[] = "Hello World";

	/***********************************************
	   create a new thread with default property
	   assign tfn as the thread's execute routine
	   and passing an argument: message
	************************************************/
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if((errno=pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0)
		perror("set detach error");

	//pthread_create(&tid, &attr, tfn, (void *)message);
	pthread_create(&tid, NULL, tfn, (void *)message);

	printf("Waiting for thread to finish...\n");
	/*******************************************
	   calling pthread_join() to wait for the
	   thread and store its return value to
	   the variable thread_result.
	********************************************/
	sleep(1);
	errno = pthread_join(tid, &thread_result);
	if(errno != 0){
		fprintf(stderr, "join falied: %s\n",
				strerror(errno));
		return 0;
	}

	printf("Thread joined, it returned %s\n",
		(char *)thread_result);
	printf("Message is now \"%s\"\n", message);

	exit(0);
}
