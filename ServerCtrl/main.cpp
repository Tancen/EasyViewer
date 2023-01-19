#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include "Global/Component/Logger/Logger.h"
#include "Global/Component/Logger/FileLogger.h"
#include <iostream>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include "CommandExecutor.h"
#include "Global/Protocol/Error.h"

#ifndef Q_OS_WIN
    #include <termios.h>
    #include <unistd.h>
    #include <signal.h>
    #include <stdio.h>
    // libreadline-dev
    #include <readline/readline.h>
    #include <readline/history.h>
#endif

#define COMMAND_LINE_ARG_SERVER_HOST        "host"
#define COMMAND_LINE_ARG_DESC_SERVER_HOST   "server host"

#define COMMAND_LINE_ARG_SERVER_PORT        "port"
#define COMMAND_LINE_ARG_DESC_SERVER_PORT   "server port"

#define COMMAND_LINE_ARG_PUBLIC_KEY_FILE         "public_key_file"
#define COMMAND_LINE_ARG_DESC_PUBLIC_KEY_FILE    "public key file"

#define VERSION "0.0.1"

void enableEchoInput()
{
#ifdef Q_OS_WIN
    DWORD  settings;
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    assert(hConsole != INVALID_HANDLE_VALUE);
    GetConsoleMode(hConsole, &settings);
    settings |= ENABLE_ECHO_INPUT;
    SetConsoleMode(hConsole, settings);
#else
    termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
#endif
}


void disableEchoInput()
{
#ifdef Q_OS_WIN
    DWORD  settings;
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);
    assert(hConsole != INVALID_HANDLE_VALUE);
    GetConsoleMode(hConsole, &settings);
    settings &= ~(ENABLE_ECHO_INPUT);
    SetConsoleMode(hConsole, settings);
#else
    termios settings;

    //set console
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
#endif
}

int main(int argc, char *argv[])
{
#ifndef Q_OS_WIN
        rl_getc_function = getc;
#endif

    QCoreApplication app(argc, argv);
    std::cout << "EasyViewer ServerCtrl Version " << VERSION << std::endl;

     QStringList arguments;
     for (int i = 0; i < argc; i++)
         arguments.append(argv[i]);

    QCommandLineOption optServerHost(COMMAND_LINE_ARG_SERVER_HOST, COMMAND_LINE_ARG_DESC_SERVER_HOST, COMMAND_LINE_ARG_SERVER_HOST);
    QCommandLineOption optServerPort(COMMAND_LINE_ARG_SERVER_PORT, COMMAND_LINE_ARG_DESC_SERVER_PORT, COMMAND_LINE_ARG_SERVER_PORT);
    QCommandLineOption optPublicKeyFile(COMMAND_LINE_ARG_PUBLIC_KEY_FILE, COMMAND_LINE_ARG_DESC_PUBLIC_KEY_FILE, COMMAND_LINE_ARG_PUBLIC_KEY_FILE);

    QCommandLineParser argsParser;
    argsParser.addOption(optServerHost);
    argsParser.addOption(optServerPort);
    argsParser.addOption(optPublicKeyFile);

    argsParser.addHelpOption();
    argsParser.addVersionOption();
    if (arguments.isEmpty())
    {
        std::cout << argsParser.helpText().toStdString();
        return 0;
    }

    argsParser.process(arguments);

    const char* DEFAULT_HOST = "127.0.0.1";
    QString host = argsParser.value(optServerHost);
    if (host.isEmpty())
        host = DEFAULT_HOST;

    const unsigned DEFAULT_PORT = 19748;
    unsigned port = argsParser.value(optServerPort).toUInt();
    if (port == 0)
        port = DEFAULT_PORT;

    QString publicKeyFilePath = argsParser.value(optPublicKeyFile);
    if (publicKeyFilePath.isEmpty())
    {
        publicKeyFilePath = qApp->applicationDirPath() + "/key.public";
    }

    std::string publicKey;
    QFile file(publicKeyFilePath);
    if (file.open(QFile::ReadOnly))
    {
        publicKey = file.readAll().toStdString();
        file.close();
    }
    std::promise<bool> promiseConnect;
    auto futureConnect = promiseConnect.get_future();

    Client client(host.toStdString(), port, publicKey);
    client.onConnected = [&promiseConnect]()
    {
        promiseConnect.set_value(true);
    };
    client.onConnectFailed = [&promiseConnect](const std::string&)
    {
        promiseConnect.set_value(false);
    };


    client.onDisconnected = []()
    {
        std::cout  << "error: disconnected" << std::endl;
        exit(0);
    };

    auto r = client.init();
    if (!r.first)
    {
        std::cout << r.second << std::endl;
        return 0;
    }
    client.connect();

    auto status = futureConnect.wait_for(std::chrono::seconds(10));
    if (status == std::future_status::timeout)
    {
        std::cout << "error: connection time out" << std::endl;
        return 0;
    }

    bool connected = futureConnect.get();
    if (!connected)
    {
        std::cout << "error: connect failed" << std::endl;
        return 0;
    }

    disableEchoInput();

    int i = 0;
    for (; i < 3; i++)
    {
        std::cout << "password:";

        std::string password;
#ifdef Q_OS_WIN
        std::cin >> password;
        std::cout << std::endl;
        if (std::cin.eof())
            return 0;
#else
        char* p = readline(nullptr);
        if (p == nullptr)
            return 0;
        password = p;
        fprintf(stdout, "\n");
#endif
        auto future = client.login(password);
        auto status = future.wait_for(std::chrono::seconds(10));
        if (status == std::future_status::timeout)
        {
            std::cout << "login timeout" << std::endl;
            return 0;
        }

        auto result = future.get();
        Client::LoginResult* loginResult = dynamic_cast<Client::LoginResult*>(result.get());
        if (loginResult == nullptr)
        {
            std::cout << "error: abnormal communication" << std::endl;
            return 0;
        }

        if (loginResult->errCode == GLOBAL_PROTOCOL_ERR_NO_ERROR)
            break;

        if (loginResult->errCode == GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT)
        {
            std::cout << "password incorrect, please try again!" << std::endl;
            continue;
        }

        std::cout << "error[" << loginResult->errCode << "]: " << loginResult->errString << std::endl;
        return 0;
    }
    if (i >= 3)
        return 0;

    enableEchoInput();

#ifdef Q_OS_WIN
    SetConsoleCtrlHandler(nullptr, true);

    EasyIO::ByteBuffer line;
#else
    struct sigaction act;
    act.sa_handler = [](int){};
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    int err = sigaction( SIGINT, &act, NULL);
    assert(!err);
#endif

    fseek(stdin, 0, SEEK_END);

    CommandExecutor executor;
    while (true)
    {
        #ifdef Q_OS_WIN
        std::cout << ">";
        do
        {
            int ch = getc(stdin);
            if (ch == EOF)
            {
                fprintf(stdout, "\n");
                break;
            }
            else if (ch != '\n')
                line.write((char)ch);
            else
            {
                line.write((char)0);
                executor.execute(line.data(), client);
                break;
            }
        } while (true);
        line.clear();
#else
        char* p = readline(">");
        if (p != nullptr)
        {
            if (p[0])
            {
                add_history(p);
                executor.execute(p, client);
            }
        }
        else
            fprintf(stdout, "\n");
        #endif
    }
}
