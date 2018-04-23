#include <stdio.h>
#include <errno.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include "struct_def.h"
#include "shmbus.h"
#include "connectd.h"
#include "protocol.pb.h"


Connectd::Connectd():m_listenfd(0),main_loop(EV_DEFAULT)
{
    srandom(time(NULL));
}
Connectd::~Connectd()
{
    delete[] m_recvWatchers;
    delete[] m_timerWatchers;
 //   delete m_shmBus;
}


Connectd& Connectd::GetInstance()
{
    static Connectd connectd;
    return connectd;
}

void Connectd::TcpInit()
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
    servaddr.sin_port = htons(ConnConfig::GetInstance().SERVER_PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int opt_val = -1;
    if(setsockopt(m_listenfd,SOL_SOCKET,SO_REUSEADDR,&opt_val,sizeof(opt_val)))
    {
        perror("set socket reuseaddr error");
        exit(1);
    }

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

}

void Connectd::Run()
{
    if(!ConnConfig::GetInstance().Init(CONFIG_PATH))
    {
        perror("Config file init failed");
        exit(-1);
    }
    m_connectionPool = new ConnectionNode[ConnConfig::GetInstance().NUM_OF_NODE];
    m_recvWatchers = new ev_io[ConnConfig::GetInstance().NUM_OF_NODE];
    m_timerWatchers = new ev_timer[ConnConfig::GetInstance().NUM_OF_NODE];

    m_shmBus.init(ConnConfig::GetInstance().CONNECT_ID);

    TcpInit();

    ev_io_init(&m_acceptWatcher,accept_cb,m_listenfd,EV_READ);
    ev_io_start(main_loop,&m_acceptWatcher);

    ev_io_init(m_recvWatchers,bus_handle_cb,m_shmBus.getListenFd(),EV_READ);
    ev_io_start(main_loop,m_recvWatchers);

    ev_signal sw;
    ev_signal_init(&sw,sig_cb_stop,SIGINT);
    ev_signal_start(main_loop,&sw);

    ev_run(main_loop,0);
}

void Connectd::CloseConnection(ConnectionNode* node)
{
    CloseConnection(node->m_connfd);
}

void Connectd::CloseConnection(const int fd)
{
    m_connectionPool[fd].Recycle();
    ev_io_stop(main_loop, &m_recvWatchers[fd]);
    ev_timer_stop(main_loop, &m_timerWatchers[fd]);
    close(fd);
}

int Connectd::packetize(ConnectionNode* node,char* startPos,int dataLen)
{
    if(dataLen< 4)
    {
        return 0;
    }
    uint32_t msgLen = *(uint32_t*)startPos ;
    uint32_t packetLen = msgLen + 4;
    if(msgLen>ConnConfig::GetInstance().MAX_PACKAGE_SIZE)
    {
        //因为数据包有一个大概的范围，更多的是对数据流是否出错进行检测
        //如果出错，直接断开连接最省事
        CloseConnection(node->m_connfd);
        return 0;
    }
    node->m_msgLen = msgLen;
    if(packetLen>dataLen)
    {
        //未收到完整的包，缓存并等待下次数据
        memcpy(node->m_recvBuf,startPos,dataLen);
        node->m_recvedLen = dataLen;
        return 0;
    }

    printf("dataLen: %d\n msgLen: %d\n",dataLen,msgLen);

    //组装BusHead、发送至DisPatch服务器

    BusPacket *packet = (BusPacket*)busSendBuf;
    BusHead *head = (BusHead*)packet;
    head->node = node;
    head->tempID = node->m_tempID;
    head->dst = ConnConfig::GetInstance().DISPATCH_ID;
    head->src = ConnConfig::GetInstance().CONNECT_ID;
    memcpy(packet->data,startPos+4,msgLen);
    m_shmBus.send(ConnConfig::GetInstance().DISPATCH_ID,packet,sizeof(BusHead)+msgLen);

    //
    int totalLen = packetLen; //处理的总长度
    int leftLen = dataLen - packetLen;
    if(leftLen>0)
    {
        totalLen += packetize(node,startPos+packetLen,leftLen); //递归处理
    }

    return packetLen;

}

