#ifdef __linux__
#include "EasyIOUDPSocket_linux.h"
#include "EasyIOError.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>
#include <assert.h>

using namespace EasyIO::UDP;
using namespace std::placeholders;


class Socket::Task
{
public:
    Task(ByteBuffer buffer)
        : m_addrLen(sizeof(sockaddr_in)),
          m_buffer(buffer)
    {

    }

    ByteBuffer buffer(){ return m_buffer; }
    sockaddr_in* addr(){ return &m_addr; }
    socklen_t* addrLen(){ return &m_addrLen; }

private:
    sockaddr_in m_addr;
    socklen_t m_addrLen;
    ByteBuffer m_buffer;
};


Socket::Socket(EventLoop *worker)
    : Socket(worker, INVALID_SOCKET)
{

}

Socket::Socket(EventLoop *worker, SOCKET sock)
    : m_worker(worker),
      m_handle(sock),
      m_context(std::bind(&Socket::handleEvents, this, _1)),
      m_opened(false),
      m_closing(false),
      m_localPort(0),
      m_userdata(NULL),
      m_numBytesPending(0)
{
    if (m_handle != INVALID_SOCKET)
    {
        m_worker->add(m_handle, &m_context);
    }
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
        if (!m_opened || m_closing)
            return;

        m_closing = true;
        m_context.events |= EPOLLIN;
        m_worker->modify(m_handle, &m_context);
        shutdown(m_handle, SHUT_RDWR);
    }
    while(0);

    do
    {
        {
            std::lock_guard g(m_lock);
            if (!m_opened && !m_closing)
                break;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    } while (true);
}

void Socket::close0()
{
    ISocketPtr holder;
    if (!this->weak_from_this().expired())
        holder = this->shared_from_this();;

    {
        std::lock_guard  g(m_lock);
        m_worker->remove(m_handle, &m_context);
        closesocket(m_handle);
        m_handle = INVALID_SOCKET;
        m_opened = false;
        m_numBytesPending = 0;
        cleanTasks(m_tasksSend);
        cleanTasks(m_tasksRecv);
        m_closing = false;
    }
}

void Socket::send(const std::string& ip, unsigned short port, ByteBuffer buffer)
{
    if (!buffer.numReadableBytes())
        return;

    int err = 0;
    {
        std::lock_guard g(m_lock);
        if (!m_opened)
            return;

        TaskPtr task(new Task(buffer));
        task->addr()->sin_family = AF_INET;
        task->addr()->sin_addr.s_addr = inet_addr(ip.c_str());
        task->addr()->sin_port = htons(port);

        m_numBytesPending += buffer.numReadableBytes();
        bool isEmpty = addTask(task, m_tasksSend);
        if (isEmpty)
        {
            err = send0();
            if (!err)
            {
                if (m_tasksSend.empty() && (m_context.events & EPOLLOUT))
                {
                    m_context.events &= ~EPOLLOUT;
                    m_worker->modify(m_handle, &m_context);
                }
            }
            else
                popFirstTask(m_tasksSend);
        }
    }
}

int Socket::send0()
{
    int ret = doFirstTask(m_tasksSend,
            [this](TaskPtr task)
            {
                do
                {
                    if (!task->buffer().numReadableBytes())
                        break;

                    int err = ::sendto(m_handle, task->buffer().readableBytes(), task->buffer().numReadableBytes(), 0,
                                       (sockaddr*)(task->addr()), *task->addrLen());
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

                    task->buffer().moveReaderIndex(err);
                    m_numBytesPending -= err;
                } while (true);
                return 0;
            },
            std::bind(&EasyIO::UDP::Socket::sendComplete, this, std::placeholders::_1),
            nullptr
            );

    return ret;
}

bool Socket::sendComplete(TaskPtr task)
{
    return !task->buffer().numReadableBytes();
}

void Socket::recv(ByteBuffer buffer)
{
    buffer.ensureWritable(4096);

    {
        std::lock_guard g(m_lock);
        if (!m_opened)
            return;

        TaskPtr task(new Task(buffer));
        bool isEmpty = addTask(task, m_tasksRecv);
        if (isEmpty)
        {
            m_context.events |= EPOLLIN;
            m_worker->modify(m_handle, &m_context);
            recv0();
        }
    }
}

size_t Socket::numBytesPending()
{
    return m_numBytesPending;
}

int Socket::recv0()
{
    int ret = doFirstTask(m_tasksRecv,
            [this](TaskPtr task)
            {
                int err = ::recvfrom(m_handle, task->buffer().data() + task->buffer().writerIndex(),
                                     task->buffer().capacity() - task->buffer().writerIndex(), 0,
                                     (sockaddr*)(task->addr()), task->addrLen());
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

                task->buffer().moveWriterIndex(err);
                return 0;
            },
            std::bind(&EasyIO::UDP::Socket::recvComplete, this, std::placeholders::_1),
            onBufferReceived
            );
    return ret;
}


bool Socket::recvComplete(TaskPtr task)
{
    return task->buffer().numReadableBytes() != 0;
}

const std::string& Socket::localIP() const
{
    return m_localIP;
}

unsigned short Socket::localPort() const
{
    return m_localPort;
}

bool Socket::updateEndPoint()
{
    sockaddr_in addrLocal = {0};
    socklen_t len = sizeof(sockaddr_in);

    if(getsockname(m_handle, (sockaddr*)&addrLocal, &len) == -1)
    {
        return false;
    }

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

bool Socket::addTask(TaskPtr task, std::list<TaskPtr> &dst)
{
    bool ret = dst.empty();
    dst.push_back(task);
    return ret;
}

int Socket::doFirstTask(std::list<TaskPtr> &tasks, std::function<int (TaskPtr)> transmitter,
                         std::function<bool(TaskPtr)> isComplete,
                         std::function<void (ISocket*, const std::string&, unsigned short, ByteBuffer)> onComplete)
{
    bool ret;
    TaskPtr task;
    {
        std::lock_guard g(m_lock);

        if (tasks.empty())
            return true;

        task = tasks.front();
        ret = transmitter(task);
    }
    if (isComplete(task))
    {
        if (onComplete)
        {
            std::string ip = inet_ntoa(task->addr()->sin_addr);
            unsigned short port = ntohs(task->addr()->sin_port);
            onComplete(this, ip, port, task->buffer());
        }
        popFirstTask(tasks);
    }
    return ret;
}

void Socket::popFirstTask(std::list<TaskPtr> &tasks)
{
    std::lock_guard g(m_lock);
    if (tasks.empty())
        return;
    tasks.pop_front();
}

void Socket::cleanTasks(std::list<TaskPtr> &tasks)
{
    std::lock_guard g(m_lock);

    while (!tasks.empty())
        popFirstTask(tasks);
}

void Socket::handleEvents(uint32_t events)
{
    do
    {
        if (events & EPOLLIN)
        {
            recv0();
            std::lock_guard g(m_lock);
            if (m_tasksRecv.empty())
            {
                m_context.events &= ~EPOLLIN;
                m_worker->modify(m_handle, &m_context);
            }
        }

        if (events & EPOLLERR)
        {
            break;
        }

        if (events & EPOLLOUT)
        {
            send0();
            std::lock_guard g(m_lock);
            if (m_tasksSend.empty())
            {
                m_context.events &= ~EPOLLOUT;
                m_worker->modify(m_handle, &m_context);
            }
        }
    } while (0);


    {
        std::lock_guard g(m_lock);
        if (m_closing)
            close0();
    }
}

#endif
