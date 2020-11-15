#include <iostream>
#include <time.h>
#include <ctime>
#include <unistd.h>
#include "pool.h"

    class MyTask:public Task_t{
    public:
        int *ans=new int;
        MyTask(void*t):Task_t(t){}
        void* run(){
            int *id=(int*)args; 
            *ans=id[1]*id[2];
            return (void*)ans;
        }
        ~MyTask(){
            delete ans;
        }
    };

clock_t start,end;

int main(){
    srand(time(NULL));
    ThreadPool_t *t=new ThreadPool_t(10000,4);
    int x[100000][3];

    for(int i=0;i<100000;i++){
        x[i][0]=i+1;
        x[i][1]=rand()%2000;
        x[i][2]=rand()%2000;
    }
    MyTask *tasks[100000];
    start=clock();
    for(int i=0;i<100000;i++){
        tasks[i]=new MyTask((void*)(x+i));
        while(t->threadpool_add(tasks[i])!=0);
    }
    end=clock();
    
    //审查返回值
     for(int i=0;i<100000;i++){
         while(tasks[i]->Status()!=finished);
         int *params=(int*)tasks[i]->args;
         printf("task %d:params: %2d*%2d=%d\n",i,params[1],params[2],*(int*)tasks[i]->callback);
         delete tasks[i];
     }

    t->stop();
    double endtime=(double)(end-start)/CLOCKS_PER_SEC;

    std::cout<<"total time conusmed:"<<endtime<<"s"<<std::endl;
    delete t;
    return -1;
}