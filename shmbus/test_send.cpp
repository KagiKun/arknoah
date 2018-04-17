#include "shmbus.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc,char** argv)
{

    ShmBus abus(CONNECT_ID);
    char message[] = "Hello";
    if(abus.send(ZONE_ID,message,sizeof(message))<0)
    {
        printf("send error\n");
        return 0;

    }
    printf("send success");
}