#ifndef _CONND_H
#define _CONND_H

#include <unistd.h>
#include <libev.h>
#include <ev.h>
#include "connection_pool.h"

class Connd()
{
private:
    Connd();
    ~Connd();

public:
    Connd& GetInstance();
    void run();

private:
    ConnectionPool& m_connectionPool;
    int m_listenfd;

    struct ev_loop *m_loop;
    ev_io m_ioWatcher; 
}



#endif
