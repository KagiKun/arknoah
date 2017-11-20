#include "connfd.h"

Connd::Connd():m_listenfd(0),m_loop(EV_DEFAULT)
{
    m_connectionPool = ConnectionPool::GetInstance();
}

Connd::~Connd()
{
}

Connd& Connd::GetInstance()
{
    static Connd connd;
    return connd;
}

void Connd::ListenInit()
{
    m_listenfd = socket(AF_INET,SOCK_STREAM|SOCK_NONBLOCK,0);
    if(m_listenfd<0)
    {
        perror("creat socket error");
        exit(1);
    }
    struct sockaddr servaddr;
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

    struct ev_loop *loop = EV_DEFAULT;
    ev_io listenWatcher;
    ev_io_init(&listenWatcher,listen_cb,m_listenfd,EV_READ);
    ev_io_start(loop,&listenWatcher);
    ev_run(loop,0);
}

void Connd::run()
{
    ListenInit();
}

