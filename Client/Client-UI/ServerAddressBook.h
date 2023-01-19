#ifndef SERVERADDRESSBOOK_H
#define SERVERADDRESSBOOK_H

#include <string>
#include <list>
#include "Global/Define.h"
#include <QSqlDatabase>

struct ServerAddress
{
    std::string name;
    std::string host;
    unsigned short port;
    std::string account;
    std::string password;
    std::string publicKey;
    bool activated;
};

struct PartnerAddress
{
    std::string alias;
    User::ID userId;
    std::string authString;
};

class ServerAddressBook
{
public:
    static ServerAddressBook* share();
    static bool init();

    bool addServerAddress(const std::string& name, const std::string& host, unsigned short port,
                    const std::string& account, const std::string& password, const std::string& publicKey);
    bool removeServerAddress(const std::string& host, unsigned short port);
    bool updateServerAddress(const std::string& name, const std::string& host, unsigned short port,
                        const std::string& account, const std::string& password, const std::string& publicKey);
    bool listServerAddress(std::list<ServerAddress>& dst);
    bool activateServerAddress(const std::string& host, unsigned short port);

    bool addPartnerAddress(const std::string& host, unsigned short port,
            const std::string& alias, User::ID userId, const std::string& authString);
    bool updatePartnerAddress(const std::string& host, unsigned short port,
            const std::string& alias, User::ID userId, const std::string& authString);
    bool removePartnerAddress(const std::string& host, unsigned short port, User::ID userId);
    bool listPartnerAddress(const std::string& host, unsigned short port, std::list<PartnerAddress>& dst);
    bool getPartnerAddress(const std::string& host, unsigned short port, User::ID userId, PartnerAddress& dst);

private:
    ServerAddressBook();

    bool initDB();
    bool initServerAddressTable();
    bool initServerPartnerAddress();

private:
    QSqlDatabase m_db;

    static ServerAddressBook s_this;
};

#endif // SERVERADDRESSBOOK_H
