#ifndef BLACKLIST_H
#define BLACKLIST_H

#include "Account.h"
#include <string>
#include <QDateTime>
#include <shared_mutex>
#include <map>
#include <variant>
#include <functional>


class BlackList
{
public:
    struct BlockedAccount
    {
        User::ID userId;
        std::string account;
        long long until;
    };

    struct BlockedIPAddress
    {
        std::string ip;
        long long until;
    };

private:
    struct Status
    {
        struct Count
        {
            int count = 0;
            long long forTime = 0;
        };

        int type;
        std::variant<std::string, Account> target;
        Count m;
        Count h;
        bool blocked = false;
        long long blockUntil = 0;
    };

public:
    static BlackList* share();

    void addIPAddress(const std::string& ip, unsigned duration);
    void increaseIPAddress(const std::string& ip);
    bool checkIPAddress(const std::string& ip);
    void removeIPAddress(const std::string& ip);
    std::list<BlockedIPAddress> listIPAddresses();

    void addAccount(const Account& account, unsigned duration);
    void increaseAccount(const Account& account);
    bool checkAccount(User::ID id);
    void removeAccount(User::ID id);
    std::list<BlockedAccount> listAccounts();

private:
    BlackList();

    void increase(const std::string& key, const Status& status);
    bool check(const std::string& key);
    void remove(const std::string& key);

    void fillForTime(QDateTime t, long long& m, long long& h);

private:
    std::shared_mutex m_mutex;
    std::map<std::string, Status> m_statuses;

    static BlackList s_this;
};

#endif // BLACKLIST_H
