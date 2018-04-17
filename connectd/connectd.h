#ifndef _Connectd_H_
#define _Connectd_H_

#include <unistd.h>
#include <ev.h>
#include "config.h"
#include "connection_pool.h"

class Connectd
{
public:
    Connectd();
    ~Connectd();

public:
    void run();
    void resolvePacket(ConnectionNode* node);
    void closeConnection(int fd);

    friend void accept_cb(EV_P_ ev_io* watcher,int revent);
    friend void recv_cb(EV_P_ ev_io* watcher,int revent);

private:
    ConnectionNode m_connectionPool[NUM_OF_NODE];
    int m_listenfd;

    struct ev_loop *loop;
    ev_io m_recvWatcher[NUM_OF_NODE];
    ev_io m_acceptWatcher;


    int packetize(ConnectionNode* node,char* startPos,int dataLen);
};

void accept_cb(EV_P_ ev_io *watcher,int revent);
void recv_cb(EV_P_ ev_io *watcher,int revent);

#endif
