#include "Global/Component/Logger/Logger.h"
#include "ServerBase.h"
#include <cinttypes>
#include "Global/Define.h"
#include "Global/Protocol/Protocol.h"
#include "RequestProcessor.h"

ServerBase::ServerBase(unsigned numWorkers)
    :   m_server(EasyIO::TCP::Server::create(numWorkers)),
        m_exit(false)
{
    m_server->onConnected = [this](EasyIO::TCP::IConnection *con)
    {
        Logger::info("%s:%d - %s:%d connected",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort());
        con->enableKeepalive(5000, 10000);
        con->setLinger(1, 0);
        std::pair<UserConnectionPtr, std::string> ret = makeUserConnection(con);
        if (!ret.first.get())
        {
            Logger::info("%s:%d - %s:%d make user connection failed: %s",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort(), ret.second.c_str());
            con->disconnect();
            return;
        }

        addConnection(ret.first);

        EasyIO::ByteBuffer data;
        con->recv(data);

        whenConnected(ret.first);
    };
    m_server->onDisconnected = [this](EasyIO::TCP::IConnection *con, const std::string& reason)
    {
        UserConnectionPtr ret = getUserConnection(con);
        assert(ret.get());
        Logger::info("%s:%d - %s:%d disconnected[%s]",  __PRETTY_FUNCTION__, __LINE__, con->peerIP().c_str(), con->peerPort(), reason.c_str());
        whenDisconnected(ret);
        removeConnection(ret);

    };
    m_server->onBufferReceived = [this](EasyIO::TCP::IConnection *con, EasyIO::ByteBuffer data)
    {
        UserConnectionPtr ret = getUserConnection(con);
        assert(ret.get());

        whenReceivedData(ret, data.uReadableBytes(), data.numReadableBytes());

        data.clear();
        con->recv(data);
    };

    m_thread = std::thread(std::bind(&ServerBase::exec, this));
}

ServerBase::~ServerBase()
{
    m_exit = true;
    if (m_thread.joinable())
        m_thread.join();
    close();
}

std::pair<UserConnectionPtr, std::string> ServerBase::makeUserConnection(EasyIO::TCP::IConnection *con)
{
    std::pair<UserConnectionPtr, std::string> ret = UserConnection::create(con->share(), m_privateKey,
                                                    getReceivedDataHandlerFactory());
    if (ret.first)
        con->bindUserdata(ret.first.get());

    return ret;
}

UserConnectionPtr ServerBase::getUserConnection(EasyIO::TCP::IConnection *con)
{
    UserConnectionPtr ret;
    UserConnection* p = (UserConnection*)con->userdata();
    if (p)
        ret = p->shared_from_this();
    return ret;
}

void ServerBase::whenConnected(UserConnectionPtr con)
{

}

void ServerBase::whenDisconnected(UserConnectionPtr con)
{

}

void ServerBase::whenReceivedData(UserConnectionPtr con, const unsigned char* data, size_t len)
{
    con->handleReceivedData(data, len);
}

void ServerBase::exec()
{
    QDateTime tCheckNotLogin;
    while (!m_exit)
    {
        QDateTime now = QDateTime::currentDateTime();
        if (tCheckNotLogin.addSecs(60) <= now)
        {
            m_lock.lock();
            auto connections = m_connections;
            m_lock.unlock();
            for (auto it : m_connections)
            {
                if (!it->getBindingUser()
                    && it->creatingTime().addSecs(60) < now)
                    it->disconnect();
            }
            tCheckNotLogin = now;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

bool ServerBase::open(unsigned short port, const std::string& privateKey)
{
    m_privateKey = privateKey;
    return m_server->open(port);
}

void ServerBase::close()
{
    m_server->close();
}

void ServerBase::addConnection(UserConnectionPtr con)
{
    m_lock.lock();
    bool b = m_connections.insert(con).second;
    assert(b);
    m_lock.unlock();
}

void ServerBase::removeConnection(UserConnectionPtr con)
{
    m_lock.lock();
    size_t n = m_connections.erase(con);
    assert(n);
    m_lock.unlock();
}

void ServerBase::kickout(User::ID userId)
{
    std::lock_guard g(m_lock);

    for (UserConnectionPtr ptr : m_connections)
    {
        auto user = ptr->getBindingUser();
        if (user && user->id == userId)
            ptr->disconnect();
    }
}

void ServerBase::kickout(const std::string& ip)
{
    std::lock_guard g(m_lock);

    for (UserConnectionPtr ptr : m_connections)
    {
        if (ptr->peerIP() == ip)
            ptr->disconnect();
    }
}
