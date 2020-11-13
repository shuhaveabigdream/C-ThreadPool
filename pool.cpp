#include <iostream>
#include <vector>
#include <pthread.h>
#include "pool.h"
#include <unistd.h>
#include <time.h>

using namespace std;


static bool Stop;//全局终止标志


ThreadPool_t::ThreadPool_t(int QueSize,int ThreadCount):que_size(QueSize),thread_count(ThreadCount){
    //队列定长
    Queue=new vector<Task_t*>(que_size);
    //初始化锁
    if(pthread_mutex_init(&mutex,NULL)!=0||pthread_cond_init(&cond,NULL)!=0){
        cout<<"锁初始化失败！"<<endl;
        throw;
    };
    //初始化线程句柄
    threads=new pthread_t[thread_count];
    head=tail=count=running_threads=0;

    //开启工作线程
    for(int i=0;i<thread_count;i++){
        if(pthread_create(&threads[i],NULL,threadpool_thread,(void*)this)!=0){
            cout<<"线程创建失败，线程ID:"<<i<<endl;
            throw;
        }
        running_threads++;
    }
}

//添加任务
//添加double check
//收到终止命令后，add不再执行添加工作。
int ThreadPool_t::threadpool_add(Task_t *t){
    int status=0;

    //NULL作为终止线程任务，不允许手动添加，仅允许通过STOP
    if(t==NULL)
    return threadpool_invalid;
    
    //第一次确认
    if(count<que_size){
        if(pthread_mutex_lock(&mutex)!=0){
            return threadpool_lock_failure;
        }

        int next=(tail+1)%que_size;//确定tail的更新值
        
        do{
            //第二次确认
            if(count==que_size){
                status=threadpool_queue_full;
                break;
            }
            //开始添加任务
            (*Queue)[tail]=t;
            tail=next;
            count++;
            
            t->SetStatus(waitthread);//此时状态为等待

            //广播条件变量
            if(pthread_cond_signal(&cond) != 0) {
                status=threadpool_lock_failure;
            }
        }while(0);

        if(pthread_mutex_unlock(&mutex)!=0)status=threadpool_lock_failure;
         return status;
    }else{
        return threadpool_queue_full;
    }
}


Task_t* ThreadPool_t::getTask(){
    if(count>0){
        Task_t*obj=(*Queue)[head];
        head=(head+1)%que_size;
        count--;
        return obj;
    }
    return NULL;
}

void *ThreadPool_t::threadpool_thread(void *args){

    ThreadPool_t*pool=(ThreadPool_t*)args;
    int idx=pool->GetPid();
    while(1){
        pthread_mutex_lock(&pool->mutex);

        while(pool->count==0){
            pthread_cond_wait(&pool->cond,&pool->mutex);
        }

        //获取任务对象
        Task_t *cur= pool->getTask();

        if(cur==NULL){
            cout<<"线程结束"<<endl;
            pthread_mutex_unlock(&pool->mutex);//释放锁
            pthread_exit(NULL);
        }

        pthread_mutex_unlock(&pool->mutex);

        cur->SetStatus(running);
        cur->callback=cur->run();
        cur->SetStatus(finished);
    }
}

//停止线程池，实现方法为禁止新任务添加，并向队列中添加n个结束线程任务。
void ThreadPool_t::stop(){
    order_Stop=true;//目的在于禁止添加新的任务
    int n=running_threads;
    

    for(int i=0;i<n;i++){
        while(count==que_size){usleep(10);};
        (*Queue)[tail]=NULL;
        tail=(tail+1)%que_size;
        count++;
        pthread_cond_signal(&cond);
    }


    for(int i=0;i<n;i++){
        pthread_join(threads[i],NULL);//等待线程结束
    }

    stus=poolstop;//更新线程池状态
}

//这里需要一个stop方法
ThreadPool_t::~ThreadPool_t(){
    for(int i=0;i<thread_count;i++)pthread_join(threads[i],NULL);
    delete threads;
     delete Queue;
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}




