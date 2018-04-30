#include "dispatchd.h"

Dispatchd::~Dispatchd()
{
    google::ShutdownGoogleLogging();
}

Dispatchd& Dispatchd::GetInstance()
{
    static Dispatchd ds;
    return ds; 
}



void Dispatchd::Run(const char* appName)
{
    loop = EV_DEFAULT;

    google::InitGoogleLogging(appName);
    FLAGS_log_dir = "/tmp/arknoah_log";
    FLAGS_logtostderr = 1;
    FLAGS_colorlogtostderr = 1;

    if(!dConfig.Init(CONFIG_PATH))
    {
        LOG(FATAL) << "Init Config file failed";
    }

    shmbus.init(2);
    ev_io busWatcher;
    ev_io_init(&busWatcher,bus_handle_cb,shmbus.getListenFd(),EV_READ);
    ev_io_start(loop,&busWatcher);

    ev_signal sigWatcher;
    ev_signal_init(&sigWatcher,sig_cb_stop,SIGINT);
    ev_signal_start(loop,&sigWatcher);

    ev_run(loop,0);
}

template<class Protocol>
SERVER_ID Dispatchd::GetDstID(const Protocol& package)
{
    switch( (Request::Head::Mask & package.head().cmd())>>8)
    {
        case Request::Head::Zoned :
            printf("Zone\n");
            return dConfig.ZONED_ID;
        default:
            printf("unknow server ID\n");
            break;
    }
}

void bus_handle_cb(struct ev_loop *loop,ev_io *w,int revent)
{

    Dispatchd& ds = Dispatchd::GetInstance();
    int ret = ds.shmbus.recv(ds.busRecvBuf,MAX_BUS_PACKAGE_LEN);
    struct BusPackage *package = (BusPackage*)ds.busRecvBuf;
    if(package->head.dst == ds.dConfig.DISPATCHD_ID)
    {
        Request req;
        if(!req.ParseFromArray(package->data,package->dataLen))
        {
            LOG(ERROR) << "Request parse error";
            return ;
        }
        LOG(INFO) << "UID: " << req.head().uid();
        SERVER_ID dst_ID = ds.GetDstID(req);
        package->head.dst = dst_ID;
    }

   // ds.shmbus.send(pacjet->dst,Dispatchd::GetInstance().busRecvBuf,ret);
    printf("parse dst id = %d\n",package->head.dst);
    printf("msgLen : %d\n",package->dataLen);
    printf("randomID: %d\n",package->head.tempID);

}

void sig_cb_stop(struct ev_loop *loop,ev_signal *w,int revent)
{
    ev_break(loop,EVBREAK_ALL);
    printf("loop stop\n");
}
