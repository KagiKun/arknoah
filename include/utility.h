#ifndef _UTILITY_H_
#define _UTILITY_H_

#include <fcntl.h>

void set_nonblocking(int fd)
{
    int tmp = fcntl(fd,F_GETFL);
    tmp |= O_NONBLOCK;
    fcntl(fd,F_SETFL,tmp);
}

#endif