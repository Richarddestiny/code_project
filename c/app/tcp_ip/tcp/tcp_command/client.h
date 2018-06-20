#ifndef _CLIENT_H_
#define _CLIENT_H_
#include <semaphore.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
extern sem_t sem_stream;
int data_pthread(char *argv[]);
#endif
