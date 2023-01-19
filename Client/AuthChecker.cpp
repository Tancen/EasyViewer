#include "AuthChecker.h"
#include <QSettings>
#include <QDir>
#include <qcoreapplication.h>

#define AUTH_FILE_DIR       (qApp->applicationDirPath() + "/data")
#define AUTH_FILE           (AUTH_FILE_DIR + "/auth.data")
#define AUTH_KEY            "Config/AuthString"

AuthChecker AuthChecker::s_this;
AuthChecker *AuthChecker::share()
{
    return &s_this;
}

void AuthChecker::init()
{
    QDir dir;
    dir.mkpath(AUTH_FILE_DIR);

    QSettings settings(AUTH_FILE, QSettings::IniFormat);
    s_this.m_authString = settings.value(AUTH_KEY).toString().toStdString();
    if (s_this.m_authString.empty())
    {
        srand(time(NULL));
        std::string str;
        for (size_t i = 0; i < 6; i++)
        {
            unsigned n = rand() % 62;
            if (n < 10)
                str.push_back('0' + n);
            else if (n < 36)
                str.push_back('a' + (n - 10));
            else
                str.push_back('A' + (n - 36));
        }
        s_this.setAuthString(str);
    }
}

std::string AuthChecker::getAuthString()
{
    m_lock.lock();
    auto ret = m_authString;
    m_lock.unlock();
    return ret;
}

void AuthChecker::setAuthString(const std::string &str)
{
    m_lock.lock();
    m_authString = str;
    QSettings settings(AUTH_FILE, QSettings::IniFormat);
    settings.setValue(AUTH_KEY, str.c_str());
    m_lock.unlock();
}

bool AuthChecker::test(const std::string &str)
{
    m_lock.lock();
    bool b = str == m_authString;
    m_lock.unlock();
    return b;
}

AuthChecker::AuthChecker()
{

}
