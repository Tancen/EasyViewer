#ifndef EASYIOTCPICONNECTION_H
#define EASYIOTCPICONNECTION_H

#include "EasyIOByteBuffer.h"
#include "EasyIODef.h"
#include <functional>
#include <memory>

namespace EasyIO
{
    namespace TCP
    {
        class IConnection;
        typedef std::shared_ptr<IConnection> IConnectionPtr;

        class IConnection : public std::enable_shared_from_this<IConnection>
        {
        public:
            IConnection(){}
            virtual ~IConnection(){}

            virtual IConnectionPtr share() = 0;
            virtual SOCKET handle() = 0;

            virtual bool connected() = 0;

            virtual void disconnect() = 0;
            virtual void syncDisconnect() = 0;
            virtual void send(ByteBuffer buffer) = 0;
            virtual void recv(ByteBuffer buffer) = 0;

            /**
            * @brief   get the number of bytes waiting to send
            */
            virtual int numBytesPending() = 0;

            virtual bool enableKeepalive(unsigned long interval = 1000, unsigned long time = 2000) = 0;
            virtual bool disableKeepalive() = 0;

            virtual bool setSendBufferSize(unsigned long size) = 0;
            virtual bool setReceiveBufferSize(unsigned long size) = 0;

            virtual bool setLinger(unsigned short onoff, unsigned short linger) = 0;

            virtual const std::string& localIP() const = 0;
            virtual unsigned short localPort() const = 0;

            virtual const std::string& peerIP() const = 0;
            virtual unsigned short peerPort() const = 0;

            /**
            * @brief   Upadate localIP, localPort, peerIP, peerPort
            */
            virtual bool updateEndPoint() = 0;

            /**
            * @brief   Bind user data on this connection
            */
            virtual void bindUserdata(void* userdata) = 0;

            /**
            * @brief   get bound user data
            */
            virtual void* userdata() const = 0;

        public:
            std::function<void (IConnection*, const std::string& reason)> onDisconnected;
            std::function<void (IConnection*, ByteBuffer data)> onBufferReceived;
        };
    }
}
#endif
