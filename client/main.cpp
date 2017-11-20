#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>

int main()
{
    int fd = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in servaddr;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    inet_pton(fd,"127.0.0.1",&servaddr.sin_addr);

    if(connect(fd,(sockaddr*)&servaddr,sizeof(servaddr))<0)
    {
        perror("connection error\n");
        exit(1);
    }
    printf("connection success");
    char msg[] = "asd";
    send(fd,msg,strlen(msg),0);

    char buf[200];
    recv(fd,buf,200,0);

    return 0;

        
}
