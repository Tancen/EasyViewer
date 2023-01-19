#include "BlackList.h"

#define DURING_MINUTE       5
#define DURING_HOUR         1

#define TO_KEY1(__ip) (__ip)
#define TO_KEY2(__userId) (std::to_string(__userId))

#define TYPE_IP_ADDRESS         1
#define TYPE_ACCOUNT            2


BlackList BlackList::s_this;

BlackList *BlackList::share()
{
    return &s_this;
}

void BlackList::addIPAddress(const std::string &ip, unsigned duration)
{
    QDateTime t = QDateTime::currentDateTime();

    Status status;
    status.type = TYPE_IP_ADDRESS;
    status.target = ip;
    status.m.count = 1;
    status.h.count = 1;
    status.blocked = true;
    status.blockUntil = t.toSecsSinceEpoch() + duration;
    fillForTime(t, status.m.forTime, status.h.forTime);

    std::unique_lock g(m_mutex);
    auto pair = m_statuses.insert({TO_KEY1(ip), status});
    Status& _status = pair.first->second;
    _status.blocked = true;
    _status.blockUntil = status.blockUntil;
}

void BlackList::increaseIPAddress(const std::string &ip)
{
    QDateTime t = QDateTime::currentDateTime();

    Status status;
    status.type = TYPE_IP_ADDRESS;
    status.target = ip;
    status.m.count = 1;
    status.h.count = 1;
    fillForTime(t, status.m.forTime, status.h.forTime);

    increase(TO_KEY1(ip), status);
}

bool BlackList::checkIPAddress(const std::string &ip)
{
    return check(TO_KEY1(ip));
}

void BlackList::removeIPAddress(const std::string &ip)
{
    remove(TO_KEY1(ip));
}

std::list<BlackList::BlockedIPAddress> BlackList::listIPAddresses()
{
    std::list<BlockedIPAddress> ret;
    std::shared_lock g(m_mutex);
    for (auto& it : m_statuses)
    {
        if (it.second.type == TYPE_IP_ADDRESS && it.second.blocked)
            ret.push_back({std::get<std::string>(it.second.target), it.second.blockUntil});
    }
    return ret;
}

void BlackList::addAccount(const Account& account, unsigned duration)
{
    QDateTime t = QDateTime::currentDateTime();

    Status status;
    status.type = TYPE_IP_ADDRESS;
    status.target = account;
    status.m.count = 1;
    status.h.count = 1;
    status.blocked = true;
    status.blockUntil = t.toSecsSinceEpoch() + duration;
    fillForTime(t, status.m.forTime, status.h.forTime);

    std::unique_lock g(m_mutex);
    auto pair = m_statuses.insert({TO_KEY2(account.id), status});
    Status& _status = pair.first->second;
    _status.blocked = true;
    _status.blockUntil = status.blockUntil;
}

void BlackList::increaseAccount(const Account& account)
{
    QDateTime t = QDateTime::currentDateTime();

    Status status;
    status.type = TYPE_ACCOUNT;
    status.target = account;
    status.m.count = 1;
    status.h.count = 1;
    fillForTime(t, status.m.forTime, status.h.forTime);

    increase(TO_KEY2(account.id), status);
}

bool BlackList::checkAccount(User::ID id)
{
    return check(TO_KEY2(id));
}

void BlackList::removeAccount(User::ID id)
{
    remove(TO_KEY2(id));
}

std::list<BlackList::BlockedAccount> BlackList::listAccounts()
{
    std::list<BlockedAccount> ret;
    std::shared_lock g(m_mutex);
    for (auto& it : m_statuses)
    {
        if (it.second.type == TYPE_ACCOUNT && it.second.blocked)
        {
            auto account = std::get<Account>(it.second.target);
            ret.push_back({account.id, account.account, it.second.blockUntil});
        }
    }
    return ret;
}

BlackList::BlackList()
{

}

void BlackList::increase(const std::string &key, const Status& status)
{
    long long m, h;
    fillForTime(QDateTime::currentDateTime(), m, h);

    std::unique_lock g(m_mutex);
    auto pair = m_statuses.insert({key, status});
    Status& _status = pair.first->second;
    if (!pair.second)
    {
        if (_status.m.forTime == m)
        {
            _status.m.count++;

            if (_status.m.count >= 5)
            {
                _status.blocked = true;
                _status.blockUntil = QDateTime::currentDateTime().addSecs(300).toSecsSinceEpoch();
            }
        }
        else
        {
            _status.m.forTime = m;
            _status.m.count = 1;
        }

        if (_status.h.forTime == h)
        {
            _status.h.count++;
            if (_status.h.count >= 5)
            {
                _status.blocked = true;
                _status.blockUntil = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();
            }

        }
        else
        {
            _status.h.forTime = h;
            _status.h.count = 1;
        }
    }
}

bool BlackList::check(const std::string &key)
{
    long long now = QDateTime::currentSecsSinceEpoch();

    std::shared_lock g(m_mutex);
    auto it = m_statuses.find(key);
    if (it != m_statuses.end())
    {
        if (it->second.blocked && it->second.blockUntil >= now)
            return false;
    }

    return true;
}

void BlackList::remove(const std::string &key)
{
    std::unique_lock g(m_mutex);
    m_statuses.erase(key);
}

void BlackList::fillForTime(QDateTime t, long long& m, long long& h)
{
    t.setTime(QTime(t.time().hour(), t.time().minute() / DURING_MINUTE * DURING_MINUTE, 0, 0));
    m = t.toSecsSinceEpoch();

    t.setTime(QTime(t.time().hour() / DURING_HOUR * DURING_HOUR, 0, 0, 0));
    h = t.toSecsSinceEpoch();
}
