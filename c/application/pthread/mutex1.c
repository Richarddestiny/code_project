#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MUTEX
int value1, value2;
pthread_mutex_t mutex;
void *function(void *arg);

int main(void)
{
	pthread_t a_thread;
	unsigned int count = 0;
	pthread_mutex_init(&mutex, NULL);

	if(pthread_create(&a_thread, NULL, function, (void *)&count) < 0)
		exit(1);

	while(1){
		count++;
#ifdef MUTEX
		pthread_mutex_lock(&mutex);
#endif
		value1 = count;
		value2 = count;
#ifdef MUTEX
		pthread_mutex_unlock(&mutex);
#endif
	}
	return 0;
}

void *function(void *arg)
{
	while(1){
#ifdef MUTEX
		pthread_mutex_lock(&mutex);
#endif
		if(value1 != value2)
			printf("count=%d, value1=%d, value2=%d\n",\
				*(int *)arg, value1, value2);
#ifdef MUTEX
		pthread_mutex_unlock(&mutex);
#endif
	} //while(1)
	return NULL;
}
