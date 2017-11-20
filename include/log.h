#include <stdio.h>

#define PATH "/var/log/arknoah"

int log_init()
{
    int fd = open(PATH,O_CREAT|O_APPEND|O_RDWR);
    if(fd<0)
    {
        printf("log file open failed\n");
        return -1;
    }
}


void log_err(char* info,char* file,int line)
