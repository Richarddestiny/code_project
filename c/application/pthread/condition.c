#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

static int global = 0;

static pthread_mutex_t m;
static pthread_cond_t v;

void perror_exit(const char *info)
{
	perror(info);
	exit(1);
}

void *count_times(void *args)
{
	int i = 1;
	while(1){
		sleep(1);
		fprintf(stderr, "second: %d\n", i++);
	}
	return (void *)0;
}

void *tfn(void *args)
{
	/* ====================== */
	pthread_mutex_lock(&m);
	/* ====================== */

	/* NOTE: whenever pthread_cond_wait() returns, it
	** indicates that the calling thread is holding
	** the mutex. So we need to unlock the mutex.
	*/
	while(global != 100)
		pthread_cond_wait(&v, &m);

	fprintf(stderr, "t%d: global = %d\n", (int)args, global);
//	global = 10;

	/* ====================== */
	pthread_mutex_unlock(&m);
	/* ====================== */

	return (void *)0;
}

int main(int argc, char **argv)
{
	if(argc != 2){
		fprintf(stderr, "Usage: %p threads-number\n", argv[0]);
		return -1;
	}

	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&v, NULL);

	/* create a set of threads */
	pthread_t tid;
	pthread_create(&tid, NULL, count_times, NULL);

	int i;
	int thread_nums = atoi(argv[1]);
	for(i=0; i<thread_nums; i++)
		if(pthread_create(&tid, NULL, tfn, (void *)i) != 0)
			perror_exit("pthread_create falied");

	/*
	** sleep(1) makes sure that the thread-routine
	** will lock the mutex first, then condition variable
	** cause the thread to be suspended.
	**
	** sleep(3) will indicate that pthread_cond_wait()
	** won't return even if the condition varialble has been
	** changed. Thus, the condition variable is NOT associated
	** with a specified condition.
	** 
	** Actually, pthread_cond_wait() calling threads will
	** be waken up ONLY IF someone calls pthread_cond_signal()
	** or pthread_cond_broadcast() and mutex is accessable.
	** therefore, pthread_cond_wait() calling thread should
	** test critical source again after pthread_cond_wait()
	** returned, because it may be changed by anther thread
	** which access the mutex first
	*/
	sleep(1);
	pthread_mutex_lock(&m);

	global = 100;
	/*
	** pthread_cond_signal() will wake up the first thread in
	** the condition waiting-queue, while pthread_cond_broadcast()
	** wake up all threads which are waiting in the queue.
	**
	** NOTE: pthread_cond_signal() and pthread_cond_broadcast()
	** won't wake up the waiting thread immediately, it only
	** wake up one(or them) in the condition-variable waiting
	** queue.
	**
	** NOTE: POSIX doesn't require to own mutex before the calling
	** thread calls pthread_cond_signal() or pthread_cond_broadcast()
	*/
//	pthread_cond_broadcast(&v);
	pthread_cond_signal(&v);
	sleep(3);

	pthread_mutex_unlock(&m);

	pthread_exit(0);

	/*
	** exit() or return in main() will cause the whole thread group
	** terminate.
	*/
	// return 0;
}
