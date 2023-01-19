#include <QtGlobal>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <pty.h>
#endif
#include "Application.h"


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
