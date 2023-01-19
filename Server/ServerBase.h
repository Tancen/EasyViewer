#ifndef SERVERBASE_H
#define SERVERBASE_H

#include "UserConnection.h"
#include <set>
#include <mutex>

class ServerBase
{
public:
    ServerBase(unsigned numWorkers = 1);
    ~ServerBase();

    bool open(unsigned short port, const std::string& privateKey);
    void close();

    void kickout(User::ID userId);
    void kickout(const std::string& ip);

protected:
    inline std::pair<UserConnectionPtr, std::string> makeUserConnection(EasyIO::TCP::IConnection* con);
    inline UserConnectionPtr getUserConnection(EasyIO::TCP::IConnection* con);

    void addConnection(UserConnectionPtr con);
    void removeConnection(UserConnectionPtr con);

    virtual void whenConnected(UserConnectionPtr  con);
    virtual void whenDisconnected(UserConnectionPtr  con);
    virtual void whenReceivedData(UserConnectionPtr  con, const unsigned char* data, size_t len);
    void exec();

    virtual UserConnection::ReceivedDataHandlerFactory getReceivedDataHandlerFactory() = 0;

protected:
    std::string m_privateKey;
    EasyIO::TCP::IServerPtr m_server;
    std::set<UserConnectionPtr> m_connections;

    std::recursive_mutex m_lock;

    bool m_exit;
    std::thread m_thread;
};

#endif // SERVERBASE_H
