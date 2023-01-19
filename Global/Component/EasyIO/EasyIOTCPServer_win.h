#if  defined(WIN32) || defined(WIN64)
#ifndef EASYIOTCPSERVER_H
#define EASYTCPSERVER_H

#include "EasyIOTCPConnection_win.h"
#include "EasyIOTCPAcceptor.h"
#include "EasyIOEventLoopGroup.h"
#include "EasyIOTCPIServer.h"
#include <map>
#include <mutex>

namespace EasyIO
{
    namespace TCP
    {
        class Server : public IServer
        {
        public:
            static IServerPtr create(unsigned numWorkers = 1);
            static IServerPtr create(EventLoopGroupPtr workers);
            ~Server();

            bool open(unsigned short port, unsigned int backlog = 15) override;
            void close() override;
            bool opened() override;

        private:
            Server(EventLoopGroupPtr workers);

            void addConnection(SOCKET sock);
            void removeConnection(IConnection* con, const std::string& reason);


        private:
            std::shared_ptr<Acceptor> m_acceptor;
            EventLoopGroupPtr m_workers;
            std::map<IConnection*, IConnectionPtr> m_connections;
            std::mutex m_mutex;
            bool m_opened;

            std::recursive_mutex m_lockConnections;
        };
    }
}

#endif
#endif
