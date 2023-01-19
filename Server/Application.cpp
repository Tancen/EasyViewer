#include <QtGlobal>
#include <QString>
#include <QDateTime>
#include "ApplicationServer.h"
#include "ManagementServer.h"
#include "RequestProcessor.h"

#include "Application.h"
#include "Global/Component/Logger/Logger.h"
#include "Version.h"
#include <QDir>
#include <QSettings>
#include "AccountManager.h"
#include "ServerBase.h"
#include "Task/TaskManager.h"
#include "Global/Component/Logger/FileLogger.h"

#define SAFELY_EXIT(p) \
    if(p) \
        p->exit();

#define SAFELY_WAIT_FOR_EXITED(p) \
    if(p) \
        p->waitForExited();


Application::Application(int &argc, char **argv)
    :   QCoreApplication(argc, argv)
{
    m_timer.setInterval(3600000 * 2);
    QObject::connect(&m_timer, SIGNAL(timeout()),
        this, SLOT(dealWithTriviality()));
}

Application::~Application()
{
    m_timer.stop();
    Logger::info("exiting");

    Logger::info("server closing");
    ApplicationServer::share()->close();
    ManagementServer::share()->close();
    Logger::info("server closed");


    Logger::info("RequestProcessor exiting");
    SAFELY_EXIT(RequestProcessor::share());

    SAFELY_WAIT_FOR_EXITED(RequestProcessor::share());
    Logger::info("RequestProcessor exited");


    Logger::info("Task::Manager exiting");
    SAFELY_EXIT(Task::Manager::share());

    SAFELY_WAIT_FOR_EXITED(Task::Manager::share());
    Logger::info("Task::Manager exited");


    Logger::info("UserArchiveManager releasing");
    AccountManager::release();
    Logger::info("UserArchiveManager released");

    Logger::info("exited");
}

bool Application::init()
{
    QSettings settings(qApp->applicationDirPath() + "/config.ini",
        QSettings::IniFormat);
    QDir::setCurrent(qApp->applicationDirPath());

    //log
    QString logDir = settings.value("Log/dir").toString();
    if (logDir.isEmpty())
        logDir = qApp->applicationDirPath() + "/log";

    QDir dir;
    dir.mkpath(logDir);
    Logger::init(std::unique_ptr<ILogger>(new FileLogger(logDir.toStdString() + "/log.txt")));

    m_timerClearLog.setInterval(1800000); //30 minutes
    m_timerClearLog.callOnTimeout([](){
        QDate date = QDate::currentDate();
        date = date.addDays(-15);
        Logger::remove(date.year(), date.month(), date.day());
    });
    m_timerClearLog.start();

    Logger::info("%s Version %s", SERVICE_NAME, VERSION);
    Logger::info("Initializing");

    m_lockFile.reset(new QLockFile(qApp->applicationDirPath() + "/lock"));
    if (!m_lockFile->tryLock())
    {
        Logger::error("The application is already running");
        return false;
    }

    //UserArchiveManager
    if(!AccountManager::init(qApp->applicationDirPath().toStdString()))
    {
        Logger::error("UserArchiveManager init failed");
        return false;
    }

    //TaskManager
    unsigned num = settings.value("Task/WorkerNum", 0).toUInt();
    if (!num || num > 128)
    {
        num = 2;
        Logger::warning("Task/WorkerNum invalid, reset to default value 2");
    }
    if (!Task::Manager::init(num))
    {
        Logger::error("Task::Manager init failed");
        return false;
    }

    //manager
    unsigned adminPort = settings.value("Management/Port").toUInt();
    if (!adminPort || adminPort >= 65535)
    {
        adminPort = 19748;
    }
    std::string adminPassword = settings.value("Management/Password", "").toString().toStdString();
    if (adminPassword.empty())
    {
        Logger::error("Management/Password cannot be empty");
        return false;
    }
    AccountManager::share()->updateAdminPassword(adminPassword);

    //Server
    unsigned port = settings.value("Server/Port").toUInt();
    if (!port || port >= 65535)
    {
        port = 9748;
    }

    QString privateKeyPath = settings.value("Server/PrivateKeyPath").toString();
    if (privateKeyPath.isEmpty())
        privateKeyPath = qApp->applicationDirPath() + "/key.private";

    QFile privateKeyFile(privateKeyPath);
    if (!privateKeyFile.open(QFile::ReadOnly))
    {
        Logger::error("open private key file failed");
        return false;
    }
    std::string privateKey = privateKeyFile.readAll().toStdString();
    privateKeyFile.close();

    if(!ApplicationServer::share()->open(port, privateKey))
    {
        Logger::error("application Server open port %d failed", port);
        return false;
    }
    Logger::info("application port %d opened", port);

    if(!ManagementServer::share()->open(adminPort, privateKey))
    {
        Logger::error("management Server open port %d failed", port);
        return false;
    }
    Logger::info("management port %d opened", adminPort);

    Logger::info("Initialized");
    return true;
}

void Application::dealWithTriviality()
{
    QDate t = QDate::currentDate().addDays(-(long long)m_logDays);
    Logger::remove(t.year(), t.month(), t.day(), -1);
}
