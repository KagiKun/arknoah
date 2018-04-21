#include "connectd.h"

int main()
{
    Connectd& connd = Connectd::GetInstance();
    connd.Run();
    return 0;
}
