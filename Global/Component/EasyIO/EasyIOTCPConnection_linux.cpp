#ifdef __linux__
#include "EasyIOTCPConnection_linux.h"
#include "EasyIOError.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <assert.h>

using namespace EasyIO::TCP;
using namespace std::placeholders;

Connection::Connection(EventLoop *worker)
    : Connection(worker, INVALID_SOCKET, false)
{

}

Connection::Connection(EventLoop *worker, SOCKET sock, bool connected)
    : m_worker(worker),
      m_handle(sock),
      m_context(std::bind(&Connection::handleEvents, this, _1)),
      m_connected(connected),
      m_disconnecting(false),
      m_localPort(0),
      m_peerPort(0),
      m_userdata(NULL),
      m_numBytesPending(0)
{
    if (m_handle != INVALID_SOCKET)
    {
        m_worker->add(m_handle, &m_context);
    }
}

Connection::~Connection()
{
    Connection::syncDisconnect();
}

IConnectionPtr Connection::share()
{
    return std::dynamic_pointer_cast<IConnection>(this->shared_from_this());
}

SOCKET Connection::handle()
{
    return m_handle;
}

bool Connection::connected()
{
    return m_connected;
}

void Connection::disconnect()
{
    disconnect(Error::STR_FORCED_CLOSURE);
}

