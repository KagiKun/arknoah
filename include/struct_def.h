#include "../connectd/connection_pool.h"

typedef size_t SERVER_ID;

#pragma pack(push)
#pragma pack(1)

struct BusHead
{
    ConnectionNode* node;
    SERVER_ID src;
    SERVER_ID dst;
    size_t tempID;
};

struct BusPackage
{
    BusHead head;
    uint32_t dataLen;
    char data[];  //柔性数组，用于存储序列化后的数据
};

#pragma pack(pop)
