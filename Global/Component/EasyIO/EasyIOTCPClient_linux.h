#ifdef __linux__
#ifndef EASYIOTCPCLIENT_H
#define EASYIOTCPCLIENT_H

#include "EasyIOTCPConnection_linux.h"
#include "EasyIOIEventLoop.h"
#include "EasyIOContext_win.h"
#include "EasyIOTCPIClient.h"
#include <atomic>

namespace EasyIO
{
    namespace TCP
    {
        class Client : public IClient, public Connection
        {
        public:
            static IClientPtr create();
            static IClientPtr create(IEventLoopPtr worker);

            ~Client();

            void connect(const std::string& host, unsigned short port) override;
            bool connecting() override;
            void disconnect() override;
            void syncDisconnect() override;

        private:
            Client(IEventLoopPtr worker);

            IConnectionPtr makeHolder() override;

        private:
            IEventLoopPtr m_worker;
            bool m_connecting;
            bool m_canceling;
            std::atomic<int> m_detained;
        };
    }
}

#endif
#endif
