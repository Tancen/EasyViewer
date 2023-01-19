#include "ServerAddressBook.h"
#include "Global/Component/Logger/Logger.h"
#include <qsqlquery.h>
#include <qsqlerror.h>
#include <qvariant.h>
#include <qcoreapplication.h>
#include <QDir>

#define DB_DIR                      (qApp->applicationDirPath() + "/data")
#define DB_PATH                     (DB_DIR + "/AddressBook.db")
#define TABLE_SERVER_ADDRESS_BOOK   "server_address_book"
#define TABLE_PARTNER_ADDRESS_BOOK  "partner_address_book"

ServerAddressBook ServerAddressBook::s_this;

ServerAddressBook *ServerAddressBook::share()
{
    return &s_this;
}

bool ServerAddressBook::init()
{
    do
    {
        if (!s_this.initDB() || !s_this.initServerAddressTable() || !s_this.initServerPartnerAddress())
            break;

        return true;
    } while (0);

    s_this.m_db.close();
    return false;
}

bool ServerAddressBook::addServerAddress(const std::string &name, const std::string &host, unsigned short port,
        const std::string& account, const std::string& password, const std::string& publicKey)
{
    QString sql = "INSERT INTO " TABLE_SERVER_ADDRESS_BOOK " (name, host, port, account, password, public_key) VALUES(?, ?, ?, ?, ?, ?)";
    QSqlQuery q(m_db);

    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, name.c_str());
    q.bindValue(1, host.c_str());
    q.bindValue(2, port);
    q.bindValue(3, account.c_str());
    q.bindValue(4, password.c_str());
    q.bindValue(5, publicKey.c_str());
    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}

bool ServerAddressBook::removeServerAddress(const std::string &host, unsigned short port)
{
    m_db.transaction();
    do
    {
        QString sql = "DELETE FROM " TABLE_PARTNER_ADDRESS_BOOK
                " WHERE server_id = (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? AND port = ? )";
        QSqlQuery q(m_db);
        if (!q.prepare(sql))
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }
        q.bindValue(0, host.c_str());
        q.bindValue(1, port);
        if (!q.exec())
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }

        sql = "DELETE FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? AND port = ?";
        if (!q.prepare(sql))
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }
        q.bindValue(0, host.c_str());
        q.bindValue(1, port);
        if (!q.exec())
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }
        if (!m_db.commit())
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, m_db.lastError().text().toStdString().c_str());
            break;
        }
        return true;
    } while (0);

    m_db.rollback();
    return false;
}

bool ServerAddressBook::updateServerAddress(const std::string &name, const std::string &host, unsigned short port,
                    const std::string& account, const std::string& password, const std::string& publicKey)
{
    QString sql = "UPDATE " TABLE_SERVER_ADDRESS_BOOK " SET name = ?, account = ?, password = ?, public_key = ? WHERE host = ? AND port = ?";
    QSqlQuery q(m_db);

    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, name.c_str());
    q.bindValue(1, account.c_str());
    q.bindValue(2, password.c_str());
    q.bindValue(3, publicKey.c_str());
    q.bindValue(4, host.c_str());
    q.bindValue(5, port);

    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}

bool ServerAddressBook::listServerAddress(std::list<ServerAddress> &dst)
{
    QString sql = "SELECT name, host, port, account, password, public_key, activated FROM " TABLE_SERVER_ADDRESS_BOOK;
    QSqlQuery q(m_db);

    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    while (q.next())
    {
        dst.push_back({q.value(0).toString().toStdString(),
                        q.value(1).toString().toStdString(),
                        (unsigned short)q.value(2).toUInt(),
                        q.value(3).toString().toStdString(),
                        q.value(4).toString().toStdString(),
                        q.value(5).toString().toStdString(),
                        q.value(6).toInt() != 0
                      });
    }

    return true;
}

bool ServerAddressBook::activateServerAddress(const std::string &host, unsigned short port)
{
    if(!m_db.transaction())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, m_db.lastError().text().toStdString().c_str());
        return false;
    }

    do
    {
        QString sql = "UPDATE " TABLE_SERVER_ADDRESS_BOOK " SET activated = 0";
        QSqlQuery q(m_db);
        if (!q.exec(sql))
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }
        sql = "UPDATE " TABLE_SERVER_ADDRESS_BOOK " SET activated = 1 WHERE host = ? AND port = ?";
        if (!q.prepare(sql))
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }
        q.bindValue(0, host.c_str());
        q.bindValue(1, port);
        if (!q.exec())
        {
            Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
            break;
        }

        if(!m_db.commit())
            break;

        return true;
    } while (0);
    m_db.rollback();

    return false;
}

