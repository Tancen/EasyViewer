#if  defined(WIN32) || defined(WIN64)
#include "EasyIOContext_win.h"
#include "EasyIOEventLoop_win.h"

using namespace EasyIO;

EventLoop::EventLoop()
    : m_handle(nullptr),
      m_exit(false)
{

}

IEventLoopPtr EventLoop::create()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
    {
        return IEventLoopPtr();
    }

    EventLoop* w;
    IEventLoopPtr ret(w = new EventLoop());
    w->m_handle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!w->m_handle)
    {
        ret.reset();
    }
    else
    {
        w->m_thread = std::thread(std::bind(&EventLoop::execute, w));
    }

    return ret;
}

EventLoop::~EventLoop()
{
    WSACleanup();

    m_exit = true;
    CloseHandle(m_handle);
    if (m_thread.joinable())
        m_thread.join();
}

IEventLoopPtr EventLoop::share()
{
    return std::dynamic_pointer_cast<IEventLoop>(this->shared_from_this());
}

bool EventLoop::attach(SOCKET sock, int& err)
{
    if(!CreateIoCompletionPort((HANDLE)sock, m_handle, 0, 0))
    {
        err = GetLastError();
        return false;
    }

    return true;
}

void EasyIO::EventLoop::detach(SOCKET sock)
{
    closesocket(sock);
}

void EventLoop::execute()
{
    int err;
    int ret;
    DWORD numBytes;
    ULONG_PTR key;
    Context *pContext;

    while (!m_exit)
    {
        ret = GetQueuedCompletionStatus(m_handle, &numBytes,
                        &key, (LPOVERLAPPED*)&pContext, INFINITE);
        if (ret)
        {
            if (!pContext)
                break;

            pContext->increaseProgress(numBytes);
        }
        else
        {
            err = GetLastError();

            if (pContext)
            {
                pContext->error(err);
            }
            else
                break;
        }
    }
}
#endif
