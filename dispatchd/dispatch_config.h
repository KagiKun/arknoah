#ifndef _DISPATCH_CONFIG_
#define _DISPATCH_CONFIG_

#include <string>
#include <set>
#include <stdio.h>
#include "INIReader.h"

#define SERVER_ID size_t

using std::string;
using std::set;

class DispatchConfig
{
public:
    DispatchConfig() = default;
    ~DispatchConfig() = default;

    bool Init(string file_path);

    SERVER_ID CONNECTD_ID;
    SERVER_ID DISPATCHD_ID;
    SERVER_ID ZONED_ID;

        
};

#endif
