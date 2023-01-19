#include "EasyIOUDPServer.h"

#if  defined(WIN32) || defined(WIN64)
    #include "EasyIOUDPSocket_win.h"
    #include "EasyIOEventLoop_win.h"
#else
    #include "EasyIOUDPSocket_linux.h"
    #include "EasyIOEventLoop_linux.h"
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <fcntl.h>
#endif

EasyIO::UDP::IServerPtr EasyIO::UDP::Server::create()
{
    IEventLoopPtr worker = EventLoop::create();
    return create(worker);
}

EasyIO::UDP::IServerPtr EasyIO::UDP::Server::create(EasyIO::IEventLoopPtr worker)
{
    IServerPtr ret;
    if (!dynamic_cast<EventLoop*>(worker.get()))
        return ret;

    ret.reset(new Server(worker));
    return ret;
}

EasyIO::UDP::Server::~Server()
{
    close();
    while (1)
    {
        {
            std::lock_guard g(m_lock);
            if (!m_closing)
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


bool EasyIO::UDP::Server::open(unsigned short port)
{
    {
        std::lock_guard g(m_lock);

        if (m_opened)
            return false;
        do
        {
            m_handle  = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            if (m_handle == INVALID_SOCKET)
            {
                break;
            }

#if !defined(WIN32) && !defined(WIN64)
            int flag = fcntl(m_handle, F_GETFL, 0);
            if (-1 == flag )
                break;
            fcntl(m_handle, F_SETFL, flag | O_NONBLOCK);
#endif
            EventLoop* w = (EventLoop*)m_worker.get();
            #if  defined(WIN32) || defined(WIN64)
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.S_un.S_addr = INADDR_ANY;
                addr.sin_port = htons(port);

                if (bind(m_handle, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
                {
                    break;
                }

                int err = 0;
                if(!w->attach(m_handle, err))
                {
                    break;
                }
            #else
                sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_addr.s_addr = INADDR_ANY;
                addr.sin_port = htons(port);

                if(bind(m_handle, (sockaddr*)&addr, sizeof(addr)) == -1)
                {
                    break;
                }
                w->add(m_handle, &m_context);
            #endif

            updateEndPoint();
            m_opened = true;
            return true;
        } while(0);
    }

    return false;
}

EasyIO::UDP::Server::Server(EasyIO::IEventLoopPtr worker)
    :
      #if !defined(WIN32) && !defined(WIN64)
        Socket((EventLoop*)worker.get()),
      #endif
        m_worker(worker)
{

}

