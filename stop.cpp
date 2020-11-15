#include <iostream>
#include <time.h>
#include <unistd.h>
#include "pool.h"

using namespace std;

class MyTask:public Task_t{
public:
    int ans=0;
    MyTask(void*t):Task_t(t){}
    void* run(){
        int *id=(int*)args; 
        printf("I am working Id:%d\n",id[0]);
        usleep(rand()%40);
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
    MyTask *tasks[1000];

    for(int i=0;i<1000;i++){
        tasks[i]=new MyTask((void*)(x+i));
        while(t->threadpool_add(tasks[i])!=0);
    }
    // usleep(20);
    //  for(int i=500;i<1000;i++){
    //     tasks[i]=new MyTask((void*)(x+i));
    //     while(t->threadpool_add(tasks[i])!=0);
    // }
    t->stop();
    cout<<t->Status()<<endl;
    delete t;
    return -1;
}