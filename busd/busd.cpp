#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include "shmbus.h"
#include <time.h>
#include <fcntl.h>
#include "config.h"

#define PATH_NAME "./bus.log"
#define length 1024*1024

int logfd;

void log_write(int shmid,int srcID,int dstID)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    char buf[100];
    memset(buf,0,100);
    snprintf(buf,100,"%04d.%02d.%02d %02d:%02d:%02d ID:%d->%d shmid:%d\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,srcID,dstID,shmid);
    write(logfd,buf,strlen(buf));
}

void log_write(const char*info)
{
    time_t timet = time(NULL);
    struct tm* ptm = localtime(&timet);
    char buf[100];
    memset(buf,0,100);
    snprintf(buf,100,"%04d.%02d.%02d %02d:%02d:%02d error: %s\n",ptm->tm_year+1900,ptm->tm_mon+1,ptm->tm_mday,ptm->tm_hour,ptm->tm_min,ptm->tm_sec,info);
    write(logfd,buf,strlen(buf));
}

void makeShmQueue(size_t srcID,size_t dstID)
{
    key_t key  = (srcID<<10)+dstID;
    
    int shmid = shmget((key_t)key,length,IPC_CREAT|0666);
    if(shmid<0)
    {
         printf("shmget error\n");
         log_write("shmget error");
         exit(-1);
    }

    void* ptr = shmat(shmid,NULL,0);
    if(ptr<0)
    {
         printf("shmat error\n");
         log_write("shmat error");
         exit(-1);
    }

    QueueHead* pHead = static_cast<QueueHead*>(ptr);
    new (pHead) QueueHead;
    pHead->size = length - sizeof(QueueHead);
    pHead->addr = static_cast<void*>(static_cast<char*>(ptr) + sizeof(QueueHead));
    pHead->front = pHead->rear = 0;

    log_write(shmid,srcID,dstID);
}

void makeShmBus(size_t ID1,size_t ID2)
{
    makeShmQueue(ID1,ID2);
    makeShmQueue(ID2,ID1);
}

int main()
{
    if(access(PATH_NAME,F_OK)<0)
        logfd = open(PATH_NAME,O_CREAT,0666);
    else
        logfd = open(PATH_NAME,O_WRONLY|O_APPEND);

    makeShmBus(CONNECT_ID,ZONE_ID);
    close(logfd);

    printf("share memory init complete\n");
}
