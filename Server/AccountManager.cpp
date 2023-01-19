#include "AccountManager.h"
#include "Global/Protocol/Error.h"
#include <regex>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>
#include <qvariant.h>
#include "Global/Component/Logger/Logger.h"
#include "Global/Define.h"
#include <QDateTime>
#include <QCryptographicHash>

#define DB_NAME "UserArchive.db"
#define TABLE_NAME "user"

AccountManager* AccountManager::s_this = nullptr;

AccountManager *AccountManager::share()
{
    return s_this;
}

void AccountManager::release()
{
    delete s_this;
}

AccountManager::~AccountManager()
{
    m_db.close();
}

int AccountManager::listAccounts(const std::string& filter, std::vector<Account> &dst)
{
    QString sql = "SELECT id, account, password, disabled FROM " TABLE_NAME ;
    if (!filter.empty())
        sql += " WHERE account like ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    if (!filter.empty())
        query.bindValue(0, ("%" + filter + "%").c_str());
    bool b = query.exec();
    if(!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    while (query.next())
    {
        dst.push_back(Account{
            query.value(0).toLongLong(),
            query.value(1).toString().toStdString(),
            query.value(2).toString().toStdString(),
            query.value(3).toBool()
        });
    }

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

bool AccountManager::checkAdminPassword(const std::string &password)
{
    std::lock_guard<std::recursive_mutex> g(m_lock);
    return password == m_adminPassword;
}

int AccountManager::updateAdminPassword(const std::string& oldPassword, const std::string& temporaryPassword)
{
    std::lock_guard<std::recursive_mutex> g(m_lock);
    if(m_adminPassword != oldPassword)
        return GLOBAL_PROTOCOL_ERR_PASSWORD_INCORRECT;
    m_adminPassword = temporaryPassword;
}

void AccountManager::updateAdminPassword(const std::string &password)
{
    std::lock_guard<std::recursive_mutex> g(m_lock);
    m_adminPassword = password;
}



bool AccountManager::init(const std::string& dbFileDir)
{
    assert(s_this == nullptr);
    do
    {
        s_this = new AccountManager();
        s_this->m_db = QSqlDatabase::addDatabase("QSQLITE", "UserArchiveManager");
        s_this->m_db.setDatabaseName((dbFileDir + "/" DB_NAME).c_str());
        if(!s_this->m_db.open())
        {
            Logger::error("%s:%d - !s_this->m_db.open: %s", __PRETTY_FUNCTION__, __LINE__,
                          s_this->m_db.lastError().text().toStdString().c_str());
            break;
        }

        QString sql = "CREATE TABLE IF NOT EXISTS " TABLE_NAME "( "
                 "id BIGINT NOT NULL, "
                 "account VARCHAR(24) NOT NULL, "
                 "password VARCHAR(24) NOT NULL, "
                 "disabled INT NOT NULL DEFAULT 0 "
          ")";

        QSqlQuery query(s_this->m_db);
        if(!query.exec(sql))
        {
            Logger::error("%s:%d - !s_this->m_db.open: %s", __PRETTY_FUNCTION__, __LINE__,
                          query.lastError().text().toStdString().c_str());
            break;
        }

        return true;
    } while (0);

    delete s_this;
    s_this = nullptr;
    return false;
}

int AccountManager::addAccount(const std::string &account, const std::string password, User::ID &id)
{
    static constexpr size_t MAX_LEN = 64;
    std::regex r("^[a-zA-Z0-9.@]+$");

    if (account.length() > MAX_LEN || !std::regex_match(account.c_str(), r)
            || password.length() > MAX_LEN || !std::regex_match(password.c_str(), r))
        return GLOBAL_PROTOCOL_ERR_INVALID_PARAMS;

    id = generateUserID();
    if (!id)
        return GLOBAL_PROTOCOL_ERR_UNKNOW;

    QString sql = "INSERT INTO " TABLE_NAME "(id, account, password) VALUES(?, ?, ?)";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, (long long)id);
    query.bindValue(1, account.c_str());
    query.bindValue(2, encodePassword(account, password).c_str());
    bool b = query.exec();

    if(!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

int AccountManager::getAccount(const std::string &account, Account &info)
{
    QString sql = "SELECT id, password, disabled FROM " TABLE_NAME " WHERE account = ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, account.c_str());
    bool b = query.exec();
    if (!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    if (!query.next())
        return GLOBAL_PROTOCOL_ERR_ACCOUNT_NOT_EXISTS;

    info.account = account;
    info.id = query.value(0).toLongLong();
    info.password = query.value(1).toString().toStdString();
    info.disabled = query.value(2).toBool();

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

int AccountManager::getAccount(User::ID userId, Account &info)
{
    QString sql = "SELECT id, account, password, disabled FROM " TABLE_NAME " WHERE id = ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, (long long)userId);
    bool b = query.exec();
    if (!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    if (!query.next())
        return GLOBAL_PROTOCOL_ERR_ACCOUNT_NOT_EXISTS;

    info.id = query.value(0).toLongLong();
    info.account = query.value(1).toString().toStdString();
    info.password = query.value(2).toString().toStdString();
    info.disabled = query.value(3).toBool();

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

int AccountManager::enableAccount(const std::string &account)
{
    return setAccountDisabled(account, false);
}

int AccountManager::disableAccount(const std::string &account)
{
    return setAccountDisabled(account, true);
}

int AccountManager::deleteAccount(const std::string &account)
{
    QString sql = "DELETE FROM " TABLE_NAME " WHERE account = ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, account.c_str());
    bool b = query.exec();
    if(!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

int AccountManager::updatePassword(const std::string &account, const std::string &password)
{
    QString sql = "UPDATE " TABLE_NAME " SET password = ? WHERE account = ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, encodePassword(account, password).c_str());
    query.bindValue(1, account.c_str());
    bool b = query.exec();
    if(!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

bool AccountManager::checkPassword(const Account &account, const std::string &password)
{
    std::string pw = encodePassword(account.account, password);
    return account.password ==  pw;
}

int AccountManager::setAccountDisabled(const std::string &account, bool disabled)
{
    QString sql = "UPDATE " TABLE_NAME " SET disabled = ? WHERE account = ?";

    std::lock_guard<std::recursive_mutex> g(m_lock);
    QSqlQuery query(m_db);
    query.prepare(sql);
    query.bindValue(0, disabled);
    query.bindValue(1, account.c_str());
    bool b = query.exec();
    if(!b)
    {
        Logger::error("%s:%d - !query.exec: %s", __PRETTY_FUNCTION__, __LINE__,
                      query.lastError().text().toStdString().c_str());
        return GLOBAL_PROTOCOL_ERR_UNKNOW;
    }

    return GLOBAL_PROTOCOL_ERR_NO_ERROR;
}

AccountManager::AccountManager()
    :   m_idCounter(0),
        m_lastTime(0)
{

}

User::ID AccountManager::generateUserID()
{
    User::ID v = 0;
    assert(sizeof (User::ID) == 8);

    static long long lastTime;
    static int counter = 0;

    std::lock_guard<std::recursive_mutex> g(m_lock);

    QDateTime now = QDateTime::currentDateTime();
    if (now.toMSecsSinceEpoch() == lastTime|| now.toMSecsSinceEpoch() < lastTime)
    {
        now = QDateTime::fromMSecsSinceEpoch(lastTime);
        if (counter + 1 > 8191)
        {
            counter = 0;
            now = now.addMSecs(1);
        }
        else
            ++counter;
    }
    else
    {
        lastTime = now.toMSecsSinceEpoch();
        counter = 0;
    }

    v = 0 & 255; //8 bit
    v <<= 6;
    v |= now.date().year() & 63;    // 6 bits
    v <<= 4;
    v |= now.date().month();   //4 bits
    v <<= 5;
    v |= now.date().day();   //5 bits
    v <<= 5;
    v |= now.time().hour();     //5 bits
    v <<= 6;
    v |= now.time().minute();   //6 bits
    v <<= 6;
    v |= now.time().second();   //6 bits
    v <<= 10;
    v |= now.time().msec();   //10 bits
    v <<= 13;

    v |= counter & 8191;  //13 bits

    return v;

}

std::string AccountManager::encodePassword(const std::string &account, const std::string &password)
{
    return QCryptographicHash::hash((account + ":" + password).c_str(),
                            QCryptographicHash::Md5).toHex().toStdString();
}
