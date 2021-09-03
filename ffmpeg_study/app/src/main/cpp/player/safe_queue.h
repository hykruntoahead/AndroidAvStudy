//
// Created by hyk on 21-9-3.
//

#ifndef FFMPEG_STUDY_SAFE_QUEUE_H
#define FFMPEG_STUDY_SAFE_QUEUE_H
/**
 * 线程安全的队列
 */
#include <queue>
#include <pthread.h>

using namespace std;

template<typename T>
class SafeQueue {
    typedef void (*ReleaseHandle)(T &);

    typedef void (*SyncHandle)(queue<T> &);
private:
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    queue<T> q;
    bool mEnable;
    ReleaseHandle releaseHandle;
    SyncHandle syncHandle;

public:
    SafeQueue(){
        pthread_mutex_init(&mutex,0);
        pthread_cond_init(&cond,0);
    }
    ~SafeQueue(){
        pthread_cond_destroy(&cond);
        pthread_mutex_destroy(&mutex);
    }

    void enQueue(T new_value){
        pthread_mutex_lock(&mutex);
        if(mEnable){
            q.push(new_value);
            //thread_cond_signal函数的作用是发送一个信号给另外一个正在处于阻塞等待状态的线程,
            // 使其脱离阻塞状态,继续执行.如果没有线程处在阻塞等待状态,pthread_cond_signal也会成功返回。
            pthread_cond_signal(&cond);
        }else{
            releaseHandle(new_value);
        }
        pthread_mutex_unlock(&mutex);
    }

    int deQueue(T &value){
        int ret = 0;
        pthread_mutex_lock(&mutex);
        //在多核处理器下 由于竞争可能虚假唤醒
        while(mEnable && q.empty()){
            pthread_cond_wait(&cond,&mutex);
        }

        if (!q.empty()){
            value = q.front();
            q.pop();
            ret = 1;
        }

        pthread_mutex_unlock(&mutex);
        return ret;
    }

    void setEnable(bool enable){
        pthread_mutex_lock(&mutex);
        this->mEnable = enable;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }

    int empty(){
        return q.empty();
    }

    int size(){
        return q.size();
    }

    void clear(){
        pthread_mutex_lock(&mutex);
        for (int i=0;i<size();++i){
            T value = q.front();
            releaseHandle(value);
            q.pop();
        }
        pthread_mutex_unlock(&mutex);
    }

    void sync(){
        pthread_mutex_lock(&mutex);
        syncHandle(q);
        pthread_mutex_unlock(&mutex);
    }

    void setReleaseHandle(ReleaseHandle r){
        releaseHandle = r;
    }

    void setSyncHandle(SyncHandle s){
        syncHandle = s;
    }
};

#endif //FFMPEG_STUDY_SAFE_QUEUE_H