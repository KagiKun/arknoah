#include "dispatchd.h"


int main(int argc,char *argv[])
{
    Dispatchd::GetInstance().Run(argv[0]);
    return 0;
}