bool ServerAddressBook::addPartnerAddress(const std::string &host, unsigned short port, const std::string &alias, User::ID userId, const std::string &authString)
{
    QString sql = " INSERT OR IGNORE INTO " TABLE_PARTNER_ADDRESS_BOOK "(server_id, alias, user_id, auth_string)"
            " VALUES ("
                " (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? AND port = ? ), ?, ?, ?"
            " )";
    QSqlQuery q(m_db);

    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, host.c_str());
    q.bindValue(1, port);
    q.bindValue(2, alias.c_str());
    q.bindValue(3, (long long)userId);
    q.bindValue(4, authString.c_str());
    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}

bool ServerAddressBook::updatePartnerAddress(const std::string &host, unsigned short port, const std::string &alias, User::ID userId, const std::string &authString)
{
    QString sql = " UPDATE " TABLE_PARTNER_ADDRESS_BOOK " SET alias = ?, auth_string = ?"
            " WHERE server_id = (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? and port = ?)"
                   " AND user_id = ?" ;

    QSqlQuery q(m_db);

    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, alias.c_str());
    q.bindValue(1, authString.c_str());
    q.bindValue(2, host.c_str());
    q.bindValue(3, port);
    q.bindValue(4, (long long)userId);
    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}

bool ServerAddressBook::removePartnerAddress(const std::string &host, unsigned short port, User::ID userId)
{
    QString sql = " DELETE FROM " TABLE_PARTNER_ADDRESS_BOOK
            " WHERE server_id = (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? and port = ?)"
                   " AND user_id = ?" ;

    QSqlQuery q(m_db);

    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, host.c_str());
    q.bindValue(1, port);
    q.bindValue(2, (long long)userId);
    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    return q.numRowsAffected() > 0;
}

bool ServerAddressBook::listPartnerAddress(const std::string& host, unsigned short port, std::list<PartnerAddress> &dst)
{
    QString sql = "SELECT alias, user_id, auth_string FROM " TABLE_PARTNER_ADDRESS_BOOK
            " WHERE server_id = (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? and port = ?)";

    QSqlQuery q(m_db);
    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, host.c_str());
    q.bindValue(1, port);

    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    while (q.next())
    {
        dst.push_back({q.value(0).toString().toStdString(),
                      q.value(1).toLongLong(),
                      q.value(2).toString().toStdString(),});
    }

    return true;
}

bool ServerAddressBook::getPartnerAddress(const std::string &host, unsigned short port, User::ID userId, PartnerAddress &dst)
{
    QString sql = "SELECT alias, user_id, auth_string FROM " TABLE_PARTNER_ADDRESS_BOOK
            " WHERE server_id = (SELECT id FROM " TABLE_SERVER_ADDRESS_BOOK " WHERE host = ? and port = ?)"
                " AND user_id = ?";

    QSqlQuery q(m_db);
    if (!q.prepare(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    q.bindValue(0, host.c_str());
    q.bindValue(1, port);
    q.bindValue(2, (long long)userId);

    if (!q.exec())
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }

    if (q.next())
    {
        dst = {
                    q.value(0).toString().toStdString(),
                    q.value(1).toLongLong(),
                    q.value(2).toString().toStdString()
              };
    }

    return true;
}

ServerAddressBook::ServerAddressBook()
{

}

bool ServerAddressBook::initDB()
{
    QDir dir;
    dir.mkpath(DB_DIR);

    m_db = QSqlDatabase::addDatabase("QSQLITE", "ServerAddressBook");
    m_db.setDatabaseName(DB_PATH);
    return m_db.open();
}

bool ServerAddressBook::initServerAddressTable()
{
    QString sql = " CREATE TABLE IF NOT EXISTS " TABLE_SERVER_ADDRESS_BOOK "("
                " id INTEGER PRIMARY KEY AUTOINCREMENT,"
                " name VARCHAR(32) NOT NULL,"
                " host VARCHAR(32) NOT NULL,"
                " port INT NOT NULL,"
                " account VARCHAR(32) NOT NULL,"
                " password VARCHAR(32) NOT NULL,"
                " public_key VARCHAR(1024) NOT NULL,"
                " activated int NOT NULL DEFAULT 0,"
                " UNIQUE (host, port)"
         " )" ;
    QSqlQuery q(m_db);
    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}

bool ServerAddressBook::initServerPartnerAddress()
{
    QString sql = " CREATE TABLE IF NOT EXISTS " TABLE_PARTNER_ADDRESS_BOOK "("
            " server_id INT NOT NULL,"
            " alias VARCHAR(32) NOT NULL,"
            " user_id BIGINT NOT NULL,"
            " auth_string VARCHAR(32) NOT NULL,"
            " UNIQUE (server_id, user_id)"
         " )" ;
    QSqlQuery q(m_db);
    if (!q.exec(sql))
    {
        Logger::error("%s:%d - %s", __PRETTY_FUNCTION__, __LINE__, q.lastError().text().toStdString().c_str());
        return false;
    }
    return true;
}
