#include <stdio.h> 
#include <stdlib.h> 
#include <stdbool.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <pthread.h> 
#include <assert.h> 

#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <errno.h>
#include <string.h>
#include <strings.h>

#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <error.h>
#include <time.h>
#include <dirent.h>

#define MAXSIZE 1024*32

static int semid;

/*
** A pool contains some following structure nodes 
** which are linked together as a link-list, and
** every node includes a thread handler for the
** task, and its argument if it has any.
*/
typedef struct worker 
{ 
	void *(*process)(void *arg); 
	void *arg;	// arguments for a task 

	struct worker *next; 

}CThread_worker; 

// the thread pool
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


void sem_zero(int semid)
{
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = 0;
	sb.sem_flg = 0;

	semop(semid, &sb, 1);
}

void sem_p(int semid)
{
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = 0;

	semop(semid, &sb, 1);
}

void sem_v(int semid)
{
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = 0;

	semop(semid, &sb, 1);
}



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
int pool_add_worker(void *(*process)(void *), void *arg) 
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

void usage(const char *info)
{
	printf("Usage: %s <src> <dst>\n", info);
	exit(1);
}

void err_sys(const char *info)
{
	perror(info);
	exit(1);
}

int Stat(const char *path, struct stat *buf)
{
	int ret;
	ret = stat(path, buf);

	if(ret < 0)
		err_sys("stat error");

	return ret;
}


DIR *Opendir(const char *name)
{
	DIR *dp;
	dp = opendir(name);

	if(dp == NULL)
		err_sys("opendir error");

	return dp;
}


int Closedir(DIR *dirp)
{
	int ret;
	ret = closedir(dirp);

	if(ret < 0)
		err_sys("closedir error");

	return ret;
}


int copyfile(int from_fd, int to_fd)
{
	char buf[MAXSIZE];
	char *bp;

	int nread, nwrite;
	int totalbytes = 0;

	while(1){
		bzero(buf, MAXSIZE);

		while((nread=read(from_fd, buf, MAXSIZE)) == -1
				&& errno == EINTR);

		if(nread == -1 && errno != EINTR){
			perror("read error");
			exit(1);
		}
		else if(nread == 0){ // hits EOF
			printf("file copy completed.\n");
			break;
		}

		bp = buf;
		while(nread > 0){
			while((nwrite=write(to_fd, bp, nread)) == -1
				&& errno == EINTR);

			if(nwrite <= 0) // real error
				break;

			nread -= nwrite;
			bp += nwrite;
			totalbytes += nwrite;
		}
	}
	return totalbytes;

}

void *copyfilepass(void *arg)
{
	int from_fd, to_fd;


	from_fd = *((int *)arg);
	to_fd = *((int *)arg + 1);

	printf("thread id=%#x\n\n", pthread_self());
	printf("from_fd: %d\n", from_fd);
	printf("to_fd: %d\n", to_fd);

	*((int *)arg + 2) = copyfile(from_fd, to_fd);
	close(to_fd);
	sem_p(semid);

	pthread_exit(arg + 2);
}


void copydir(const char *src, const char *dst)
{
	DIR *dp;
	struct dirent *ep;

	char abs_src[MAXSIZE];
	char abs_dst[MAXSIZE];

	strncpy(abs_src, get_current_dir_name(), MAXSIZE);
	strncat(abs_src, "/", MAXSIZE);
	strncat(abs_src, src, MAXSIZE);

	strncpy(abs_dst, get_current_dir_name(), MAXSIZE);
	strncat(abs_dst, "/", MAXSIZE);
	strncat(abs_dst, dst, MAXSIZE);


	dp = Opendir(src);
	mkdir(dst, 0755);

	struct stat s;
	int targs[MAXSIZE], curi = 0;


	while((ep=readdir(dp)) != NULL){
		
		chdir(abs_src);
		if(ep->d_name[0] == '.')
			continue;

		lstat(ep->d_name, &s);
		if((s.st_mode & S_IFMT) != S_IFREG){
			printf("this program can copy regular files only.\n");
			exit(0);
		}

		targs[curi] = open(ep->d_name, O_RDONLY);
		chdir(abs_dst);
		targs[curi+1] = open(ep->d_name, O_WRONLY|O_CREAT|O_TRUNC, 0644);

		if(targs[0] == -1 || targs[1] == -1){
			perror("open error");
			exit(1);
		}
				
		sem_v(semid);
		pool_add_worker(copyfilepass, targs + curi);
		curi += 2;

	}
	Closedir(dp);

	// destroy the pool
	sem_zero(semid);
	pool_destroy(); 

	//printf("total copy files: %d\n", totalbytes);
}

int main(int argc, char **argv) 
{ 
	if(argc != 3)
		usage(argv[0]);

	umask(0);
	struct stat s;
	semid = semget(ftok(".", 'a'), 1, IPC_CREAT|0666);
	union semun
	{
		int val;
		struct semid_ds *buf;
		unsigned short *array;
		struct seminfo *__buf;	
	}a;
	a.val = 0;
	semctl(semid, 0, SETVAL, a);

	Stat(argv[1], &s);

 	// 3 active threads 
	pool_init(5);


	/************
	  Directory
	*************/
	if((s.st_mode & S_IFMT) == S_IFDIR)

		copydir(argv[1], argv[2]);

	/****************
	   Regular File
	*****************/
	else if((s.st_mode & S_IFMT) == S_IFREG){

		int targs[3];
		targs[0] = open(argv[1], O_RDONLY);
		targs[1] = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);

		if(targs[0] == -1 || targs[1] == -1){
			perror("open error");
			exit(1);
		}
	
		sem_v(semid);
		pool_add_worker(copyfilepass, targs);

		// destroy the pool
		sem_zero(semid);
		pool_destroy(); 
	}

	else
		exit(1);

	//pthread_exit(NULL);
	exit(0);
} 
