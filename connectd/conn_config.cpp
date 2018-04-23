#include "conn_config.h"

ConnConfig::ConnConfig()
{}

ConnConfig::~ConnConfig()
{}

ConnConfig& ConnConfig::GetInstance()
{
    static ConnConfig conn;
    return conn;
}

bool ConnConfig::Init(string pathName)
{
    INIReader reader(pathName);
    if(reader.ParseError()<0)
        return false;
    set<string> sections = reader.Sections();
    for(const string& se : sections)
    {
        if(se.compare("SERVER")==0)
        {
            SERVER_PORT = reader.GetInteger(se,"SERVER_PORT",6666);
            NUM_OF_NODE = reader.GetInteger(se,"NUM_OF_NODE",1024);
            MAX_PACKAGE_SIZE = reader.GetInteger(se,"MAX_PACKAGE_SIZE",1024);
            LEN_MASK = reader.GetInteger(se,"LEN_MASK",0xffff);
            TYPEID_MASK = reader.GetInteger(se,"TYYPEID_MASK",0xffff0000);
        }
        else if(se.compare("BUS_ID")==0)
        {
            CONNECT_ID = reader.GetInteger(se,"CONNECT_ID",1);
            DISPATCH_ID = reader.GetInteger(se,"DISPATCH_ID",2);
        }
    }
    return true;
}




