#ifndef EASYIOUDPSERVER_H
#define EASYIOUDPSERVER_H

#if  defined(WIN32) || defined(WIN64)
    #include "EasyIOUDPSocket_win.h"
#else
    #include "EasyIOUDPSocket_linux.h"
#endif
#include "EasyIOUDPIServer.h"
#include "EasyIOIEventLoop.h"
#include <atomic>
#include <mutex>

namespace EasyIO
{
    namespace UDP
    {
        class Server : public Socket, virtual public IServer
        {
        public:
            static IServerPtr create();
            static IServerPtr create(IEventLoopPtr worker);

            ~Server();

            bool open(unsigned short port) override;

        protected:
            Server(IEventLoopPtr worker);

        protected:
            IEventLoopPtr m_worker;
        };
    }
}

#endif
