#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "connection_pool.h"
#include "config.h"


ConnectionNode::ConnectionNode():m_connfd(0),m_uin(0),m_msgLen(0),m_recvedLen(0)
{
    m_recvBuf = new char[RECVBUF_LEN];
    memset(m_recvBuf,0,RECVBUF_LEN);
}

ConnectionNode::~ConnectionNode()
{
    delete m_recvBuf;
}


void ConnectionNode::Recycle()
{
    memset(m_recvBuf,0,RECVBUF_LEN);
    m_connfd = 0;
    m_uin = 0;
    m_msgLen = 0;
    m_recvedLen = 0;
}

/********************************************/

ConnectionPool::ConnectionPool()
{
    m_nodePool = new ConnectionNode[NUM_OF_NODE];
    if(m_nodePool==NULL)
    {
        perror("creat pool failed");
        exit(1);
    }
    for(int i=0;i<NUM_OF_NODE;++i)
        m_idleNode.push_back(m_nodePool+i);
}

ConnectionPool::~ConnectionPool()
{
    delete m_nodePool;
}

ConnectionNode* ConnectionPool::GetFreeNode()
{
    ConnectionNode* node =NULL;
    if(!m_idleNode.empty())
    {
        node = m_idleNode.back();
        m_idleNode.pop_back();
    }
    return node;
}

void ConnectionPool::Recycle(ConnectionNode* node)
{
    node->Recycle();
    m_idleNode.push_back(node);
}
