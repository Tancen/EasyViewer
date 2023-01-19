#include "MainWidget.h"
#include <QApplication>
#include <QDir>
#include "Global/Component/Logger/Logger.h"
#include "ServerAddressBook.h"
#include <qmessagebox.h>
#include "AuthChecker.h"
#include "File/FileTransmission.h"
#include "Desktop/Clipboard.h"
#include "Global/Component/Logger/FileLogger.h"
#include "Define.h"

#ifdef Q_OS_LINUX
    #include <sys/types.h>
    #include <signal.h>
#endif

#ifdef Q_OS_WIN
void checkUserGpuPreferences(const std::string& path)
{
    HKEY hKey;
    DWORD dwDisposition;
    long err;
    do
    {
        err = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\DirectX\\UserGpuPreferences",
                               0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
        if (err != ERROR_SUCCESS)
        {
            QMessageBox::critical(nullptr, "", QString::asprintf("Ccreate registry key failed[%d]", err));
            break;
        }

        const int BUF_LEN = 512;
        BYTE buf[BUF_LEN] = {0};
        DWORD len = BUF_LEN - 1;
        err = RegQueryValueExA(hKey, path.c_str(), NULL, NULL, buf, &len);
        if (err != ERROR_SUCCESS && err != ERROR_FILE_NOT_FOUND)
        {
            QMessageBox::critical(nullptr, "", QString::asprintf("query registry value failed[%d]", err));
            break;
        }

        const char* EXPECT_VALUE = "GpuPreference=1;";
        if (err == ERROR_FILE_NOT_FOUND || strcmp(EXPECT_VALUE, (char*)buf) != 0)
        {
            err = RegSetValueExA(hKey, path.c_str(), 0, REG_SZ, (BYTE*)EXPECT_VALUE, strlen(EXPECT_VALUE));
            if (err != ERROR_SUCCESS)
            {
                QMessageBox::critical(nullptr, "", QString::asprintf("set registry value failed[%d]", err));
                break;
            }

            QMessageBox::about(nullptr, "", QString::asprintf("Gpu Preferences has been set. Please restart this progrom manually."));

            break;
        }


        return;
    } while (0);

    RegCloseKey(hKey);
    exit(0);
}
#endif

int main(int argc, char *argv[])
{
    QApplication::addLibraryPath("plugins");
    QApplication a(argc, argv);
    QApplication::setWindowIcon(QIcon(ICON_48));

    QFile f(":/QSS/global.qss");
    f.open(QFile::ReadOnly);
    QString qss = f.readAll();
    f.close();
    a.setStyleSheet(qss);

#ifdef Q_OS_WIN
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    checkUserGpuPreferences(argv[0]);
#endif

    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string");
    qRegisterMetaType<User::ID>("User::ID");
    qRegisterMetaType<std::vector<FileTransmission::EntryInfo>>("std::vector<FileTransmission::EntryInfo>");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<FileTransmission::Task>("FileTransmission::Task");

    //log
    QDir dir(qApp->applicationDirPath());
    dir.mkdir("log");
    Logger::init(std::unique_ptr<ILogger>(new FileLogger((qApp->applicationDirPath() + "/log/log.txt").toStdString())));
    QTimer timerClearLog;
    timerClearLog.setInterval(1800000); //30 minutes
    timerClearLog.callOnTimeout([](){
        QDate date = QDate::currentDate();
        date = date.addDays(-15);
        Logger::remove(date.year(), date.month(), date.day());
    });
    timerClearLog.start();


    if (!ServerAddressBook::init())
    {
        QMessageBox::critical(nullptr, "error", "ServerAddressBook init failed");
        return 1;
    }

    AuthChecker::init();
    if (!FileTransmission::TaskScheduler::init())
    {
        QMessageBox::critical(nullptr, "error", "TaskScheduler init failed");
        return 1;
    }

    Clipboard::init();

    MainWidget *w = new MainWidget();
    w->show();
    bool ret = a.exec();
    delete w;

    FileTransmission::TaskScheduler::release();

#ifdef Q_OS_LINUX
    kill(0, SIGKILL);
#endif

    return ret;
}
