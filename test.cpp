#include <iostream>
#include <time.h>
#include <unistd.h>
#include "pool.h"

class MyTask:public Task_t{
public:
    int ans=0;
    MyTask(void*t):Task_t(t){}
    void* run(){
        int *id=(int*)args; 
        printf("I am working Id:%d\n",id[0]);
        
        ans=id[1]*id[2];
        return (void*)&ans;
    }
    ~MyTask(){
        
    }
};

int main(){
    srand(time(NULL));
    ThreadPool_t *t=new ThreadPool_t(10,4);
    int x[1000][3];

    for(int i=0;i<1000;i++){
        x[i][0]=i+1;
        x[i][1]=rand()%20;
        x[i][2]=rand()%20;
    }
    MyTask *tasks[100];

    for(int i=0;i<100;i++){
        tasks[i]=new MyTask((void*)(x+i));
        while(t->threadpool_add(tasks[i])!=0);
    }

    //审查返回值
    for(int i=0;i<100;i++){
        while(tasks[i]->Status()!=finished);
        int *params=(int*)tasks[i]->args;
        printf("task %d:params: %2d*%2d=%d\n",i,params[1],params[2],*(int*)tasks[i]->callback);
        delete tasks[i];
    }

    sleep(1);
    t->stop();
    delete t;
    return -1;
}