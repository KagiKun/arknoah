#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include "protocol.pb.h"
#include <string>

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
    printf("connection success\n");
    arknoah::Request request;
    request.mutable_proto_head()->set_cmd(arknoah::Request_head::INIT);
    request.mutable_proto_head()->set_passwd("464379852luo***");
    request.mutable_proto_head()->set_uid(123);
    std::string sendbuf;
    request.SerializeToString(&sendbuf);
    char* byteArry = new char[sendbuf.length()+4];
    *(uint32_t*)byteArry = (uint32_t)sendbuf.length();
    memcpy(byteArry+4,sendbuf.data(),sendbuf.length());
    printf("serialize size:%d\n",sendbuf.length());
    write(fd,byteArry,sendbuf.length()+4);
    printf("send complete\n");

    printf("test\n");
    uint32_t msgLen = *(uint32_t*)byteArry;
    std::string s_data(byteArry+4,byteArry+5+msgLen);
    arknoah::Request testPkg;
    if(!testPkg.ParseFromArray(byteArry+4,msgLen))
    {
        printf("parse error\n");
    }
    else
    {
        printf("parse success\n");
    }



    return 0;

        
}
