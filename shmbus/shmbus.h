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
#define BUS_NOTIFY_PATH "/tmp/bus_noityf_pipe"

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
    ssize_t send(void* data,size_t send_size,size_t srcID,int notifyFd);
    ssize_t recv(void* dst,size_t recv_size);
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
    ShmBus();
    ~ShmBus();
    size_t send(size_t dstID,void* data,size_t size);
    size_t recv(void* buf,size_t size);

    int getListenFd();
    void init(size_t localID_);

private:
    size_t localID;
    int bus_notify_pipe;
    char busPipePath[64];
    map<key_t,int> serverPipeMap;
    map<key_t,ShmQueue*> queueMap;
    key_t getKey(size_t srcID,size_t dstID);
    ShmQueue* getQueue(key_t key);

    const char* getPathName(size_t ID);

};

#endif
