#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 
#include <assert.h> 

/*
** A pool contains some following structure nodes 
** which are linked together as a link-list, and
** every node includes a thread handler for the
** task, and its argument if it has any.
*/
typedef struct worker 
{ 
	void *(*process) (void *arg); 
	void *arg;	// arguments for a task 

	struct worker *next; 

}CThread_worker; 

/*
** the thread pool
*/
typedef struct 
{ 
	pthread_mutex_t queue_lock; 
	pthread_cond_t queue_ready; 

	// all waiting tasks
	CThread_worker *queue_head; 

	/*
	** this field indicates the thread-pool's
	** state, if the pool has been shut down
	** the field will be set to true, and it is
	** false by default.
	*/
	bool shutdown; 

	// containning all threads's tid
	pthread_t *threadid; 

	// max active taks
	int max_thread_num; 

	// current waitting tasks
	int cur_queue_size; 

}CThread_pool; 

int pool_add_worker (void *(*process) (void *arg), void *arg); 
void *thread_routine (void *arg); 


static CThread_pool *pool = NULL; 

void pool_init (int max_thread_num) 
{ 
	pool = (CThread_pool *) malloc (sizeof (CThread_pool)); 

	pthread_mutex_init (&(pool->queue_lock), NULL); 
	pthread_cond_init (&(pool->queue_ready), NULL); 

	pool->queue_head = NULL; 

	pool->max_thread_num = max_thread_num; 
	pool->cur_queue_size = 0; 

	pool->shutdown = false; 

	pool->threadid = 
		(pthread_t *)malloc(max_thread_num * sizeof(pthread_t));

	/*
	** create max_thread_num threads
	*/
	int i;
	for(i=0; i<max_thread_num; i++){

		pthread_create (&(pool->threadid[i]), NULL,
			thread_routine ,NULL); 
		printf("thread %d created! id=%#x\n\n", i,
			(int)pool->threadid[i]);
	} 
} 

// add task into the pool!
int pool_add_worker (void *(*process) (void *arg), void *arg) 
{ 
	/*
	** instruct a new worker structure
	** 
	** fill the field by parameters pass by and
	** set the next to NULL, then add this work
	** node to the thread-pool
	*/
	CThread_worker *newworker = 
		(CThread_worker *) malloc (sizeof (CThread_worker)); 
	newworker->process = process;
	newworker->arg = arg;
	newworker->next = NULL;

	/*
	** the queue is a critical source, thus whenever
	** operates the queue, it should be protected by
	** a mutex or a semaphore.
	*/
	// ====================================== //
	pthread_mutex_lock (&(pool->queue_lock)); 
	// ====================================== //

	/*
	** find the last worker is the pool and then add this
	** one to its tail
	**
	** NOTE: since the worker list which pointed by queue_head
	** has no head-node(which means queue_head could be NULL
	** at first), we should deal with the empty queue carefully.
	*/
	CThread_worker *member = pool->queue_head;
	if(member == NULL)
		pool->queue_head = newworker;
	else{
		while (member->next != NULL)
			member = member->next;
		member->next = newworker; 
	}

	pool->cur_queue_size++; // waiting tasks increase
	printf("cur_queue_size=%d\n", pool->cur_queue_size);

	// ====================================== //
	pthread_mutex_unlock (&(pool->queue_lock)); 
	// ====================================== //

	// wake up waiting task
	pthread_cond_signal(&(pool->queue_ready));

	sleep(3);
	return 0; 
} 

void *thread_routine(void *arg)
{ 
	while(1){ 
		
		// ====================================== //
		pthread_mutex_lock (&(pool->queue_lock)); 
		// ====================================== //

		/*
		** routine will waiting for a task to run, and the
		** condition is cur_queue_size == 0 and the pool
		** has NOT been shutdowned.
		*/
	        while(pool->cur_queue_size == 0 && !pool->shutdown)
		    pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));

		/*
		** the pool has been shutdowned.
		** unlock before any break, contiune or return
		*/
		if (pool->shutdown == 1){
		    pthread_mutex_unlock (&(pool->queue_lock)); 
		    pthread_exit (NULL); 
		} 

		/*
		** consume the first work in the work link-list
		*/
		pool->cur_queue_size--; 
		CThread_worker *worker = pool->queue_head; 
		pool->queue_head = worker->next; 

		// ====================================== //
		pthread_mutex_unlock (&(pool->queue_lock)); 
		// ====================================== //


		/*
		** Okay, everything is ready, now excutes the process
		** from the worker, with its argument.
		*/
		(*(worker->process))(worker->arg); 

		/*
		** when the work is done, free the source and continue
		** to excute another work.
		*/
		free(worker); 
		worker = NULL; 
	} 
	// should never be exectuted
	pthread_exit(NULL); 
} 

void *myprocess(void *arg) 
{ 
	printf ("running in myprocess, threadid: %#x,"
		"working on task %d\n\n", (int)pthread_self(), *(int *)arg); 

	// sleep 1 sec, make the task lasting longer
	sleep(1);

	return NULL; 
} 

/*
** tasks waiting in the queue will be discarded
** but will wait for the tasks which are still
** running in the pool
*/
int pool_destroy(void) 
{ 
	// make sure it wont be destroy twice
	if(pool->shutdown) 
	   return -1;

	pool->shutdown = true;  // set the flag

	// wake up all of the tasks
	pthread_cond_broadcast(&(pool->queue_ready)); 

	// wait for all of the task exit
	int i; 
	for(i=0; i < pool->max_thread_num; i++) 
		pthread_join(pool->threadid[i], NULL); 

	free (pool->threadid); 

	// destroy the queue
	CThread_worker *head = NULL; 
	while (pool->queue_head != NULL) 
	{ 
		head = pool->queue_head; 
		pool->queue_head = pool->queue_head->next; 
		free (head); 
	} 

	// destroy the mutex and cond
	pthread_mutex_destroy(&(pool->queue_lock)); 
	pthread_cond_destroy(&(pool->queue_ready)); 
	
	free(pool); 
	pool=NULL; 
	return 0; 
} 

int main (int argc, char **argv) 
{ 
	pool_init(3); // 3 threads active most 

	int *workingnum = (int *)malloc(sizeof(int) * 5), i;

	// throw 5 tasks into the pool
	for(i = 0; i < 5; i++){
		workingnum[i] = i+70; 
		pool_add_worker(myprocess, &workingnum[i]); 
	}

	// waiting for all of the tasks
	sleep(8); 
	
	// destroy the pool
	pool_destroy(); 

	free(workingnum); 

	return 0; 
} 
