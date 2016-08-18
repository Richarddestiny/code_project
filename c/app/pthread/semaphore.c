#include <stdio.h>
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

sem_t sem;

void *tfn(void *args)
{
    while(1)
    {
	fprintf(stderr, "waitting for sem...\n");
	/***** P operation *****/
	sem_wait(&sem);
	fprintf(stderr, "get it!\n");

	int len = strlen((char *)args) - 1;
    printf("you have enter %d characters:%s\n", len,(char *)args);
}
	return 0;
}

int main(void)
{
	pthread_t tid;
	sleep(1);
	static char buf[64];

	void *ret;

	/***** initialize your semaphore before using it *****/
	sem_init(&sem, 0, 0);

	printf("input 'quit' to exit\n");
		pthread_create(&tid, NULL, tfn, (char *)buf);
	do{
		/***** create a thread *****/


		fgets(buf, 60, stdin);

		/***** V operation *****/
		sem_post(&sem);

	}while(strncmp(buf, "quit", 4) != 0);
		/***** Waiting for the thread *****/
		pthread_join(tid, &ret);

		printf("ret: %d\n", *((int *)&ret));


	return 0;
}
