#ifndef EASYIOUDPISERVER_H
#define EASYIOUDPISERVER_H

#include "EasyIOByteBuffer.h"
#include "EasyIOUDPISocket.h"
#include <functional>

namespace EasyIO
{
    namespace UDP
    {
        class IServer : virtual public ISocket
        {
        public:
            IServer(){}
            virtual ~IServer(){}
            virtual bool open(unsigned short port) = 0;
        };

        typedef std::shared_ptr<IServer> IServerPtr;
    }
}

#endif
