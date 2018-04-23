#include "dispatchd.h"

Dispatchd::Dispatchd()
{}

Dispatchd::~Dispatchd()
{}

Dispatchd& Dispatchd::GetInstance()
{
    static Dispatchd ds;
    return ds; 
}


void Dispatchd::Run()
{
    loop = EV_DEFAULT;
    shmbus.init(2);
    ev_io busWatcher;
    ev_io_init(&busWatcher,bus_handle_cb,shmbus.getListenFd(),EV_READ);
    ev_io_start(loop,&busWatcher);

    ev_signal sigWatcher;
    ev_signal_init(&sigWatcher,sig_cb_stop,SIGINT);
    ev_signal_start(loop,&sigWatcher);

    ev_run(loop,0);
}

void bus_handle_cb(struct ev_loop *loop,ev_io *w,int revent)
{
    int ret = Dispatchd::GetInstance().shmbus.recv(Dispatchd::GetInstance().busRecvBuf,MAX_BUS_PACKET_LEN);
    struct BusPacket *packet = (BusPacket*)Dispatchd::GetInstance().busRecvBuf;
    int src = packet->busHead.src;
    printf("recv packet Len:%d\n,src=%d\n",ret,src);
    

}

void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent)
{
    ev_break(loop,EVBREAK_ALL);
    printf("loop stop\n");
}
