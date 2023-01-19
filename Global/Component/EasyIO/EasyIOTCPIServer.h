#ifndef EASYIOTCPISERVER_H
#define EASYIOTCPISERVER_H

#include <memory>
#include <functional>
#include "EasyIOTCPIConnection.h"

namespace EasyIO
{
    namespace TCP
    {
        class IServer;
        typedef std::shared_ptr<IServer> IServerPtr;

        class IServer
        {
        public:
            IServer(){}
            virtual ~IServer(){}

            virtual bool open(unsigned short port, unsigned int backlog = 15) = 0;
            virtual void close() = 0;
            virtual bool opened() = 0;

        public:
            std::function<void (IConnection*)> onConnected;
            std::function<void (IConnection*, const std::string& reason)> onDisconnected;
            std::function<void (IConnection*, ByteBuffer data)> onBufferReceived;
        };
    }
}


#endif
