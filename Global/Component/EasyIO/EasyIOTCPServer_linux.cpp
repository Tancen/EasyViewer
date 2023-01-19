#ifdef __linux__
#include "EasyIOTCPServer_linux.h"
#include "EasyIOEventLoop_linux.h"
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

using namespace EasyIO::TCP;
using namespace std::placeholders;

Server::Server(EventLoopGroupPtr workers)
    :   m_workers(workers),
        m_opened(false)
{

}

IServerPtr Server::create(unsigned numWorkers)
{
    std::vector<IEventLoopPtr> ws;
    for (unsigned i = 0; i < numWorkers; i++)
    {
        IEventLoopPtr w = EventLoop::create();
        if (!w.get())
            return IServerPtr();
        ws.push_back(w);
    }

    return create(EventLoopGroupPtr(new EventLoopGroup(ws)));
}

IServerPtr Server::create(EasyIO::EventLoopGroupPtr workers)
{
    const auto& ws = workers->getEventLoopGroup();
    for (const auto & it :ws)
    {
        if (!dynamic_cast<EventLoop*>(it.get()))
            return IServerPtr();
    }

    return IServerPtr(new Server(workers));
}

Server::~Server()
{
    close();
}

bool Server::open(unsigned short port, unsigned int backlog)
{
    std::lock_guard<std::mutex> guard(m_mutex);

    if (m_opened)
        return false;

    do
    {
        m_acceptor.reset(new Acceptor());
        m_acceptor->onAccepted = std::bind(&Server::addConnection, this, _1);
        int err = 0;
        if(!m_acceptor->accept(port, backlog, err))
        {
            break;
        }

        m_opened = true;
        return true;
    } while(0);

    m_acceptor.reset();
    return false;
}

void Server::close()
{
    std::lock_guard<std::mutex> guard(m_mutex);
    m_acceptor.reset();

    m_lockConnections.lock();
    auto connections = m_connections;
    m_lockConnections.unlock();

    for (auto it : connections)
        it.second->disconnect();

    while (1)
    {
        {
            std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
            if (m_connections.empty())
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    m_opened = false;
}

bool Server::opened()
{
    return m_opened;
}

void Server::addConnection(SOCKET sock)
{
    int flag = fcntl(sock, F_GETFL, 0);
    if (-1 == flag )
    {
        closesocket(sock);
        return;
    }
    fcntl(sock, F_SETFL, flag | O_NONBLOCK);

    EventLoop* w = (EventLoop*)m_workers->getNext();

    IConnectionPtr con(new Connection(w, sock, true));
    con->updateEndPoint();
    con->onBufferReceived = onBufferReceived;
    con->onDisconnected = std::bind(&Server::removeConnection, this, _1, _2);


    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.insert(std::make_pair(con.get(), con));
    }

    if (onConnected)
        onConnected(con.get());
}

void Server::removeConnection(IConnection *con, const std::string& reason)
{
    if (onDisconnected)
        onDisconnected(con, reason);

    {
        std::lock_guard<std::recursive_mutex> lockGuard(m_lockConnections);
        m_connections.erase(con);
    }
}

#endif
