#include "shmbus.h"
#include "config.h"
#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    ShmBus abus(ZONE_ID);
    int queuefd = abus.getListenFd(CONNECT_ID);
    if(queuefd<0)
    {
        printf("get queue fd error");
        return 0;
    }
    int epollfd = epoll_create(1);
    struct epoll_event ev;
    ev.data.fd = queuefd;
    ev.events = EPOLLIN;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,queuefd,&ev);
    while(1)
    {
        struct epoll_event evs[100];
        epoll_wait(epollfd,evs,100,-1);
        char recvBuf[100];
        size_t ret = abus.recv(CONNECT_ID,recvBuf);
        printf("recv size %ud\n%s\n",ret,recvBuf);
    }
}
