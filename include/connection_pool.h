/************************************
 *author:kagi
 *date: 2017.11.19
 *连接池，用于管理连接信息和缓存数据
 **********************************/
#ifndef _CONNECTION_POOL_H
#define _CONNECTION_POOL_H

#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <memory.h>
#include "config.h"

using namespace std;

//连接节点，用于管理连接
class ConnectionNode
{
public:
    ConnectionNode();
    ~ConnectionNode();

    void Recycle();
    
public:
    int m_connfd;
    int m_uin;

    int m_msgLen;
    int m_recvedLen;
    char* m_recvBuf;
};

//连接池，用于管理连接节点
class ConnectionPool
{
public:
    ConnectionPool();
    ~ConnectionPool();

public:

    void Recycle(ConnectionNode* _node);
    ConnectionNode* GetFreeNode();
    
private:
    ConnectionNode* m_nodePool;
    vector<ConnectionNode*> m_idleNode;
};

#endif
