#ifndef UTIL_H_
#include "util.h"
#endif

#ifdef THREADINFO
static int get_thread_priority( pthread_attr_t *attr );
#endif
static int set_thread_priority( pthread_attr_t *attr, int priority );
#ifdef THREADINFO
static int get_thread_policy( pthread_attr_t *attr );
#endif
static int set_thread_policy( pthread_attr_t *attr, int policy );

#ifdef THREADINFO
static int get_thread_priority( pthread_attr_t *attr )
{
    struct sched_param param;

    int rs = pthread_attr_getschedparam( attr, &param );
    if(rs != 0)
        PRINT("get_thread_priority failed\n");

    return param.__sched_priority;
}
#endif

static int set_thread_priority( pthread_attr_t *attr, int priority )
{
    struct sched_param param;
    int rs = pthread_attr_getschedparam( attr, &param );
    if( rs != 0 )
        PRINT("get_thread_priority failed\n");
    if( param.__sched_priority != priority )
        param.__sched_priority = priority;

    return pthread_attr_setschedparam( attr, &param );
}

#ifdef THREADINFO
static int get_thread_policy( pthread_attr_t *attr )
{
    int policy;
    int rs;
    rs = pthread_attr_getschedpolicy( attr, &policy );
    if( rs != 0 )
        PRINT("get_thread_priority failed\n");
    return policy;
}
#endif

static int set_thread_policy( pthread_attr_t *attr, int policy )
{
    return pthread_attr_setschedpolicy( attr, policy );
}

void create_thread(thread_info *threadInfo)
{
    threadInfo->is_running = 1;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    if( threadInfo->policy != SCHED_OTHER )
    {
        set_thread_policy(&attr, threadInfo->policy);
        set_thread_priority(&attr, threadInfo->priority);
#ifdef THREADINFO
        PRINT("policy input:%d, priority input:%d\n", threadInfo->policy, threadInfo->priority);
        PRINT("policy output:%d, priority output:%d\n", get_thread_policy(&attr), get_thread_priority(&attr));
#endif
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    pthread_create(
        &threadInfo->thread,
        &attr,
        threadInfo->start_routine,
        threadInfo->arg);
    pthread_attr_destroy(&attr);
}


void destroy_thread(thread_info *threadInfo)
{
    threadInfo->is_running = 0;
    sem_post(&threadInfo->sema);
    pthread_join(threadInfo->thread, NULL);
}


void dumpyuv(nv_buffer_t * dst_buf,  FILE* fout)
{
    unsigned int i, j = 0;
    unsigned char * p_buff;
    unsigned width = 0;

    for (i = 0; i < dst_buf->data->num_surf; ++i) {
        p_buff = dst_buf->data->mem[i];
        width = dst_buf->data->width[i];
        PRINT("write surf %d %dx%d\n", i, width, dst_buf->data->height[i]);
        width *= dst_buf->data->color_format[i] == NV_BUFFER_COLOR_FORMAT_U8V8 ? 2 : 1;
        for (j = 0; j < dst_buf->data->height[i]; ++j) {
            fwrite(p_buff, width, 1, fout);
            p_buff += dst_buf->data->pitch[i];
        }
    }
}