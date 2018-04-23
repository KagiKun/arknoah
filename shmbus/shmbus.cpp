#include <shmbus.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utility>
#include "log.h"


ShmQueue::ShmQueue()
{
}


ShmQueue::~ShmQueue()
{
 //   unlink(pathName);
}


ssize_t ShmQueue::init(key_t key)
{
    shmid = shmget(key,0,0);
    if(shmid<0)
    {
        log_error("shmget error",__FILE__,__LINE__);
        return -1;
    }

    void* addr = shmat(shmid,NULL,0);
    if(addr<0)
    {
        log_error("shmat error",__FILE__,__LINE__);
        return -1;
    }
    queueAddr = (void*)((char*)addr+sizeof(QueueHead));
    head = static_cast<QueueHead*>(addr);

    memset(pathName,0,NAME_MAX_LENGTH);
    snprintf(pathName,NAME_MAX_LENGTH,"/tmp/fifo:%d",shmid);
    if(access(pathName,F_OK)<0)
        if(mkfifo(pathName,0666)<0)
        {
            log_error("mkfifo error");
            return -1;
        }
    fifofd = open(pathName,0666);

    return 1;
}

ssize_t ShmQueue::send(void* data,size_t send_size,size_t src,int notifyFd)
{
    //因为是无锁，所以缓存此次调用的数据
    size_t front = head->front;
    size_t rear = head->rear;
    size_t size = head->size;
    void* addr = queueAddr;
    size_t left_size = front > rear?
                        front-rear: size-rear+front;
    if(left_size < send_size)
        return QUEUE_IS_FULL;
    //是否需要返回原点
    if(send_size+rear<=left_size)
    {//否
        memcpy((char*)addr+rear,data,send_size);
        head->rear+=send_size;
    }
    else
    {//是
        size_t trail_left = send_size - rear;
        memcpy((char*)addr+rear,data,trail_left);
        memcpy((char*)addr,(char*)data+trail_left,send_size-trail_left);
        head->rear = send_size-trail_left;
    }
    write(fifofd,&send_size,sizeof(size_t));
    write(notifyFd,&src,sizeof(src));
    return send_size;
}

ssize_t ShmQueue::recv(void* dst,size_t recv_size)
{
    size_t front = head->front;
    size_t rear = head->rear;
    size_t size = head->size;
    void* addr = queueAddr;

    size_t msg_size = 0;
    size_t diff_size = 0;
    size_t ret = read(fifofd,&msg_size,sizeof(size_t));
    if(msg_size>recv_size)     //如果实际发过来的包比接收缓存还要大，那么只接收预订缓存大小，并认定此包发生错误，将该包的后续舍去
    {
        log_error("The recv buffer of shmqueue is too small\n");
        diff_size = msg_size - recv_size;
        msg_size = recv_size;
    }
    if(front==rear)
        return QUEUE_IS_EMPTY;
    if(front+msg_size<=size)
    {
        //未溢出
        memcpy(dst,(char*)addr+front,msg_size);
        head->front += msg_size;
    }
    else
    {   //溢出
        memcpy(dst,(char*)addr+front,size-front);
        memcpy((char*)dst+size-front,addr,msg_size-front);
        head->front = msg_size - size + front;
    }
    if(diff_size>0)  //如果出错则进行调整，舍去该包剩下的数据
    {
        if(front+diff_size<=size)
            head->front += msg_size;
        else
            head->front = diff_size - size + front;
    }

    return msg_size;
}

const void* ShmQueue::peek(size_t* _size)
{
}

int ShmQueue::getFd()
{
    return fifofd;
}


ShmBus::ShmBus()
{
    log_init();
}


ShmBus::~ShmBus()
{
    for(auto pair : queueMap)
    {
        delete pair.second;
    }
}

void ShmBus::init(size_t localID_)
{
    localID = localID_;
    const char* path = getPathName(localID);
    if(access(path,F_OK)<0)
        if(mkfifo(path,0666)<0)
        {
            log_error("mkfifo for bus notify erro");
            exit(-1);
        }
    
    bus_notify_pipe = open(path,0666);

    
}

const char* ShmBus::getPathName(size_t ID)
{
    memset(busPipePath,0,64);
    snprintf(busPipePath,64,"%s%ld","/tmp/shmbus_notify_pipe",ID);
    return busPipePath;
}

size_t ShmBus::send(size_t dstID,void* data,size_t size)
{
    key_t key = getKey(localID,dstID);
    ShmQueue* pShmQueue = getQueue(key);
    if(pShmQueue==NULL)
    {
        log_error("get shm error while sending,srcID = ",__FILE__,__LINE__,dstID);
        return 0;
    }

    int busNotifyFd = 0;

    auto it = serverPipeMap.find(key);
    if(it==serverPipeMap.end())
    {
        const char* name = getPathName(dstID);
        busNotifyFd = open(name,0666);
        if(busNotifyFd<0)
        {
            log_error("dst server is no runing");
            return 0;
        }

        serverPipeMap.emplace(make_pair(key,busNotifyFd));
    }
    else
        busNotifyFd = it->second;

    return pShmQueue->send(data,size,localID,busNotifyFd);
}

size_t ShmBus::recv(void* buf,size_t size)
{
    size_t srcID;
    read(bus_notify_pipe,&srcID,sizeof(srcID));
    key_t key = getKey(srcID,localID);
    ShmQueue* pShmQueue = getQueue(key);
    if(pShmQueue==NULL)
    {
        log_error("get shm error while recving,srcID = ",__FILE__,__LINE__,srcID);
        return -1;
    }
    return pShmQueue->recv(buf,size);
}

key_t ShmBus::getKey(size_t srcID,size_t dstID)
{
    key_t key = (srcID<<10)+dstID;
    return key;
}

ShmQueue* ShmBus::getQueue(key_t key)
{
    ShmQueue* pShmQueue;
    auto search = queueMap.find(key);
    if(search!=queueMap.end())
        pShmQueue = search->second;
    else
    {
        pShmQueue = new ShmQueue;  //直至程序结束才被释放
        if(pShmQueue->init(key)<0)
        {
            log_error("shmQueue init error",__FILE__,__LINE__);
            return NULL;
        }
        queueMap.emplace(make_pair(key,pShmQueue));
    }

    return pShmQueue;
}

int ShmBus::getListenFd()
{
    return bus_notify_pipe;
}
