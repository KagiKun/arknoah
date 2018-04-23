#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "ev.h"
#include "shmbus.h"
#include "struct_def.h"

#define MAX_BUS_PACKET_LEN 1024

class Dispatchd
{
private:
    Dispatchd();
    ~Dispatchd();

public:
    static Dispatchd& GetInstance();
    void Run();

    friend void bus_handle_cb(struct ev_loop *loop,ev_io* w,int revent);
    friend void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent);

private:
    struct ev_loop *loop;
    ev_io *busWatchers;
    ShmBus shmbus;
    char busRecvBuf[MAX_BUS_PACKET_LEN];

    template<class Protocol>
    int Parse(Protocol& packet,void* dataBuf);
    template<class Protocol>
    size_t GetDstID(const Protocol& packet);


};


void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent);
void bus_handle_cb(struct ev_loop *loop,ev_io* w,int revent);

#endif
