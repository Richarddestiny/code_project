#ifndef __IPC__H__
#define __IPC__H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <sys/sem.h>

typedef struct flock lock;

#define BUFFER_SIZE 1024

#define Error(info)                                     \
do{                                                     \
	fprintf(stderr,"%s:%s\n",info,strerror(errno)); \
	exit(EXIT_FAILURE);                             \
}while(0)

typedef struct msgbuf 
{
	long mtype;     /* message type, must be > 0 */
	char ptext[1024];  /* message data */
}msgbuf;

#endif
