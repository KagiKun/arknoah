#ifndef CONN_CONFIG
#define CONN_CONFIG

#include "INIReader.h"
#include <string>
#include <set>

using namespace std;
class ConnConfig
{
private:
    ConnConfig();
    ~ConnConfig();

public:
    bool Init(string pathName);
    static ConnConfig& GetInstance();

public:
    size_t SERVER_PORT;
    size_t LEN_MASK;
    size_t TYPEID_MASK;
    size_t NUM_OF_NODE;
    size_t MAX_PACKAGE_SIZE;

    size_t CONNECTD_ID;
    size_t DISPATCHD_ID;
};

#endif
