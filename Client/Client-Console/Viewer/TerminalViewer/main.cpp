#include <QtGlobal>

#ifdef Q_OS_WIN
    #include <windows.h>
#else
    #include <pty.h>
#endif
#include "Application.h"
#include <QDir>
#include "Global/Component/Logger/Logger.h"
#include "Global/Component/Logger/ConsoleLogger.h"


int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    bool success;
    DWORD  settings;

    //set console
    //input
    success= SetConsoleCP(65001); // utf-8
    assert(success);

    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    assert(hConsole != INVALID_HANDLE_VALUE);
    GetConsoleMode(hConsole, &settings);
    settings &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT | ENABLE_PROCESSED_INPUT | ENABLE_WINDOW_INPUT);
    settings |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(hConsole, settings);

    //output
    success = SetConsoleOutputCP(65001); // utf-8
    assert(success);

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    assert(hConsole != INVALID_HANDLE_VALUE);
    GetConsoleMode(hConsole, &settings);
    settings |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
    SetConsoleMode(hConsole, settings);

#else
    termios settings;

    //set console
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag &= ~(ICANON | ECHO);
    settings.c_lflag &= ~ISIG;
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
#endif


    Application a(argc, argv);

    //log
    Logger::init(std::unique_ptr<ILogger>(new ConsoleLogger()));
    Logger::setLevel(ILogger::Level::L_ERROR);

    int ret = 0;
    do
    {
        if (!a.init())
        {
            Logger::error("init failed");
            ret = 1;
            break;
        }

        ret =  a.exec();
    } while(0);
#ifdef Q_OS_WIN
    system("pause");
#endif

    return ret;
}
