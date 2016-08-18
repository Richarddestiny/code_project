#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static pthread_rwlock_t rwlock;

int global = 10;

void *thread_1(void *arg)
{
	pthread_rwlock_wrlock(&rwlock);
	
	global += 20;
	printf("I am %s, now global=%d\n", (char *)arg, global);

	sleep(3);
	pthread_rwlock_unlock(&rwlock);

	pthread_exit(NULL);
}

void *thread_2(void *arg)
{
	sleep(2);
	pthread_rwlock_wrlock(&rwlock);

	global  = 60;
	printf("I am %s, now global=%d\n", (char *)arg, global);

	pthread_rwlock_unlock(&rwlock);

	pthread_exit(NULL);
}

void *thread_3(void *arg)
{
	while(1){
		pthread_rwlock_rdlock(&rwlock);
		printf("I am %s, now global=%d\n", (char *)arg, global);
		pthread_rwlock_unlock(&rwlock);
		sleep(1);
	}

	pthread_exit(NULL);
}

int main (int argc, char *argv[])
{
	pthread_t pt1,pt2,pt3;
	
	pthread_rwlock_init(&rwlock,NULL);
	
	if(pthread_create(&pt1,NULL,thread_1,"thread 1") != 0)
		perror("Create thread 1:");
	if(pthread_create(&pt2,NULL,thread_2,"thread 2") != 0)
		perror("Create thread 2:");
	if(pthread_create(&pt3,NULL,thread_3,"thread 3") != 0)
		perror("Create thread 3:");
		
	pthread_join(pt1,NULL);
	pthread_join(pt2,NULL);
	pthread_join(pt3,NULL);
	
	pthread_rwlock_destroy(&rwlock);
	
	return 0;
}
