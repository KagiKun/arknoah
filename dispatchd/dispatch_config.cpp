#include "dispatch_config.h"

bool DispatchConfig::Init(string file_path)
{
    INIReader reader(file_path);
    if(reader.ParseError()<0)
    {
        printf("config file parse error\n");    
        return false;
    }
    set<string> sections = reader.Sections();
    if(sections.find("BUS_ID")==sections.end())
    {
        printf("config file must have \"BUS_ID\" section\n");
        return false;
    }
    for(string se : sections)
    {
        if(se.compare("BUS_ID")==0) 
        {
            CONNECTD_ID = reader.GetInteger(se,"CONNECTD_ID",1);
            DISPATCHD_ID = reader.GetInteger(se,"DISPATCHD_ID",2);
            ZONED_ID = reader.GetInteger(se,"ZONED_ID",3);
        }
    }
    return true;

}


