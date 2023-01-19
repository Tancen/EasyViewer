#if  defined(WIN32) || defined(WIN64)
#include "EasyIOUDPSocket_win.h"
#include <mstcpip.h>
#include <mswsock.h>
#include <WinBase.h>
#include <thread>

using namespace EasyIO::UDP;
using namespace std::placeholders;

EasyIO::UDP::Socket::Context1::Context1(
    EasyIO::ByteBuffer buffer, Flag flag)
    :   ::EasyIO::Context(buffer, flag),
        m_addrLen(sizeof(sockaddr_in))
{

}

int *EasyIO::UDP::Socket::Context1::addrLen()
{
    return &m_addrLen;
}

sockaddr_in *EasyIO::UDP::Socket::Context1::addr()
{
    return &m_addr;
}

Socket::Socket()
    :   Socket(INVALID_SOCKET)
{

}

Socket::Socket(SOCKET sock)
    : m_handle(sock),
      m_opened(false),
      m_closing(false),
      m_numBytesPending(0),
      m_userdata(nullptr),
      m_localPort(0),
      m_countPost(0)
{

}

Socket::~Socket()
{
    close();
}

ISocketPtr Socket::share()
{
    return std::dynamic_pointer_cast<ISocket>(this->shared_from_this());
}

SOCKET Socket::handle()
{
    return m_handle;
}

bool Socket::opened()
{
    return m_opened;
}

