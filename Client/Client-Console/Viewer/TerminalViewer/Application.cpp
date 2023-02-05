#include "Application.h"

#ifdef Q_OS_WIN
    #include <Winsock2.h>
    #include <windows.h>
    #include <conio.h>
    #include <fcntl.h>
#endif

#include "Command/KeyboardHit.h"
#include "Command/TerminalArguments.h"
#include <QCommandLineParser>
#include <QHostInfo>
#include <QFile>
#include <QDebug>
#include "Global/Component/Logger/Logger.h"

Application::Application(int &argc, char **argv)
    :   QCoreApplication(argc, argv),
        m_viewer(nullptr)
{

}

Application::~Application()
{
    if (KeyboardHit::share())
    {
        KeyboardHit::share()->exit();
        KeyboardHit::share()->waitForExited();
    }


    if(m_viewer)
    {
        delete m_viewer;
    }

    KeyboardHit::release();
}

bool Application::init()
{
    QCommandLineOption optServerName(COMMAND_LINE_ARG_SERVER_NAME, COMMAND_LINE_ARG_DESC_SERVER_NAME, COMMAND_LINE_ARG_SERVER_NAME);
    QCommandLineOption optServerHost(COMMAND_LINE_ARG_SERVER_HOST, COMMAND_LINE_ARG_DESC_SERVER_HOST, COMMAND_LINE_ARG_SERVER_HOST);
    QCommandLineOption optServerPort(COMMAND_LINE_ARG_SERVER_PORT, COMMAND_LINE_ARG_DESC_SERVER_PORT, COMMAND_LINE_ARG_SERVER_PORT);
    QCommandLineOption optPublicKey(COMMAND_LINE_ARG_PUBLIC_KEY, COMMAND_LINE_ARG_DESC_PUBLIC_KEY, COMMAND_LINE_ARG_PUBLIC_KEY);
    QCommandLineOption optPublicKeyFile(COMMAND_LINE_ARG_PUBLIC_KEY_FILE, COMMAND_LINE_ARG_DESC_PUBLIC_KEY_FILE, COMMAND_LINE_ARG_PUBLIC_KEY_FILE);
    QCommandLineOption optAccount(COMMAND_LINE_ARG_ACCOUNT, COMMAND_LINE_ARG_DESC_ACCOUNT, COMMAND_LINE_ARG_ACCOUNT);
    QCommandLineOption optPassword(COMMAND_LINE_ARG_PASSWORD, COMMAND_LINE_ARG_DESC_PASSWORD, COMMAND_LINE_ARG_PASSWORD);
    QCommandLineOption optPartnerAlias(COMMAND_LINE_ARG_PARTNER_ALIAS, COMMAND_LINE_ARG_DESC_PARTNER_ALIAS, COMMAND_LINE_ARG_PARTNER_ALIAS);
    QCommandLineOption optPartnerId(COMMAND_LINE_ARG_PARTNER_ID, COMMAND_LINE_ARG_DESC_PARTNER_ID, COMMAND_LINE_ARG_PARTNER_ID);
    QCommandLineOption optAuthString(COMMAND_LINE_ARG_AUTH_STRING, COMMAND_LINE_ARG_DESC_AUTH_STRING, COMMAND_LINE_ARG_AUTH_STRING);
    QCommandLineOption optDebug(COMMAND_LINE_ARG_DEBUG, COMMAND_LINE_ARG_DESC_DEBUG);

    QCommandLineParser argsParser;
    argsParser.addOption(optServerName);
    argsParser.addOption(optServerHost);
    argsParser.addOption(optServerPort);
    argsParser.addOption(optPublicKey);
    argsParser.addOption(optPublicKeyFile);
    argsParser.addOption(optAccount);
    argsParser.addOption(optPassword);
    argsParser.addOption(optPartnerId);
    argsParser.addOption(optPartnerAlias);
    argsParser.addOption(optAuthString);
    argsParser.addOption(optDebug);

    argsParser.addHelpOption();
    argsParser.addVersionOption();
    argsParser.process(*this);

    if (argsParser.isSet(optDebug))
        Logger::setLevel(ILogger::Level::L_DEBUG);

    std::string serverName = argsParser.value(optServerName).toStdString();
    std::string host = argsParser.value(optServerHost).toStdString();
    unsigned port = argsParser.value(optServerPort).toUInt();
    if (!port)
        port = 9748;
    std::string partnerAlias = argsParser.value(optPartnerAlias).toStdString();
    std::string publicKey = argsParser.value(optPublicKey).toStdString();
    std::string account = argsParser.value(optAccount).toStdString();
    std::string password = argsParser.value(optPassword).toStdString();
    User::ID partnerUserId = argsParser.value(optPartnerId).toLongLong();
    std::string authString = argsParser.value(optAuthString).toStdString();

    // title
    QString title = QString::asprintf("%s-%s", (serverName.empty() ? host.c_str() : serverName.c_str()),
                                (partnerAlias.empty() ? std::to_string(partnerUserId).c_str() : partnerAlias.c_str()));
#ifdef Q_OS_WIN
    SetConsoleTitleA(title.toStdString().c_str());
#else
    printf("\033]0;%s\007", title.toStdString().c_str());
#endif

    if (publicKey.empty())
    {
        QString path = argsParser.value(optPublicKeyFile);
        if (path.isEmpty())
        {
            Logger::error("you must specify public_key or public_key_file");
            return false;
        }
        QFile file(path);
        if (file.open(QFile::ReadOnly))
        {
            publicKey = file.readAll().toStdString();
            file.close();
        }
        else
        {
            Logger::error("open file %s failed", path.toStdString().c_str());
            return false;
        }
    }
    auto hostInfo = QHostInfo::fromName(host.c_str());
    if (hostInfo.error() != QHostInfo::NoError)
    {
        Logger::error("server host invalid");
        return false;
    }
    std::string ip = hostInfo.addresses().first().toString().toStdString();

    if (publicKey.empty())
    {
        Logger::error("public key invalid");
        return false;
    }

    if(!KeyboardHit::init())
    {
        Logger::error("init KeyboardHit failed");
        return false;
    }

    m_viewer = new TerminalViewer(host, ip, port, publicKey, account, password, partnerUserId, authString);
    auto ret = m_viewer->init();
    if (!ret.first)
    {
        Logger::error(ret.second.c_str());
        return false;
    }

    m_viewer->login();
    return true;
}

