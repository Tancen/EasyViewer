#if  defined(WIN32) || defined(WIN64)
#include "EasyIOTCPClient_win.h"
#include "EasyIOEventLoop_win.h"
#include "EasyIOContext_win.h"
#include "EasyIOError.h"
#include <mstcpip.h>
#include <mswsock.h>
#include <WinBase.h>
#include <functional>

using namespace EasyIO::TCP;
using namespace std::placeholders;

Client::Client(IEventLoopPtr worker)
    :   m_worker(worker),
        m_connecting(false),
        m_detained(0),
        m_connectEx(nullptr)
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
    std::lock_guard<std::recursive_mutex> guard(m_lock);
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
        {
            break;
        }

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.S_un.S_addr = INADDR_ANY;
        addr.sin_port = 0;
        if (bind(m_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
        {
            break;
        }

        int err = 0;
        if (!((EventLoop*)m_worker.get())->attach(m_handle, err))
        {
            break;
        }

        if (!m_connectEx)
        {
            GUID guid = WSAID_CONNECTEX;
            DWORD numBytes;

            if(WSAIoctl(m_handle, SIO_GET_EXTENSION_FUNCTION_POINTER,
                &guid, sizeof(guid), &m_connectEx,
                sizeof(m_connectEx), &numBytes, NULL, NULL) == SOCKET_ERROR)
            {
                break;
            }
        }

        Context *ctx = new Context(Context::INBOUND);
        ctx->onDone = [this](Context* ctx, size_t)
        {
            {
                std::lock_guard g(m_lock);
                m_connected = true;
                m_connecting = false;

                setsockopt(m_handle, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0);
                updateEndPoint();
            }

            if (onConnected)
                this->onConnected(this);

            m_lock.lock();
            if (m_disconnecting)
            {
                disconnect0(true, true, Error::STR_FORCED_CLOSURE);
            }
            else
            {
                decreasePostCount();
                m_lock.unlock();
            }

            ctx->decrease();
        };

        ctx->onError = [this](Context* ctx, int err)
        { 
            closesocket(m_handle);
            m_handle = INVALID_SOCKET;
            ++m_detained;
            decreasePostCount();
            m_connecting = false;

            if (onConnectFailed)
                this->onConnectFailed(this, Error::formatError(err));
            --m_detained;
            ctx->decrease();
        };

        {
            sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.S_un.S_addr = inet_addr(host.c_str());
            addr.sin_port = htons(port);

            increasePostCount();
            m_connecting = true;
            if(!((LPFN_CONNECTEX)m_connectEx)(m_handle, (sockaddr*)&addr, sizeof(addr), NULL, 0, NULL, ctx))
            {
                int err = GetLastError();
                if (err != ERROR_IO_PENDING)
                {
                    decreasePostCount();
                    break;
                }
            }
        }

        return;
    }
    while (0);

    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
    ++m_detained;
    m_connecting = false;
    if (onConnectFailed)
        this->onConnectFailed(this, Error::formatError(WSAGetLastError()));
    --m_detained;
}

void Client::disconnect()
{
    std::lock_guard g(m_lock);
    if (m_connected)
        Connection::disconnect();
    else
        closesocket(m_handle);
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
            if (!m_connected && !m_detained && !m_connecting && !m_countPost)
                break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    } while (true);
}

#endif
