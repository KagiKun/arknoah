#ifndef _Connectd_H_
#define _Connectd_H_

#include <unistd.h>
#include <ev.h>
#include <fcntl.h>
#include "conn_config.h"
#include "connection_pool.h"
#include "shmbus.h"

#define LISTEN_BACKLOG 50
#define CONFIG_PATH "./config.ini"

class Connectd
{
private:
    enum ReasonOfClose
    {
        ERROR,
        TIME_OUT,

    };

private:
    Connectd();
    ~Connectd();


public:
    static Connectd& GetInstance();
    void Run();
    void ResolvePacket(ConnectionNode* node);
    void CloseConnection(int fd);
    void CloseConnection(ConnectionNode* node);

    friend void accept_cb(struct ev_loop *loop, ev_io* watcher,int revent);
    friend void recv_cb(struct ev_loop *loop, ev_io* watcher,int revent);
    friend void timeout_cb(struct ev_loop *loop,ev_timer* watcher,int revent);
    friend void sig_cb_stop(struct ev_loop *loop,ev_signal *watcher,int revent);

private:
    ConnectionNode *m_connectionPool;
    static const ConnConfig& m_config;
    int m_listenfd;

    ShmBus shmBus;

    struct ev_loop *main_loop;
    ev_io m_acceptWatcher;
    ev_io *m_recvWatchers;
    ev_timer *m_timerWatchers;


    int packetize(ConnectionNode* node,char* startPos,int dataLen);
};

void accept_cb(struct ev_loop *loop, ev_io *watcher,int revent);
void recv_cb(struct ev_loop *loop, ev_io *watcher,int revent);
void timeout_cb(struct ev_loop *loop,ev_timer* watcher,int revent);
void sig_cb_stop(struct ev_loop *loop,ev_signal *watcher,int revent);
void set_nonblocking(int fd);

#endif
