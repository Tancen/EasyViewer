#ifdef __linux__
#include "EasyIOEventLoop_linux.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <signal.h>
#include <bits/types.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

using namespace EasyIO;

EventLoop::EventLoop()
    : m_handle(-1),
      m_pid(-1),
      m_exit(false)
{

}

EventLoop::~EventLoop()
{
    m_exit = true;
    close(m_handle);

    if (m_thread.joinable())
    {
        kill(m_pid, SIGUSR1);
        m_thread.join();
    }
}

IEventLoopPtr EventLoop::share()
{
    return std::dynamic_pointer_cast<IEventLoop>(this->shared_from_this());
}

IEventLoopPtr EventLoop::create(int maxEvents)
{
    EventLoop* w;
    IEventLoopPtr ret(w = new EventLoop());

    do
    {
        w->m_handle = epoll_create(1);
        if (w->m_handle == -1)
            break;

        w->m_thread = std::thread(std::bind(&EventLoop::execute, w, maxEvents));
        while (w->m_pid == -1)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
        if (!w->m_pid)
            break;

        return ret;
    } while (0);

    ret.reset();
    return ret;
}

void EventLoop::add(int fd, Context::Context *context)
{
    int ret = epoll_ctl(m_handle, EPOLL_CTL_ADD, fd, context);
    assert(!ret);
}

void EventLoop::modify(int fd, Context::Context *context)
{
    int ret = epoll_ctl(m_handle, EPOLL_CTL_MOD, fd, context);
    int err = errno;
    std::string str = strerror(err);
    assert(!ret);
}

void EventLoop::remove(int fd, Context::Context *context)
{
    int ret = epoll_ctl(m_handle, EPOLL_CTL_DEL, fd, context);
    assert(!ret);
}

void EventLoop::execute(int maxEvents)
{
    int i, ret;
    epoll_event* events;
    Context::Context *pContext;

    struct sigaction act;
    act.sa_handler = [](int){};
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(sigaction( SIGUSR1, &act, NULL))
    {
        m_pid = 0;
        return;
    }

    m_pid = syscall(SYS_gettid);

    if (maxEvents <= 0)
        maxEvents = 1024;

    events = new epoll_event[maxEvents];
    while (!m_exit)
    {
        ret = epoll_wait(m_handle, events, maxEvents, -1);
        if (ret == -1)
        {
            switch(errno)
            {
            case EINTR:
            case EAGAIN:
                continue;
            default:
                break;
            }
        }
        for (i = 0; i < ret; i++)
        {
            pContext = (Context::Context *)events[i].data.ptr;
            pContext->update(events[i].events);
        }
    }

    delete[] events;
}
#endif
