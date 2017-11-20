#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "config.h"
#include "ev.h"
#include "connection_pool.h"
#include <errno.h>

#define Debug

ev_io listen_watcher;
ev_io connection_watcher[NODE_NUM];


struct ConnectionNode connection_pool[NODE_NUM]; 

void set_nonblocking(int fd)
{
    int tmp = fcntl(fd,F_GETFL);
    tmp |= O_NONBLOCK;
    fcntl(fd,F_SETFL,tmp);
}

void close_connection(ConnectionNode* node)
{
    ev_stop(EV_A_ &connection[node->fd]);
    close(node->fd);
    node->Recycle();
}


int resolve_package_core(char* start,int len

void resolve_package(ConnectionNode* node)
{
    char *recvBuf = node->m_recvBuf;
    bool has_left = false;
    while(!has_left)
    {
        if(node->recvedLen < 4)
        {
            return; 
        }
        uint32_t msgLen = *(uint32_t*)recvBuf;
        node->m_msgLen = msgLen;

    
    }
}

static void recv_cb(EV_P_ ev_io *watcher,int revent)
{
#ifdef Debug
    printf("recv_cb\n");
#endif
    int fd = watcher->fd;
    char recv_buf[RECVBUF_LEN];
    bzero(recv_buf,RECVBUF_LEN);
again:
    int ret = recv(fd,recv_buf,RECVBUF_LEN,0);
    if(ret<0)
    {
        if(errno==EINTR)
            goto again;
        else 
        {
#ifdef Debug
            printf("recv error\n");
#endif
            return;
        }
    }
    else if(ret==0)
    {
        connection_pool[fd].Recycle();
        ev_io_stop(EV_A_ watcher);
    }
    
    int& recvedLen = connection_pool[fd].m_recvedLen;

    if((recvedLen + ret)<=RECVBUF_LEN)
    {
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                ret);
        recvedLen += ret;
    }
    else
    {
        int diff = RECVBUF_LEN - recvedLen;
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                diff);
        resolve_package(connection_pool+fd);
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                ret - diff);
    }
    resolve_package(connection_pool+fd);
}


static void accept_cb(EV_P_ ev_io *watcher,int revent)
{
#ifdef Debug
    printf("accept\n");
#endif
    int connfd = accept(watcher->fd,NULL,NULL);
    if(connfd<0)
    {
#ifdef Debug
        printf("accept error");
#endif
        return;
    }
    set_nonblocking(connfd);
    ev_io_init(&connection_watcher[connfd],recv_cb,connfd,EV_READ);
    ev_io_start(EV_A_ &connection_watcher[connfd]);

}

int listen_init()
{
    int fd = socket(AF_INET,SOCK_STREAM,0);
    assert(fd);
    
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(fd,(sockaddr*)&servaddr,sizeof(servaddr));
    if(ret<0)
    {
        printf("bind error\n");
        exit(1); 
    }

    ret = listen(fd,LISTEN_BACKLOG);
    if(ret<0)
    {
        printf("listen failed");
        exit(1);
    }

    return fd;
}

int main()
{
    int listenfd = listen_init();

    struct ev_loop * loop;
    loop = EV_DEFAULT;
    ev_io_init (&listen_watcher,accept_cb,listenfd,EV_READ);
    ev_io_start(loop,&listen_watcher);

    printf("connd server init complete\n");
    ev_run(loop,0);
    
    return 0;
}
