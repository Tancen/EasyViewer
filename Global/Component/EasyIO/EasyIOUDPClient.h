#ifndef EASYIOUDPCLIENT_WIN_H
#define EASYIOUDPCLIENT_WIN_H

#include "EasyIOUDPISocket.h"
#include "EasyIOUDPIClient.h"
#include "EasyIOUDPServer.h"
#include "EasyIOIEventLoop.h"
#include <atomic>

namespace EasyIO
{
    namespace UDP
    {
        class Client : public Server, public IClient
        {
        public:
            static IClientPtr create();
            static IClientPtr create(IEventLoopPtr worker);

            ~Client();

        private:
            Client(IEventLoopPtr worker);
        };
    }
}
#endif