void Connectd::ResolvePacket(ConnectionNode* node)
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

void recv_cb(struct ev_loop *loop, ev_io *watcher,int revent)
{
#ifdef Debug
    printf("recv_cb\n");
#endif
    int fd = watcher->fd;
    char *socketRecvBuf = Connectd::GetInstance().socketRecvBuf;
    ConnectionNode* connection_pool = Connectd::GetInstance().m_connectionPool;
again:
    int ret = recv(fd,socketRecvBuf,RECVBUF_LEN,0);
    if(ret<0)
    {
        if(errno==EINTR)
            goto again;
        else if(errno==EAGAIN)  //据说libev可能会出现这种情况
            return;
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
       Connectd::GetInstance().CloseConnection(fd);
    }
    int& recvedLen = connection_pool[fd].m_recvedLen;

    if((recvedLen + ret)<=RECVBUF_LEN)
    {
        //新旧数据总长小于缓存
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                socketRecvBuf,
                ret);
        recvedLen += ret;
    }
    else
    {
        //大于缓存，先处理一段数据
        int diff = RECVBUF_LEN - recvedLen;
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                socketRecvBuf,
                diff);
        recvedLen += diff;
        Connectd::GetInstance().ResolvePacket(connection_pool+fd);
        if(recvedLen+ret-diff > RECVBUF_LEN)
        {
            //先处理后仍大于最大缓存，即一个包都大于缓存，视为出错
            Connectd::GetInstance().CloseConnection(fd);
        }
        memcpy( connection_pool[fd].m_recvBuf + recvedLen,
                socketRecvBuf,
                ret - diff);
        recvedLen += (ret-diff);
    }
    Connectd::GetInstance().ResolvePacket(connection_pool+fd);

    ev_timer *timer_watcher =  Connectd::GetInstance().m_timerWatchers + fd;
    ev_init(timer_watcher,timeout_cb);
    timer_watcher->repeat = TIMEOUT_SEC;
    ev_timer_again(loop,timer_watcher);
}


void accept_cb(struct ev_loop *loop, ev_io *watcher,int revent)
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
    ev_io *recv_watcher = Connectd::GetInstance().m_recvWatchers;
    ev_io_init(&recv_watcher[connfd],recv_cb,connfd,EV_READ);
    ev_io_start(loop, &recv_watcher[connfd]);

    ev_timer *timer_watcher = Connectd::GetInstance().m_timerWatchers;
    timer_watcher->data = Connectd::GetInstance().m_connectionPool + connfd;
    ev_timer_init(&timer_watcher[connfd],timeout_cb,300,0);
    ev_timer_start(loop,&timer_watcher[connfd]);

    Connectd::GetInstance().m_connectionPool[connfd].m_tempID = random();
}

void timeout_cb(struct ev_loop *loop,ev_timer *watcher,int revent)
{
    Connectd::GetInstance().CloseConnection((ConnectionNode*)watcher->data);
}

void sig_cb_stop(struct ev_loop *loop,ev_signal *watcher,int revent)
{
    printf("stop connectd server\n");
    ev_break(loop,EVBREAK_ALL);
}

void bus_handle_cb(struct ev_loop *loop,ev_io *watcher,int revent)
{
    char *busRecvBuf = Connectd::GetInstance().busRecvBuf;
    size_t ret = Connectd::GetInstance().m_shmBus.recv(busRecvBuf,MAX_BUS_PACKET);
    if(ret==0)
    {
        printf("shmbus recv error");
        return;
    }
    BusPacket *packet = (BusPacket*)busRecvBuf;
    BusHead *head = (BusHead*)packet;
    if(head->node->m_tempID != head->tempID)
    {
        printf("origin client is log out");
        return;
    }
    write(head->node->m_connfd,&packet->dataLen,packet->dataLen+4);
}

void set_nonblocking(int fd)
{
    int tmp = fcntl(fd,F_GETFL);
    tmp |= O_NONBLOCK;
    fcntl(fd,F_SETFL,tmp);
}