void Connection::syncDisconnect()
{
    Connection::disconnect();
    do
    {
        {
            std::lock_guard g(m_lock);
            if (!m_connected && !m_disconnecting)
                break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    } while (true);
}

void Connection::disconnect(const std::string &reason)
{
    std::lock_guard  g(m_lock);
    if (m_disconnecting || !m_connected)
        return;
    m_disconnecting = true;
    m_context.events |= EPOLLIN;
    m_worker->modify(m_handle, &m_context);
    shutdown(m_handle, SHUT_RDWR);
    m_reason = reason;
}


void Connection::disconnect0(bool requireUnlock)
{
    IConnectionPtr holder = makeHolder();
    bool unlocked = false;
    do
    {
        {
            std::lock_guard  g(m_lock);
            if (!m_connected)
                break;

            m_worker->remove(m_handle, &m_context);
            closesocket(m_handle);
            m_handle = INVALID_SOCKET;
            m_connected = false;
            m_numBytesPending = 0;
            cleanTasks(m_tasksSend);
            cleanTasks(m_tasksRecv);
        }
        if (requireUnlock)
        {
            m_lock.unlock();
            unlocked = true;
        }

        if (onDisconnected)
            onDisconnected(this, m_reason);

        {
            std::lock_guard  g(m_lock);
            m_disconnecting = false;
        }
    } while(0);

    if (requireUnlock && !unlocked)
        m_lock.unlock();
}

void Connection::send(ByteBuffer buffer)
{
    if (!buffer.numReadableBytes())
        return;

    int err = 0;
    {
        std::lock_guard g(m_lock);
        if (!m_connected)
            return;

        int v = m_numBytesPending.fetch_add(buffer.numReadableBytes());
        assert(v >= 0);
        bool isEmpty = addTask(std::move(buffer), m_tasksSend);
        if (isEmpty)
        {
            assert(v == 0);
            err = send0();
            if (!err)
            {
                if (m_tasksSend.empty() && (m_context.events & EPOLLOUT))
                {
                    m_context.events &= ~EPOLLOUT;
                    m_worker->modify(m_handle, &m_context);
                }
            }
        }
    }
    if (err)
        disconnect(Error::formatError(err));
}

int Connection::send0()
{
    int ret = doFirstTask(m_tasksSend,
            [this](ByteBuffer buf)
            {
                do
                {
                    if (!buf.numReadableBytes())
                        break;

                    int err = ::send(m_handle, buf.readableBytes(), buf.numReadableBytes(), 0);
                    if (err == 0)
                    {
                        return Error::getSocketError(m_handle);
                    }
                    if (err < 0)
                    {
                        if (errno == EAGAIN)
                        {
                            if (!(m_context.events & EPOLLOUT))
                            {
                                m_context.events |= EPOLLOUT;
                                m_worker->modify(m_handle, &m_context);
                            }
                            return 0;
                        }
                        return errno;
                    }

                    buf.moveReaderIndex(err);
                    m_numBytesPending -= err;
                } while (true);
                return 0;
            },
            std::bind(&EasyIO::TCP::Connection::sendComplete, this, std::placeholders::_1),
            nullptr
            );

    return ret;
}

bool Connection::sendComplete(ByteBuffer buffer)
{
    return !buffer.numReadableBytes();
}

void Connection::recv(ByteBuffer buffer)
{
    buffer.ensureWritable(4096);

    int err = 0;
    {
        std::lock_guard g(m_lock);
        if (!m_connected)
            return;

        bool isEmpty = addTask(std::move(buffer), m_tasksRecv);
        if (isEmpty)
        {
            m_context.events |= EPOLLIN;
            m_worker->modify(m_handle, &m_context);
            err = recv0();
        }
    }
    if (err)
    {
        disconnect(Error::formatError(err));
    }
}

int Connection::numBytesPending()
{
    int ret = m_numBytesPending.load();
    assert (ret >= 0);
    return ret;
}

int Connection::recv0()
{
    int ret = doFirstTask(m_tasksRecv,
            [this](ByteBuffer buf)
            {
                int err = ::recv(m_handle, buf.data() + buf.writerIndex(), buf.capacity() - buf.writerIndex(), 0);
                if (err == 0)
                {
                    return Error::getSocketError(m_handle);
                }
                if (err < 0)
                {
                    if (errno == EAGAIN)
                        return 0;
                    return errno;
                }

                buf.moveWriterIndex(err);
                return 0;
            },
            std::bind(&EasyIO::TCP::Connection::recvComplete, this, std::placeholders::_1),
            onBufferReceived
            );
    return ret;
}

bool Connection::recvComplete(ByteBuffer buffer)
{
    return buffer.numReadableBytes() != 0;
}


bool Connection::enableKeepalive(unsigned long nInterval, unsigned long nTime)
{
    int onoff = 1;
    int keepaliveinterval = nInterval;
    int keepalivetime = nTime;

    if (setsockopt(m_handle, SOL_SOCKET, SO_KEEPALIVE, (void*)&onoff, sizeof(onoff))
        || setsockopt(m_handle, SOL_TCP, TCP_KEEPIDLE, (void*)&keepalivetime, sizeof(keepalivetime))
        || setsockopt(m_handle, SOL_TCP, TCP_KEEPINTVL, (void*)&keepaliveinterval, sizeof(keepaliveinterval)))
    {
        return false;
    }

    return true;
}

bool Connection::disableKeepalive()
{
    int onoff = 0;

    int ret = setsockopt(m_handle, SOL_SOCKET, SO_KEEPALIVE, (void*)&onoff, sizeof(onoff));
    return !ret;
}

bool Connection::setSendBufferSize(unsigned long nSize)
{
    int ret = setsockopt(m_handle, SOL_SOCKET, SO_SNDBUF, (char*)&nSize, sizeof(unsigned long));
    return !ret;
}

bool Connection::setReceiveBufferSize(unsigned long nSize)
{
    int ret = setsockopt(m_handle, SOL_SOCKET, SO_RCVBUF, (char*)&nSize, sizeof(unsigned long));
    return !ret;
}

bool Connection::setLinger(unsigned short onoff, unsigned short linger)
{
    struct linger opt = {onoff, linger};
    int ret = setsockopt(m_handle, SOL_SOCKET, SO_LINGER, (char*)&opt, sizeof(opt));
    return !ret;
}

const std::string& Connection::localIP() const
{
    return m_localIP;
}

unsigned short Connection::localPort() const
{
    return m_localPort;
}

const std::string& Connection::peerIP() const
{
    return m_peerIP;
}

unsigned short Connection::peerPort() const
{
    return m_peerPort;
}

bool Connection::updateEndPoint()
{
    socklen_t len = sizeof(sockaddr_in);
    sockaddr_in addrLocal, addrPeer;

    if(getpeername(m_handle, (sockaddr*)&addrPeer, &len) == -1)
    {
        return false;
    }

    m_peerIP = inet_ntoa(addrPeer.sin_addr);
    m_peerPort = ntohs(addrPeer.sin_port);

    if(getsockname(m_handle, (sockaddr*)&addrLocal, &len) == -1)
    {
        return false;
    }

    m_localIP = inet_ntoa(addrLocal.sin_addr);
    m_localPort = ntohs(addrLocal.sin_port);

    return true;
}

void Connection::bindUserdata(void *userdata)
{
    m_userdata = userdata;
}

void *Connection::userdata() const
{
    return m_userdata;
}

bool Connection::addTask(ByteBuffer buffer, std::list<ByteBuffer> &dst)
{
    bool ret = dst.empty();
    dst.push_back(buffer);
    return ret;
}

int Connection::doFirstTask(std::list<ByteBuffer> &tasks, std::function<int (ByteBuffer)> transmitter,
                            std::function<bool(ByteBuffer)> isComplete, std::function<void (IConnection*, ByteBuffer)> onComplete)
{
    int ret;
    ByteBuffer buf;
    {
        std::lock_guard g(m_lock);

        if (tasks.empty())
            return 0;

        buf = tasks.front();
        ret = transmitter(buf);
    }
    if (isComplete(buf))
    {
        if (onComplete)
            onComplete(this, buf);
        popFirstTask(tasks);
    }
    return ret;
}

void Connection::popFirstTask(std::list<ByteBuffer> &tasks)
{
    std::lock_guard g(m_lock);
    if (tasks.empty())
        return;
    tasks.pop_front();
}

void Connection::cleanTasks(std::list<ByteBuffer> &tasks)
{
    std::lock_guard g(m_lock);

    while (!tasks.empty())
        popFirstTask(tasks);
}

void Connection::handleEvents(uint32_t events)
{
    int err = 0;
    do
    {
        if (events & EPOLLIN)
        {
            err = recv0();
            if (err)
                break;

            std::lock_guard g(m_lock);
            if (m_tasksRecv.empty())
            {
                m_context.events &= ~EPOLLIN;
                m_worker->modify(m_handle, &m_context);
            }
        }

        if (events & EPOLLERR)
        {
            std::lock_guard g(m_lock);
            err = Error::getSocketError(m_handle);
            break;
        }

        if (events & EPOLLOUT)
        {
            err = send0();
            if (err)
                break;

            std::lock_guard g(m_lock);
            if (m_tasksSend.empty())
            {
                m_context.events &= ~EPOLLOUT;
                m_worker->modify(m_handle, &m_context);
            }
        }
    } while (0);


    m_lock.lock();
    if (m_disconnecting)
    {
        m_reason = Error::STR_FORCED_CLOSURE;
        disconnect0(true);
    }
    else if (err)
    {
        m_reason = Error::formatError(err);
        disconnect0(true);
    }
    else
        m_lock.unlock();
}

IConnectionPtr Connection::makeHolder()
{
    IConnectionPtr holder;
    if (!this->weak_from_this().expired())
        holder = this->shared_from_this();;
    return holder;
}


#endif