void Socket::close()
{
    do
    {
        std::lock_guard g(m_lock);
        if (m_handle == INVALID_SOCKET)
            break;

        m_closing = true;
        closesocket(m_handle);
        m_handle = INVALID_SOCKET;
    }
    while(0);
    close0(false);

    do
    {
        {
            std::lock_guard  g(m_lock);
            if (!m_countPost && !m_opened && !m_closing)
                break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    } while (true);
}

void Socket::close0(bool requireDecrease)
{
    ISocketPtr holder;
    if (!this->weak_from_this().expired())
        holder = this->shared_from_this();;

    do
    {
        {
            std::lock_guard g(m_lock);
            if (!m_opened || !m_closing)
                break;

            if (requireDecrease)
            {
                if (m_countPost - 1 > 0)
                    break;
            }
            else if (m_countPost)
                break;

            m_opened = false;
            m_numBytesPending = 0;
            cleanTasks(m_tasksSend);
            cleanTasks(m_tasksRecv);
        }
        {
            std::lock_guard  g(m_lock);
            m_closing = false;
        }
    } while (0);

    if (requireDecrease)
        decreasePostCount();
}

void Socket::send(const std::string& ip, unsigned short port, ByteBuffer buffer)
{
    if (!buffer.numReadableBytes())
        return;

    {
        std::lock_guard g(m_lock);
        if (!m_opened)
            return;

        Context1* ctx = new Context1(buffer, Context1::OUTBOUND);
        ctx->addr()->sin_family = AF_INET;
        ctx->addr()->sin_addr.s_addr = inet_addr(ip.c_str());
        ctx->addr()->sin_port = htons(port);
        ctx->onDone = std::bind(&Socket::whenSendDone, this, _1, _2);
        ctx->onError = std::bind(&Socket::whenError, this, _1, _2);

        m_numBytesPending += buffer.numReadableBytes();
        bool isEmpty = addTask(ctx, m_tasksSend);
        if (isEmpty)
        {
            send0();
        }
        ctx->decrease();
    }
}

int Socket::send0()
{
    int ret = doFirstTask(m_tasksSend, [this](Context1* ctx)
    {
        DWORD flags = 0;
        int ret = WSASendTo(m_handle, ctx->WSABuf(), 1,  NULL, flags, (sockaddr*)(ctx->addr()), *(ctx->addrLen()), ctx, NULL);
        if (ret == SOCKET_ERROR)
        {
            int err = GetLastError();
            if(err != WSA_IO_PENDING)
                return err;
        }
        return 0;
    });
    return ret;
}

void Socket::recv(ByteBuffer buffer)
{
    buffer.ensureWritable(4096);

    {
        std::lock_guard g(m_lock);
        if (!m_opened)
            return;

        Context1* ctx = new Context1(buffer, Context1::INBOUND);
        ctx->onDone = std::bind(&Socket::whenRecvDone, this, _1, _2);
        ctx->onError = std::bind(&Socket::whenError, this, _1, _2);

        bool isEmpty = addTask(ctx, m_tasksRecv);
        if (isEmpty)
        {
            recv0();
        }
        ctx->decrease();
    }
}

int Socket::recv0()
{
    int ret = doFirstTask(m_tasksRecv, [this](Context1* ctx)
    {
        DWORD flags = 0;
        int ret = WSARecvFrom(m_handle, ctx->WSABuf(), 1,  NULL, &flags,
                          (sockaddr*)(ctx->addr()), ctx->addrLen(), ctx, NULL);
        if (ret == SOCKET_ERROR)
        {
            int err = GetLastError();
            if(err != WSA_IO_PENDING)
                return err;
        }
        return 0;
    });
    return ret;
}


size_t Socket::numBytesPending()
{
    return m_numBytesPending;
}

const std::string &Socket::localIP() const
{
    return m_localIP;
}

unsigned short Socket::localPort() const
{
    return m_localPort;
}

bool Socket::updateEndPoint()
{
    int len = sizeof(sockaddr_in);
    sockaddr_in addrLocal;

    if(getsockname(m_handle, (sockaddr*)&addrLocal, &len) == SOCKET_ERROR)
        return false;

    m_localIP = inet_ntoa(addrLocal.sin_addr);
    m_localPort = ntohs(addrLocal.sin_port);
    return true;
}

void Socket::bindUserdata(void *userdata)
{
    m_userdata = userdata;
}

void *Socket::userdata() const
{
    return m_userdata;
}

bool Socket::addTask(Context1 *ctx, std::list<Context1 *> &dst)
{
    bool ret = dst.empty();
    ctx->increase();
    dst.push_back(ctx);
    return ret;
}

int Socket::doFirstTask(std::list<Context1*>& tasks, std::function<int(Context1*)> transmitter)
{
    std::lock_guard g(m_lock);

    if (tasks.empty())
        return 0;

    increasePostCount();
    Context1* ctx = tasks.front();
    int ret = transmitter(ctx);
    if (ret)
        decreasePostCount();
    return true;
}

void Socket::popFirstTask(std::list<Context1*>& tasks)
{
    std::lock_guard g(m_lock);
    if (tasks.empty())
        return;
    Context1* ctx = tasks.front();
    tasks.pop_front();
    ctx->decrease();
}

void Socket::cleanTasks(std::list<Context1 *> &tasks)
{
    std::lock_guard g(m_lock);

    while (!tasks.empty())
        popFirstTask(tasks);
}

void Socket::whenSendDone(Context *ctx, size_t increase)
{
    if (ctx->finished())
        popFirstTask(m_tasksSend);

    send0();

    std::lock_guard g(m_lock);
    if (m_closing)
    {
        close0(true);
    }
    else
    {
        decreasePostCount();
    }
}
void Socket::whenRecvDone(Context *ctx, size_t increase)
{
    if (increase)
    {
        if (onBufferReceived)
        {
            Context1* ctx1= (Context1*)ctx;
            std::string ip = inet_ntoa(ctx1->addr()->sin_addr);
            unsigned short port = ntohs(ctx1->addr()->sin_port);
            onBufferReceived(this, ip, port, ctx->buffer());
        }

        popFirstTask(m_tasksRecv);
    }

    recv0();
    std::lock_guard g(m_lock);
    if (m_closing)
    {
        close0(true);
    }
    else
    {
        decreasePostCount();
    }
}

void Socket::whenError(Context *context, int err)
{
    context->increaseProgress(0);
}

int Socket::increasePostCount()
{
    int ret = ++m_countPost;
    return ret;
}

int Socket::decreasePostCount()
{
    int ret = --m_countPost;
    return ret;
}

#endif

