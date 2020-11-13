#ifndef __POOL__H
#define __POOL__H
#include<pthread.h>
#include <vector>
#include <atomic>
typedef enum {
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

typedef enum{
    created,
    waitthread,
    running,
    finished,
    error,
    poolstop,
}Task_stus;


class Task_t{
public:
    Task_t(void*t):args(t){}
    virtual void* run()=0;//纯虚函数，继承写入任务
    const void *args;//参数
    void* callback;//返回值
    Task_stus Status(){return stus;}
    void SetStatus(Task_stus val){stus=val;}
private:
    Task_stus stus=created;
};

class ThreadPool_t{
public:
    ThreadPool_t(int,int);
    int threadpool_add(Task_t *t);
    ~ThreadPool_t();
    Task_t*getTask();
    void stop();//停止线程池，销毁线程池的前提操作
    int GetPid(){
        idx++;
        return (int)idx;
    }
    bool StopOrder(){return order_Stop;}
    Task_stus Status(){return stus;}

private:

    int thread_count;
    int que_size;
    int head;//还是先采用循环队列的策略
    int tail;
    int count;//当前任务数
    int running_threads;
    std::atomic<int>idx;

    std::vector<Task_t*>*Queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_t *threads;

    bool order_Stop;
    Task_stus stus;
    
    static void *threadpool_thread(void *);
};

#endif