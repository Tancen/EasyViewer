#include "Application.h"
#include "Global/Component/Logger/Logger.h"
#include <cinttypes>

int main(int argc, char *argv[])
{
    Application a(argc, argv);

    if (!a.init())
    {
        fprintf(stderr, "Application initialize failed\n");
        return 1;
    }

    return a.exec();
}
