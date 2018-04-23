#include <stdio.h>
#include <fcntl.h>
#include "ev.h"

void cb(EV_P_ ev_io* w,int re)
{
    printf("a");
}

int main()
{
    struct ev_loop* loop= EV_DEFAULT;
    int fd = 1;
    ev_io w;
    ev_io_init(&w,cb,fd,EV_READ);
    ev_io_start(EV_A_ &w);
    
    return 1;
}


