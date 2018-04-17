#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <set>
#include <string>
#include "shmbus.h"
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "INIReader.h"
#include "config.h"

#define PATH_NAME "./bus.log"

using namespace std;

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

void makeShmQueue(size_t srcID,size_t dstID,size_t length)
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

void removeShmQueue(size_t srcID,size_t dstID)
{
    key_t key = (srcID<<10)+dstID;
    int shmid = shmget(key,0,0);
    if(shmid<0)
    {
        std::cout << "shmget error when remove shmbus\n";
        exit(-1);
    }
    shmctl(shmid,IPC_RMID,NULL);
    
}

void makeShmBus(size_t ID1,size_t ID2,size_t size1,size_t size2)
{
    makeShmQueue(ID1,ID2,size1);
    makeShmQueue(ID2,ID1,size2);
}

void removeShmBus(size_t ID1,size_t ID2)
{
    removeShmQueue(ID1,ID2);
    removeShmQueue(ID2,ID1);
}

void makeBus(INIReader& reader)
{
    if(access(PATH_NAME,F_OK)<0)
        logfd = open(PATH_NAME,O_CREAT,0666);
    else
        logfd = open(PATH_NAME,O_WRONLY|O_APPEND);

    std::set<string> sections = reader.Sections();
    for(string se : sections)
    {
        size_t BUS_ID1 = 0;
        size_t BUS_ID2 = 0;
        size_t size1 = 0;
        size_t size2 = 0;
        if( (BUS_ID1=reader.GetInteger(se,"BUS_ID1",-1))==-1
            || (BUS_ID2=reader.GetInteger(se,"BUS_ID2",-1))==-1
            || (size1=reader.GetInteger(se,"SIZE1",-1))==-1
            || (size2=reader.GetInteger(se,"SIZE2",-1))==-1)
        {
            std::cout << "Parse configure file error\n";
            exit(1);
        }
        makeShmBus(BUS_ID1,BUS_ID2,size1,size2);
    }
    close(logfd);
    printf("share memory init complete\n");
}

void removeBus(INIReader& reader)
{
    std::set<string> sections = reader.Sections();
    for(string se : sections)
    {
        int BUS_ID1 = 0;
        int BUS_ID2 = 0;
        if((BUS_ID1=reader.GetInteger(se,"BUS_ID1",-1))==-1
            || (BUS_ID2=reader.GetInteger(se,"BUS_ID2",-1))==-1)
        {
            std::cout << "Parse configure file error\n";
            exit(1);
        }
        removeShmBus(BUS_ID1,BUS_ID2);
    }
}

int main(int argc,char** argv)
{
    if(argc<3)
    {
        std::cout << "Error use: bus make configure_file or bus clear configure_file\n";
        exit(1);
    }
    INIReader reader(argv[2]);
    if(reader.ParseError()<0)
    {
        std::cout << "Can't load " << argv[2] << std::endl;
        return -1;
    }
    if(strcmp(argv[1],"make")==0)
        makeBus(reader);
    else if(strcmp(argv[1],"remove")==0)
        removeBus(reader);
    else
        std::cout << "Error use: bus make configure_file or bus clear configure_file\n";
}
