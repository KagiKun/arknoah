#include <stdio.h>
#include <errno.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "connectd.h"
#include "utility.h"
#include "protocol.pb.h"



Connectd::Connectd():m_listenfd(0),loop(EV_DEFAULT)
{
}

Connectd::~Connectd()
{
}

void Connectd::run()
{
    m_listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    if(m_listenfd<0)
    {
        perror("creat socket error");
        exit(1);
    }
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(m_listenfd,(sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        perror("bind error");
        exit(1);
    }

    if(listen(m_listenfd,LISTEN_BACKLOG)<0)
    {
        perror("listen error");
        exit(1);
    }

    ev_io_init(&m_acceptWatcher,accept_cb,m_listenfd,EV_READ);
    ev_io_start(loop,&m_acceptWatcher);
    ev_run(loop,0);
}

void Connectd::closeConnection(const int fd)
{
    m_connectionPool[fd].Recycle();
    ev_io_stop(EV_A_ &m_recvWatcher[fd]);
    close(fd);
}

int Connectd::packetize(ConnectionNode* node,char* startPos,int dataLen)
{
    if(dataLen< 4)
    {
        return 0;
    }
    uint32_t msgLen = *(uint32_t*)startPos ;
    if(msgLen>MAX_PACKAGE_SIZE)
    {
        //因为数据包有一个大概的范围，更多的是对数据流是否出错进行检测
        //如果出错，直接断开连接最省事
        closeConnection(node->m_connfd);
        return 0;
    }
    node->m_msgLen = msgLen;
    if(msgLen>dataLen)
    {
        //未收到完整的包，等待下次数据
         return 0;
    }

    printf("dataLen: %d\n msgLen: %d\n",dataLen,msgLen);

    //反序列化、发送完整的包
    arknoah::Request requestPkg;
    if(!requestPkg.ParseFromArray(startPos+4,msgLen))
    {
        printf("Parse error\n");
        exit(1);
    }

    switch(requestPkg.proto_head().cmd())
    {
        case arknoah::Request_head_PackageType_INIT:
            printf("get inint\n");
    }

    int totalLen = msgLen; //处理的总长度
    int leftLen = dataLen - msgLen;
    if(leftLen>0)
    {
        totalLen += packetize(node,startPos+msgLen,leftLen); //递归处理
    }

    return msgLen;

}

void Connectd::resolvePacket(ConnectionNode* node)
{
    int recvedLen = node->m_recvedLen;
    char* nodeBuf = node->m_recvBuf;
    int ret = packetize(node,nodeBuf,recvedLen);
    if(ret>0 && node->m_recvedLen>0)
    {
        node->m_recvedLen -= ret;
        memcpy(node->m_recvBuf,node->m_recvBuf+ret,node->m_recvedLen);  //解包剩余数据移动到缓冲区前
    }
}

void recv_cb(EV_P_ ev_io *watcher,int revent)
{
#ifdef Debug
    printf("recv_cb\n");
#endif
    int fd = watcher->fd;
    char recv_buf[RECVBUF_LEN];
    bzero(recv_buf,RECVBUF_LEN);

    ConnectionNode* connection_pool = Connectd::GetInstance().m_connectionPool;
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
        //用户断开
       Connectd::GetInstance().closeConnection(fd);
    }
    int& recvedLen = connection_pool[fd].m_recvedLen;

    if((recvedLen + ret)<=RECVBUF_LEN)
    {
        //新旧数据总长小于缓存
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                ret);
        recvedLen += ret;
    }
    else
    {
        //大于缓存，先处理一段数据
        int diff = RECVBUF_LEN - recvedLen;
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                diff);
        recvedLen += diff;
        Connectd::GetInstance().resolvePacket(connection_pool+fd);
        if(recvedLen+ret-diff > RECVBUF_LEN)
        {
            //先处理后仍大于最大缓存，即一个包都大于缓存，视为出错
            Connectd::GetInstance().closeConnection(fd);
        }
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                recv_buf,
                ret - diff);
        recvedLen += (ret-diff);
    }
    Connectd::GetInstance().resolvePacket(connection_pool+fd);
}


void accept_cb(EV_P_ ev_io *watcher,int revent)
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
    ev_io *recv_watcher = Connectd::GetInstance().m_recvWatcher;
    ev_io_init(&recv_watcher[connfd],recv_cb,connfd,EV_READ);
    ev_io_start(EV_A_ &recv_watcher[connfd]);

}
