#ifndef ACCOUNTMANAGER_H
#define ACCOUNTMANAGER_H

#include <QtSql/QSqlDatabase>
#include "Account.h"
#include <mutex>
#include <vector>

class AccountManager
{
public:
    static bool init(const std::string& dbFileDir);
    static AccountManager* share();
    static void release();
    ~AccountManager();

    int listAccounts(const std::string& filter, std::vector<Account>& dst);

    bool checkAdminPassword(const std::string& password);
    int updateAdminPassword(const std::string& oldPassword, const std::string& temporaryPassword);
    void updateAdminPassword(const std::string& password);

    int addAccount(const std::string& account, const std::string password, User::ID& id);
    int getAccount(const std::string& account, Account& info);
    int getAccount(User::ID userId, Account& info);
    int enableAccount(const std::string& account);
    int disableAccount(const std::string& account);
    int deleteAccount(const std::string& account);
    int updatePassword(const std::string& account, const std::string& password);
    bool checkPassword(const Account& account, const std::string& password);

private:
    int setAccountDisabled(const std::string& account, bool disabled);

private:
    AccountManager();
    User::ID generateUserID();

    std::string encodePassword(const std::string& account, const std::string& password);

private:
    std::recursive_mutex m_lock;
    std::string m_adminPassword;

    QSqlDatabase m_db;

    unsigned m_idCounter;
    int64_t m_lastTime;
    static AccountManager* s_this;
};

#endif // ACCOUNTMANAGER_H
