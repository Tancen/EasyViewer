#ifndef EASYIOUDPISOCKET_H
#define EASYIOUDPISOCKET_H

#include "EasyIOByteBuffer.h"
#include "EasyIODef.h"
#include <functional>

namespace EasyIO
{
    namespace UDP
    {
        class ISocket;
        typedef std::shared_ptr<ISocket> ISocketPtr ;

        class ISocket : public std::enable_shared_from_this<ISocket>
        {
        public:
            ISocket(){}
            virtual ~ISocket(){}

            virtual ISocketPtr share() = 0;
            virtual SOCKET handle() = 0;
            virtual void send(const std::string& ip, unsigned short port, ByteBuffer buffer) = 0;
            virtual void recv(ByteBuffer buffer) = 0;
            virtual void close() = 0;
            virtual bool opened() = 0;

            /**
            * @brief   get the number of bytes waiting to send
            */
            virtual size_t numBytesPending() = 0;

            virtual const std::string& localIP() const = 0;
            virtual unsigned short localPort() const = 0;

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
            std::function<void (ISocket*, const std::string& peerIP, unsigned short peerPort,
                                ByteBuffer data)> onBufferReceived;
        };
    }
}

#endif
