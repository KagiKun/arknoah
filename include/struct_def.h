#include "./connection_pool.h"

typedef SERVER_ID size_t

struct BusHead
{
    ConnectionNode* connectionNode;
    SERVER_ID Src;
    SERVER_ID Dst;
    size_t dataSize;
};

struct BusPacket
{
    BusHead busHead;
    char buffer[];  //柔性数组，用于存储序列化后的数据
};
