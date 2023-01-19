#include "Application.h"

#ifdef Q_OS_WIN
    #include <Winsock2.h>
    #include <windows.h>
    #include <conio.h>
    #include <fcntl.h>
#else
    #include <netdb.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

#include <QDateTime>
#include <QSettings>
#include <QHostInfo>
#include <QFile>
#include <QDir>
#include <cinttypes>
#include "Version.h"
#include "AuthChecker.h"
#include "Command/TerminalGuardAbility.h"
#include "File/FileTransmissionGuardAbility.h"
#include "Global/Component/Logger/Logger.h"
#include "Global/Component/Logger/FileLogger.h"


Application::Application(int &argc, char **argv)
    :   QCoreApplication(argc, argv)
{
    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(connectionCheck()));
}

Application::~Application()
{
    m_timer.stop();
    m_guard.reset();
    FileTransmission::TaskScheduler::release();
}

bool Application::init()
{
    QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);

    QString logDir = settings.value("Log/dir").toString();
    if (logDir.isEmpty())
        logDir = qApp->applicationDirPath() + "/log";

    QDir dir;
    dir.mkpath(logDir);
    Logger::init(std::unique_ptr<ILogger>(new FileLogger(logDir.toStdString() + "/log.txt")));
    Logger::info("%s Version %s", SERVICE_NAME, VERSION);
    Logger::info("Initializing");

    m_timerClearLog.setInterval(1800000); //30 minutes
    m_timerClearLog.callOnTimeout([](){
        QDate date = QDate::currentDate();
        date = date.addDays(-15);
        Logger::remove(date.year(), date.month(), date.day());
    });
    m_timerClearLog.start();

    std::string host = settings.value("Config/serverHost", "").toString().toStdString();
    unsigned port = settings.value("Config/serverPort").toUInt();
    if (!port || port >= 65535)
        port = 9748;
    std::string publicKeyFile = settings.value("Config/publicKeyFile", "").toString().toStdString();
    std::string account = settings.value("Config/account", "").toString().toStdString();
    std::string password = settings.value("Config/password", "").toString().toStdString();
    std::string publicKey;
    if (publicKeyFile.empty())
    {
        publicKeyFile = qApp->applicationDirPath().toStdString() + "/key.public";
    }

    QFile f(publicKeyFile.c_str());
    if (!f.open(QFile::ReadOnly))
    {
        Logger::error("open file '%s' failed", publicKeyFile.c_str());
        return false;
    }
    publicKey = f.readAll().toStdString();
    f.close();


    hostent* hostInfo = gethostbyname(host.c_str());
    if (!hostInfo || !hostInfo->h_addr_list[0])
    {
        Logger::error("server host invalid");
        return false;
    }
    std::string ip = inet_ntoa(*(struct in_addr*)hostInfo->h_addr_list[0]);

    AuthChecker::init();
    Logger::info("auth string: %s", AuthChecker::share()->getAuthString().c_str());

    if (!FileTransmission::TaskScheduler::init())
    {
        Logger::error("FileTransmission::TaskScheduler init failed");
        return false;
    }

    m_guard.reset(new Guard(host, ip, port, publicKey, account, password));

    std::unique_ptr<TerminalGuardAbility> terminalGuardAbility(new TerminalGuardAbility());
    m_guard->addAbility(std::move(terminalGuardAbility));

    std::unique_ptr<FileTransmissionGuardAbility> fileTransmissionGuardAbility(new FileTransmissionGuardAbility());
    m_guard->addAbility(std::move(fileTransmissionGuardAbility));

    m_guard->onLoggedIn = [this](Guard* g)
    {
        Logger::error("local user id % " PRId64 , g->getUserId());
    };

    m_guard->onLoginFailed = [this](Guard* g, int errCode, const std::string& errString)
    {
        Logger::error("login failed[%d]: %s", errCode, errString.c_str());
        g->disconnect();
    };

    m_guard->onError = [this](Guard* g, int errorCode, const std::string& errorString)
    {
        Logger::error("error[%d]: %s", errorCode, errorString.c_str());
    };

    auto err = m_guard->init();
    if (!err.first)
    {
        Logger::error("error: %s", err.second.c_str());
        return false;
    }

    m_timer.start(1000);

    return true;
}

void Application::connectionCheck()
{
    if (!m_guard->connected())
    {
        m_guard->connect();
        return;
    }
    if (!m_guard->loggedIn())
    {
        if (!m_guard->loggingIn())
            m_guard->login();
        return;
    }
}

