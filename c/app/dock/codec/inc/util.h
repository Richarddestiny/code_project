#ifndef UTIL_H_
#define UTIL_H_

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <time.h>

#ifndef _NV_BUFFER_H_
#include "nv_buffer.h"
#endif

#ifndef PRINT
#define PRINT(...) printf( __VA_ARGS__)
#endif

/*used to dump VIC's output*/
//#define DUMPVICYUV 1

/*used to dump Encode's output*/
//#define DUMPENCYUV 1

/*used to debug thread info*/
//#define THREADINFO 1


typedef struct thread_info
{
    pthread_t thread;
    void *(*start_routine)(void*);
    void *arg;
    sem_t sema;
    int  priority; /*1 - 99, this value only take effect when policy is SCHED_FIFO/SCHED_RR*/
    int  policy; /*SCHED_OTHER, SCHED_FIFO, SCHED_RR*/
    unsigned is_running;
} thread_info;

/*common api for thread manager*/
void create_thread(thread_info *threadInfo);

void destroy_thread(thread_info *threadInfo);

/*commaon api for dump yuv, which mainly be used in vic and cuda, encoder process*/
void dumpyuv(nv_buffer_t * dst_buf, FILE* fout);

#endif