#ifndef _SHMQUEUE_H
#define _SHMQUEUE_H

#include <queue>
#include <unistd.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <map>

#define QUEUE_IS_FULL -1
#define QUEUE_IS_EMPTY -2
#define NAME_MAX_LENGTH 50


using namespace std;

struct QueueHead
{

    size_t front;
    size_t rear;
    size_t size;
    void* addr;

    std::queue<size_t> sizeQueue;
};

class ShmQueue
{
public:
    ShmQueue();
    ~ShmQueue();

    ssize_t init(key_t key);
    ssize_t send(void* data,size_t send_size);
    ssize_t recv(void* dst);
    const void* peek(size_t* _size);

    int getFd();
    void notify();

private:
    QueueHead* head;
    int shmid;
    int fifofd;
    void* queueAddr;
    char pathName[NAME_MAX_LENGTH];
};

class ShmBus
{
public:
    ShmBus(size_t localID);
    ~ShmBus();
    size_t send(size_t dstID,void* data,size_t size);
    size_t recv(size_t srcID,void* buf,size_t size);
    int getListenFd(size_t srcID);

private:
    size_t localID;
    map<key_t,ShmQueue*> keyMap;
    key_t exchange(size_t srcID,size_t dstID);
    ShmQueue* getQueue(key_t key);

};

#endif
