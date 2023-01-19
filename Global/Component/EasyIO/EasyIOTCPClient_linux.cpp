#ifdef __linux__
#include <arpa/inet.h>
#include "EasyIOTCPClient_linux.h"
#include "EasyIOEventLoop_linux.h"
#include "EasyIOError.h"
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

using namespace EasyIO::TCP;
using namespace std::placeholders;

Client::Client(IEventLoopPtr worker)
    : Connection(static_cast<EventLoop*>(worker.get())),
      m_worker(worker),
      m_connecting(false),
      m_canceling(false),
      m_detained(0)
{

}

IConnectionPtr Client::makeHolder()
{
    //don't hold this, avoid deadlock in the destructor of worker
    return IConnectionPtr();
}

IClientPtr Client::create()
{
    IEventLoopPtr worker = EventLoop::create();
    return create(worker);
}

IClientPtr Client::create(IEventLoopPtr worker)
{
    IClientPtr ret;
    if (!dynamic_cast<EventLoop*>(worker.get()))
        return ret;

    ret.reset(new Client(worker));
    return ret;
}

Client::~Client()
{
    Client::syncDisconnect();
}

void Client::connect(const std::string& host, unsigned short port)
{
    std::lock_guard  g(m_lock);
    if (m_connected || m_connecting)
    {
        if (onConnectFailed)
            this->onConnectFailed(this, "connected or connecting");
        return;
    }

    do
    {
        m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_handle == INVALID_SOCKET)
            break;

        int flag = fcntl(m_handle, F_GETFL, 0);
        if (-1 == flag )
            break;
        fcntl(m_handle, F_SETFL, flag | O_NONBLOCK);

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(host.c_str());
        addr.sin_port = htons(port);

        if(::connect(m_handle, (sockaddr*)&addr, sizeof(addr)) != 0 && errno != EINPROGRESS)
            break;

        m_context.events = EPOLLOUT | EPOLLONESHOT;
        m_context.setCallback([this](uint32_t events)
        {
            int err = 0;
            do
            {
                socklen_t l = sizeof(err);
                if (getsockopt(m_handle, SOL_SOCKET, SO_ERROR, &err, &l))
                {
                    err = errno;
                    break;
                }
                if (err)
                    break;

                if (!(events & EPOLLOUT))
                    break;

                {
                    std::lock_guard g(m_lock);
                    if (m_canceling)
                        break;

                    EventLoop* w = (EasyIO::EventLoop*)(m_worker.get());
                    m_context.events = 0;
                    w->modify(m_handle, &m_context);

                    m_connected = true;
                    m_connecting = false;
                    updateEndPoint();
                }

                if (this->onConnected)
                    this->onConnected(this);

                m_context.setCallback(std::bind(&Client::handleEvents, this, _1));

                return;
            } while (false);

            std::string reason = Error::formatError(err);
            {
                std::lock_guard g(m_lock);
                EventLoop* w = (EasyIO::EventLoop*)(m_worker.get());
                w->remove(m_handle, &m_context);
                closesocket(m_handle);
                ++m_detained;
                m_connecting = false;
                m_handle = INVALID_SOCKET;
                if (m_canceling)
                    reason = Error::STR_FORCED_CLOSURE;
                m_canceling = false;
            }

            if (onConnectFailed)
                onConnectFailed(this, reason);
            --m_detained;
        });

        m_connecting = true;
        EventLoop* w = (EasyIO::EventLoop*)(m_worker.get());
        w->add(m_handle, &m_context);
        return;
    }
    while(0);

    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    ++m_detained;
    m_connecting = false;
    if (onConnectFailed)
        this->onConnectFailed(this, Error::formatError(errno));
    --m_detained;
}

void Client::disconnect()
{
    std::lock_guard g(m_lock);
    if (m_connected)
        Connection::disconnect();
    else
    {
        m_canceling = true;
        shutdown(m_handle, SHUT_RDWR);
    }
}

bool EasyIO::TCP::Client::connecting()
{
    return m_connecting;
}

void Client::syncDisconnect()
{
    Client::disconnect();
    do
    {
        {
            std::lock_guard  g(m_lock);
            if (!m_connected && !m_detained && !m_connecting)
                break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    } while (true);
}

#endif



