#ifndef _DISPATCH_H_
#define _DISPATCH_H_

#include "ev.h"
#include "shmbus.h"
#include "struct_def.h"
#include "dispatch_config.h"
#include "glog/logging.h"
#include "protocol.pb.h"

using namespace Arknoah;
#define MAX_BUS_PACKAGE_LEN 1024
#define CONFIG_PATH "./config.ini"

class Dispatchd
{
private:
    Dispatchd() = default;
    ~Dispatchd();

public:
    static Dispatchd& GetInstance();
    void Run(const char* appName);

    friend void bus_handle_cb(struct ev_loop *loop,ev_io* w,int revent);
    friend void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent);

private:
    DispatchConfig dConfig;
    struct ev_loop *loop;
    ShmBus shmbus;
    char busRecvBuf[MAX_BUS_PACKAGE_LEN];

    template<class Protocol>
    int Parse(Protocol& package,void* dataBuf);
    template<class Protocol>
    SERVER_ID GetDstID(const Protocol& package);


};


void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent);
void bus_handle_cb(struct ev_loop *loop,ev_io* w,int revent);

#endif
